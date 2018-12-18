#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <carmen/carmen.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;

#define rrt_obstacle_probability_threshold 0.5			 // carmen-ford-escape.ini
#define IS_OBSTACLE rrt_obstacle_probability_threshold   // Minimum probability of a map cell being an obstacle

int true_positives = 0;
int true_negatives = 0;
int false_positives = 0;
int false_negatives = 0;


int
str_ends_with(const char * str, const char * suffix)          // Returns 1 if string ends with suffix
{
  if( str == NULL || suffix == NULL )
    return 0;

  size_t str_len = strlen(str);
  size_t suffix_len = strlen(suffix);

  if(suffix_len > str_len)
    return 0;

  return (0 == strncmp((str + str_len - suffix_len), suffix, suffix_len));
}


cv::Vec3b
transform_map_to_image(double gt_cell, double map_cell)
{
	if (gt_cell >= 0.0 && map_cell >= 0.0)
	{
		if (gt_cell >  IS_OBSTACLE && map_cell >  IS_OBSTACLE)
			return cv::Vec3b(0,0,0);
		if (gt_cell <= IS_OBSTACLE && map_cell <= IS_OBSTACLE)
			return cv::Vec3b(255,255,255);
		if (gt_cell <= IS_OBSTACLE && map_cell >  IS_OBSTACLE)
			return cv::Vec3b(0,0,255);
		if (gt_cell >  IS_OBSTACLE && map_cell <= IS_OBSTACLE)
			return cv::Vec3b(31,172,255);
	}

	return cv::Vec3b(255, 144, 30);
}


void
compute_map_diff(char *gt_path, char *map_path)
{
	carmen_map_t gt_map, map;

	if (carmen_map_read_gridmap_chunk(gt_path, &gt_map) == 0)
	{
		printf("Opening gt_map: %s\n", gt_path);

		if (carmen_map_read_gridmap_chunk(map_path, &map) == 0)
		{
			printf("Opening map:    %s\n", map_path);
			cv::Mat img = cv::Mat(cv::Size(gt_map.config.x_size, gt_map.config.y_size), CV_8UC3, cv::Scalar(255, 144, 30));

			int size = gt_map.config.x_size * gt_map.config.y_size;
			for (int i = 0; i < size; i++)
			{
//				if (gt_map.complete_map[i] < 0.0)
//					continue;

				if (gt_map.complete_map[i] >= 0.0 && map.complete_map[i] >= 0.0)
				{
					true_positives  += (gt_map.complete_map[i] >  IS_OBSTACLE && map.complete_map[i] >  IS_OBSTACLE);
					true_negatives  += (gt_map.complete_map[i] <= IS_OBSTACLE && map.complete_map[i] <= IS_OBSTACLE);
					false_positives += (gt_map.complete_map[i] <= IS_OBSTACLE && map.complete_map[i] >  IS_OBSTACLE);
					false_negatives += (gt_map.complete_map[i] >  IS_OBSTACLE && map.complete_map[i] <= IS_OBSTACLE);
				}
				int x = int(gt_map.config.y_size - 1 - (i % gt_map.config.x_size));
				int y = int(i / gt_map.config.x_size);
				img.at<cv::Vec3b>(cv::Point(y, x)) = transform_map_to_image(gt_map.complete_map[i], map.complete_map[i]);
			}
			carmen_map_free_gridmap(&map);

			char img_name[1024];
			sprintf(img_name, "%s.png", map_path);

			resize(img, img, cv::Size(0, 0), 2.5, 2.5, cv::INTER_NEAREST);
			imwrite(img_name, img);
			//imshow("Map", img);
			//cv::waitKey(0);
		}
		else
			printf("Could not read map: %s\n", map_path);

		carmen_map_free_gridmap(&gt_map);
	}
	else
		printf("Could not read gt_map: %s\n", map_path);
}


void
compute_metrics()
{
	double precision = 0.0;
	double recall    = 0.0;
	double accuracy  = 0.0;
	int positives = 0;
	int negatives = 0;
	int total = 0;

	precision = (double) true_positives / (true_positives + false_positives);

	recall = (double) true_positives / (true_positives + false_negatives);

	accuracy = (double)(true_positives + true_negatives) / (true_positives + true_negatives + false_positives + false_negatives);

	positives = true_positives + false_negatives;

	negatives = true_negatives + false_positives;

	total = positives + negatives;

	printf("True Positives  (%8d)\n", true_positives);
	printf("True Negatives  (%8d)\n", true_negatives);
	printf("False Positives (%8d)\n", false_positives);
	printf("False Negatives (%8d)\n", false_negatives);

	printf("Positives %9d (%6.2lf%%)\n", positives, (double) 100 * positives / total);
	printf("Negatives %9d (%6.2lf%%)\n", negatives, (double) 100 * negatives / total);
	printf("Precision %9lf \nRecall    %9lf\nAccuracy  %9lf\n", precision, recall, accuracy);
}


void
compute_maps_difference(char *gt_dir_path, char *map_dir_path)
{
    DIR *gt_dir;
    struct dirent *gt_content;
    char complete_gt_path[1024], complete_path[1024];
    int x, y;

    if ((gt_dir = opendir(gt_dir_path)) == NULL)
    {
        printf("Error while opening ground_truth_map_path: %s\n", gt_dir_path);
        return;
    }

    while ((gt_content = readdir(gt_dir)) != NULL)
    {
    	unsigned int pos = 0;

    	if (sscanf(gt_content->d_name, "m%d_%d.map%n", &x, &y, &pos) == 2 && pos == strlen(gt_content->d_name))
    	{
    		if (gt_dir_path[strlen(gt_dir_path) - 1] == '/')
    			sprintf(complete_gt_path, "%s%s", gt_dir_path, gt_content->d_name);
    		else
    			sprintf(complete_gt_path, "%s/%s", gt_dir_path, gt_content->d_name);

    		if (map_dir_path[strlen(map_dir_path) - 1] == '/')
    			sprintf(complete_path, "%s%s", map_dir_path, gt_content->d_name);
    		else
    			sprintf(complete_path, "%s/%s", map_dir_path, gt_content->d_name);

    		compute_map_diff(complete_gt_path, complete_path);
    	}
    }
    closedir(gt_dir);
}



int
main(int argc, char **argv)
{
	if (argc < 3 || argc > 4)
	{
		printf("\nUsage: %s <ground_truth_map_path> <map_path> -show\n\n", argv[0]);
		return 0;
	}

    compute_maps_difference(argv[1], argv[2]);

    compute_metrics();

    return 0;
}
