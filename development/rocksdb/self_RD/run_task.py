import os

params = {
    "-i": [100000],
    # "--RD": [1, 5, 10, 50, 100, 500, 900],
    "-P": [512],
    # "-B": [4, 16, 64, 256],
    # "-E": [1024, 256, 64, 16],
    "-T": [2],
    "--selectivity": [0.001],
    "--insert_before_range_delete": [0.9],
    "--gen_workload": [1],
    "--max_open_files": [999],
    # "--workload_filename"
}


tasks = ["./simple_example"]
for key, values in params.items():
    tasks = [" ".join([task, key, str(value)]) for value in values for task in tasks]
    # print(tasks)

B_list = [4, 16, 64, 256]
E_list = [1024, 256, 64, 16]
RD_list = [1, 5, 10, 50, 100, 500, 900]
selectivity_list = [0.001]

exp_code = "4"
for i, (B, E) in enumerate(zip(B_list, E_list)):
    # print(B, E)
    for j, RD in enumerate(RD_list):
        # print(RD)
        file_code = exp_code+str(i+1)+str(j+1)
        # print(file_code)
        task = tasks[0] + f" -B {B} -E {E}"
        task += f" --RD {RD} --selectivity {selectivity_list[0]}"
        task += f" --workload_filename workload/workload{file_code}.txt > log{file_code}"
        print(task)
        os.system(task)




B_list = [4, 16, 64, 256]
E_list = [1024, 256, 64, 16]
RD_list = [100]
selectivity_list = [0.0001, 0.0005, 0.001, 0.005, 0.009]

exp_code = "5"
for i, (B, E) in enumerate(zip(B_list, E_list)):
    # print(B, E)
    for j, sel in enumerate(RD_list):
        # print(RD)
        file_code = exp_code+str(i+1)+str(j+1)
        # print(file_code)
        task = tasks[0] + f" -B {B} -E {E}"
        task += f" --RD {RD_list[0]} --selectivity {sel}"
        task += f" --workload_filename workload/workload{file_code}.txt > log{file_code}"
        print(task)
        os.system(task)




B_list = [4, 16, 64, 256]
E_list = [1024, 256, 64, 16]
RD_list = [10]
selectivity_list = [0.001, 0.005, 0.009, 0.01, 0.05, 0.09]

exp_code = "5"
for i, (B, E) in enumerate(zip(B_list, E_list)):
    # print(B, E)
    for j, sel in enumerate(RD_list):
        # print(RD)
        file_code = exp_code+str(i+1)+str(j+1)
        # print(file_code)
        task = tasks[0] + f" -B {B} -E {E}"
        task += f" --RD {RD_list[0]} --selectivity {sel}"
        task += f" --workload_filename workload/workload{file_code}.txt > log{file_code}"
        print(task)
        os.system(task)