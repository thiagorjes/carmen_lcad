
import os

results_dir = "results/0006_rerun_com_parametros_do_paper/"
files = [f for f in os.listdir(results_dir) if 'report_' in f]

for f in files:
    path = results_dir + "/" + f
    data = 'data_' + f.rsplit('_data_')[1]
    data_path = '/dados/data/' + data
    out_path = results_dir + '/summary_' + data 
    cmd = "python scripts/summarize.py " + path + " " + data_path + " | tee " + out_path
    print("-------------------------------------------")
    print(data)
    os.system(cmd)
    print()
   
