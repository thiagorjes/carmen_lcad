/* -*- mode: C;-*- */
%{
/*
   LoIniFileParser.y -- yacc source for Robolocust/Lobot config files

   This file implements the grammar used by yacc to spit out a C/C++
   parser that parses the config files for the Robolocust/Lobot program.
   These config files are simplified INI files whose syntax rules are as
   follows:

       1. Blank lines and comments are ignored.

       2. Only single-line comments are supported.

       3. Comments are started with either a '#' or ';' character.

       4. Comments must appear on lines by themselves (i.e., config lines
          cannot have comments embedded in them).

       5. Config lines can be either section names or key-value
          assignments.

       6. Section names must start with a letter and may be followed by
          letters, numbers or the underscore character.

       7. Section names have to be enclosed in square brackets, i.e., '['
          and ']'.

       8. The key name in a key-value assignment follows the same rules as
          section names (except that they should not be enclosed in square
          brackets).

       9. The key name must be followed by an equals sign (which may be
          surrounded by optional whitespace).

      10. The value must follow the equals sign in a key-value assignment
          statement.

      11. All tokens following the equals sign (disregarding optional
          trailing whitespace after the equals sign) comprise the value
          portion of the key-value statement.

      12. Long key-value statements may be broken up across multiple
          lines and continued from one line to the next by ending each
          line with a '\' character. Note that the '\' must be the last
          character on the line, i.e., it should be immediately followed
          by a newline and no other trailing whitespace between the '\'
          and the newline.

          All initial whitespace on the following line will be discarded.
          Thus, to break up a really long string across multiple lines,
          don't have any whitespace between the '\' at the end of lines
          and the immediately preceding character. However, to ensure
          that tokens on the following lines are considered separate, put
          some intervening whitespace between the '\' and the previous
          character.

          The '\' line continuation characters themselves are not part of
          the value portion of key-value statements.

      13. The value portion of a key-value statement is taken verbatim,
          i.e., there is no notion of quoting strings and/or escape
          sequences; the whitespace following the equals sign is ignored
          and then all characters up to the end of the line are taken as
          the value corresponding to the specified key

   The following example illustrates what such an INI file looks like:

      # This is a comment and will be ignored as will all the blank lines
      # in this example.

      ; Comments can also be introduced by a semicolon

      # These entries do not belong to any section and are part of the
      # anonymous/unnamed "global" section:
      foo = 123
      bar = look Ma, no quotes!

      # Here is some random section
      [some_section]

      # It is okay to repeat key names in different sections/scope:
      foo = ping
      bar = pong

      # A section ends when another starts
      [another_section]
      some_key = some_value

      # It is okay to repeat sections. Key-value assignments will simply
      # "resume" under the previously defined section's scope.
      [some_section]
      quoted_string = "no such thing! quotes are part of the value"

      # Repeating keys in a section is also okay. The newly assigned
      # value will override the previous one. This feature should be
      # exercised with extreme care!
      foo = oops! already defined, but not a problem
      bar = # this is not a comment; it is the value for the bar key

      # Here is an example of a long statement continued across multiple
      # lines.
      baz = this is a very very very long key-value statement that \
            continues on and on across several consecutive lines simply \
            to illustrate how such long statements may be broken up for \
            the sake of readability by using the backslash at the end \
            of each line as a line continuation character

      boo = this is another long key-value statement conti\
            nued on the next line but broken in the middle of words to ill\
            ustrate how this strange feature works

   Straying from the above rules and the syntax illustrated above will
   probably lead to errors (usually of the worst kind, i.e., silent
   assumptions of weird intent). This code is mostly just a
   quick-and-dirty hack and has not been tested extensively to iron out
   all possible bugs. So try not to push the envelope here; the basic
   config file syntax rules are more than adequate for most purposes.
*/

// Primary maintainer for this file: mviswana usc edu
// $HeadURL: svn://isvn.usc.edu/software/invt/trunk/saliency/src/Robots/LoBot/config/LoIniFileParser.y $ */
// $Id: LoIniFileParser.y 14083 2010-09-30 13:59:37Z mviswana $

/* lobot headers */
#include "Robots/LoBot/config/LoConfig.H"
#include "Robots/LoBot/config/LoLexYaccDefs.H"

/* Standard C++ headers */
#include <string>

/* Globals used in this module */
static std::string g_current_section = LOCD_TOP_LEVEL ;
static std::string g_identifier ;
static std::string g_value ;

%}

/* Symbols for the tokens recognized by the lexer and parser */
%token LOCD_IFP_OPENSEC      LOCD_IFP_CLOSESEC   LOCD_IFP_ASSOP
%token LOCD_IFP_IDENTIFIER   LOCD_IFP_VALUE

%%

/*------------------------- INI FILE GRAMMAR --------------------------*/

/*
   An INI file consists of multiple single-line configuration directives
   (comments and blank lines are filtered out by the lexer).
*/
ini_file: config_directive | ini_file config_directive ;

/*
   As illustrated above, Robolocust/lobot configuration files consist of
   blank lines and single-line comments and configuration directives. The
   blank lines and comments are filtered out by the lexer generated by
   flex (see LoIniFileLexer.l).

   The configuration directive lines, however, are tokenized by the lexer
   and passed in here to be handled by the parser. There are two kinds of
   configuration directives, viz., section specifications and option
   specifications.
*/
config_directive: section_spec | option_spec ;

/*
   When we encounter a section specification in the config file, we
   simply "switch context" to this newly defined section by updating the
   parser's notion of the current section to which key-value pairs are
   being added.

   Initially, the current section corresponds to the unnamed/anonymous
   global/top-level section and key-value pairs are inserted into this
   scope. When a new section is defined, the current section variable is
   updated and subsequent option specs will result in key-value pairs
   being added to this newly (or previously) defined scope.
*/
section_spec: LOCD_IFP_OPENSEC LOCD_IFP_IDENTIFIER LOCD_IFP_CLOSESEC {
      g_current_section = g_identifier ;
   } ;

/*
   When we encounter an option specification, we simply insert the
   supplied key-value pair into the Robolocust/lobot configuration
   database at the scope identified by the current section variable.
*/
option_spec: LOCD_IFP_IDENTIFIER LOCD_IFP_ASSOP LOCD_IFP_VALUE {
      lobot::Configuration::set(g_current_section, g_identifier, g_value) ;
   } ;

%%

/*---------------------------- EXTRA CODE -----------------------------*/

/* Let client module print its own error messages */
void yyerror(const char* error){}

/* API to allow lexer to set the current identifier and value tokens */
void locd_ifp_set_current_identifier(const char* name)
{
   g_identifier = name ;
}

void locd_ifp_set_current_value(const char* value)
{
   g_value = value ;
}
