import os
#for SuRF_RDF:
#if skip_reading_RD_blocks == 1:
#   surf__key_len_in_bytes > key_size_to_insert --> to ensure no (False Positive)

params = {
    #"-i": [100000*10],
    "-i": [64000],
    "-P": [4],
    "-T": [2],
    "--insert_before_range_delete": [0.9],
    "--gen_workload": [1],
    #"--max_open_files": [999],
    "--max_open_files": [20],
    "--skip_reading_RD_blocks": [0],
    #"--number_of_PQ": [5000*100],
    "--number_of_PQ": [5000],
    "--bb": [0],
    "--key_size_to_insert": [12],
    "--surf__key_len_in_bytes": [13],
    "--surf__hash_suffix_len": [0],
    "--surf__real_suffix_len": [0],
    "--surf__include_dense": [0],    # 1: true, 0: false
    "--surf__sparse_dense_ratio": [16],
    "--log_during_insertion": [1],    # 1: true, 0: false
    "--surf_use_condensed_digit_key": [1],    # 1: true, 0: false
}

tasks = ["./simple_example"]
for key, values in params.items():
    tasks = [" ".join([task, key, str(value)]) for value in values for task in tasks]

B_list: list = [256]
E_list: list = [16]

task_tmp = []
for i, (B, E) in enumerate(zip(B_list, E_list)):
    task_tmp.extend([f"{task} -B {B} -E {E}" for task in tasks])
tasks = task_tmp


def run(
        exp_code: chr = '4',
        RD_list: list = [1, 5, 10, 50, 100, 500, 900],
        sel_list: list = [0.001],
):
    # Make the length the same
    if len(sel_list) == 1:
        sel_list = sel_list * len(RD_list)
    else:
        RD_list = RD_list * len(sel_list)

    for i, task_orig in enumerate(tasks):
        for j, (RD, sel) in enumerate(zip(RD_list, sel_list)):
            file_code = exp_code+str(i+1)+str(j+1)
            task = task_orig + f" --RD {RD} --selectivity {sel} --workload_filename workload/workload{file_code}.txt > log{file_code}"
            print(task)
            os.system(task)
        
        
#run('4', RD_list=[0, 1, 5, 10, 50, 100, 500, 900], sel_list=[0.001])
#run('5', RD_list=[100], sel_list=[0.0001, 0.0005, 0.001, 0.005, 0.009])
#run('6', RD_list=[10], sel_list=[0.001, 0.005, 0.009, 0.01, 0.05, 0.09])
#run('7', RD_list=[50], sel_list=[0.001, 0.005, 0.009, 0.01])
#run('8', RD_list=[50], sel_list=[0.001, 0.005, 0.009, 0.01])
#run('9', RD_list=[50], sel_list=[0.001, 0.005, 0.009, 0.01])
#run('7', RD_list=[0], sel_list=[0.001, 0.005, 0.009, 0.01])
#run('8', RD_list=[0], sel_list=[0.001, 0.005, 0.009, 0.01])
#run('9', RD_list=[0], sel_list=[0.001, 0.005, 0.009, 0.01])
#run('10', RD_list=[100], sel_list=[0.001, 0.005, 0.009])
run('11', RD_list=[100], sel_list=[0.001, 0.005, 0.009])
run('12', RD_list=[1000], sel_list=[0.0001, 0.0005, 0.0009])
run('13', RD_list=[1000], sel_list=[0.0001, 0.0005, 0.0009])
