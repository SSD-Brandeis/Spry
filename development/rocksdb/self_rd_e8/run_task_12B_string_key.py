import os
from copy import deepcopy

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
    "--load_pq_workload": [0],
    #"--max_open_files": [999],
    "--max_open_files": [20],
    "--skip_reading_RD_blocks": [1], # control on block_based_table_reader, but not on table_cache, cannot set to 1 (skip range) if RDF will answer keyMayBeDeleted, basically just set it to 0 if each time just running on 1 RDF_TYPE
    #"--number_of_PQ": [5000*100],
    # "--number_of_PQ": [5000], # -1: for testing on all PQ, >=0 : sample #PQ from all PQ
    "--system_check_test_on_all_PQ": [0], # for debugging, 0: off, 1: on, equal to "--number_of_PQ": [-1]
    "--bb": [0],
    "--key_size_to_insert": [12],
    "--surf__key_len_in_bytes": [13],
    "--surf__hash_suffix_len": [0],
    "--surf__real_suffix_len": [0],
    "--surf__include_dense": [0], # 1: true, 0: false
    "--surf__sparse_dense_ratio": [16],
    "--log_during_insertion": [1], # 1: true, 0: false
    "--surf_use_condensed_digit_key": [1], # 1: true, 0: false # digit_key to condensed_char_keys
    "--run_pq_during_insertion_interval": [200], # default: 200
    "--using_rdf_types": ["NONE_DUMMY,NONE_CACHE_RANGETOMBSTONE_TRACING,NONE,NONE2,PLRDF,SPLIT_PLRDF,PLRDF_STRING_KEY,SPLIT_PLRDF_STRING_KEY,TOP_LEVEL_RDF,TOP_LEVEL_RDF_STRING_KEY,SKYLINE_RDF,SuRF_LF_RDF,SuRF_LF_SPLIT_RDF,NONE_DUMMY"],
    "--show_surf_compaction_info": [0],
    "--show_tombstones_during_compaction_info": [0], 
    "--flag_skip_compaction_trivial_move": [0], # if TOP_LEVEL_RDF is in --using_rdf_types, then no matter what value is set, compaction trivial move will always be skip
    "--use_surf_base": [0], # 1: true, 0: false
    "--surf_base_store_key_to_k_diff": [1], # is used when --use_surf_base = [1], value can only be nature number (> 0 integer)
    "--use_string_key": [1],
}


# logic for surf__flag_bypass_if_same_key, surf__flag_allow_range_boundary_overlapped
# if(use_surf_base == true){
#     surf__flag_bypass_if_same_key = true;
#     surf__flag_allow_range_boundary_overlapped = true;
# }else{
#     //key_size_to_insert this is the clipped off length
#     if(surf_use_condensed_digit_key){
#       surf__flag_bypass_if_same_key = surf__key_len_in_bytes <= length_of_condensed_digit_key; // whether to skip RDF checking if searding key is the same as the next greater key in the SuRF
#       surf__flag_allow_range_boundary_overlapped = surf__key_len_in_bytes <= length_of_condensed_digit_key;
#     }else{
#       surf__flag_bypass_if_same_key = surf__key_len_in_bytes <= key_size_to_insert; // whether to skip RDF checking if searding key is the same as the next greater key in the SuRF
#       surf__flag_allow_range_boundary_overlapped = surf__key_len_in_bytes <= key_size_to_insert;
#     }
# }

def set_B_E_list_to_task(
        params: dict,
        B_list: list = [256],
        E_list: list = [16],
        tasks: list = ["./simple_example"]
):
    for key, values in params.items():
        tasks = [" ".join([task, key, str(value)]) for value in values for task in tasks]

    task_tmp = []
    for i, (B, E) in enumerate(zip(B_list, E_list)):
        task_tmp.extend([f"{task} -B {B} -E {E}" for task in tasks])
    tasks = task_tmp

    return tasks

tasks = set_B_E_list_to_task(params = params)


def run_with_RD_sel(
        exp_code: chr = '4',
        RD_list: list = [1, 5, 10, 50, 100, 500, 900],
        sel_list: list = [0.001],
        tasks: list = tasks,
        workload_file_code: int = None,
):
    # Make the length the same
    if len(sel_list) == 1:
        sel_list = sel_list * len(RD_list)
    else:
        RD_list = RD_list * len(sel_list)

    for i, task_orig in enumerate(tasks):
        for j, (RD, sel) in enumerate(zip(RD_list, sel_list)):
            file_code = exp_code+str(i+1)+str(j+1)
            if workload_file_code == None:
                workload_file_code = file_code
            log_file_code = file_code
            task = task_orig + f" --RD {RD} --selectivity {sel} --workload_filename workload/workload{workload_file_code}.txt > log{log_file_code}"
            print(task)
            os.system(task)

def run_tasks(tasks: list):
    for task in tasks:
        print(task)
        os.system(task)


def gen_insertion_workload(
        insert: int,
        rd: int, 
        sel: float, 
        E: int,
        rd_threshold: float,
        key_size: int = -1,
        using_string_key: bool = False,
        proportional_to_zero_result_point: float = 1,
        n_pq: int = 10000,
):
    #task = "./K-V-Workload-Generator-master/load_gen" + f" --insert={insert} --range_delete={rd} --range_delete_selectivity={sel} --entry_size={E} --range_delete_threshold={rd_threshold} --key_size={key_size} --using_string_key={using_string_key} --proportional_to_zero_result_point={proportional_to_zero_result_point} --point_query={n_pq}"
    task = "./K-V-Workload-Generator-master/load_gen" + f" --insert={insert} --range_delete={rd} --range_delete_selectivity={sel} --entry_size={E} --range_delete_threshold={rd_threshold} --key_size={key_size} --using_string_key={using_string_key} --zero_result_point_lookup_proportion={proportional_to_zero_result_point} --point_query={n_pq}"
    print(task)
    os.system(task)

def gen_PQ_workload(
        file_path: int,
        # number_of_PQ: int,
        number_of_PQ_on_existing_keys: int,
        number_of_PQ_on_historic_existing_keys: int,
        number_of_PQ_on_currently_deleted_keys: int,
        number_of_PQ_on_currently_non_inserted_keys: int,
        using_string_key: bool = False,
):
    # task =  "gen_pq_workload/main_gen_workload" + f" --workload_filename {file_path} --number_of_PQ {number_of_PQ}"
    task =  "gen_pq_workload/main_gen_workload" + f" --workload_filename {file_path}  --number_of_PQ_on_existing_keys {number_of_PQ_on_existing_keys} --number_of_PQ_on_historic_existing_keys {number_of_PQ_on_historic_existing_keys} --number_of_PQ_on_currently_deleted_keys {number_of_PQ_on_currently_deleted_keys} --number_of_PQ_on_currently_non_inserted_keys {number_of_PQ_on_currently_non_inserted_keys} --using_string_key {using_string_key}"
    print(task)
    os.system(task)

def get_task_with_permuting_parameters(
        tasks: list = [],
        param_dict: dict = {},
):
    for k,v_list in param_dict.items():
        for v in v_list:
            task_tmp = []
            task_tmp.extend([f"{k} {v}" for task in tasks])
        tasks = task_tmp
    return tasks


def get_task_with_parallelling_parameters(
        tasks: list,
        param_dict: dict,
):
    tmp_tasks = []
    parallel_param = []
    for i_k, (k, v_list) in enumerate(param_dict.items()):
        for i_v, v in enumerate(v_list):
            if i_k == 0:
                parallel_param.append(f"{k} {v}")
            else:
                parallel_param[i_v] += f" {k} {v}"

    for task in tasks:
        for p_param in parallel_param:
            tmp_tasks.append(task + f" {p_param}")
    return tmp_tasks
        
#run_with_RD_sel('4', RD_list=[0, 1, 5, 10, 50, 100, 500, 900], sel_list=[0.001])
#run_with_RD_sel('5', RD_list=[100], sel_list=[0.0001, 0.0005, 0.001, 0.005, 0.009])
#run_with_RD_sel('6', RD_list=[10], sel_list=[0.001, 0.005, 0.009, 0.01, 0.05, 0.09])
#run_with_RD_sel('7', RD_list=[50], sel_list=[0.001, 0.005, 0.009, 0.01])
#run_with_RD_sel('8', RD_list=[50], sel_list=[0.001, 0.005, 0.009, 0.01])
#run_with_RD_sel('9', RD_list=[50], sel_list=[0.001, 0.005, 0.009, 0.01])
#run_with_RD_sel('7', RD_list=[0], sel_list=[0.001, 0.005, 0.009, 0.01])
#run_with_RD_sel('8', RD_list=[0], sel_list=[0.001, 0.005, 0.009, 0.01])
#run_with_RD_sel('9', RD_list=[0], sel_list=[0.001, 0.005, 0.009, 0.01])
# run_with_RD_sel('10', RD_list=[100], sel_list=[0.001, 0.005, 0.009])
# run_with_RD_sel('11', RD_list=[100], sel_list=[0.001, 0.005, 0.009])
#run_with_RD_sel('12', RD_list=[1000], sel_list=[0.0001, 0.0005, 0.0009])
#run_with_RD_sel('13', RD_list=[1000], sel_list=[0.0001, 0.0005, 0.0009])

# from copy import deepcopy
# params2 = deepcopy(params)
# params2["-i"] = [1000000]
# params2["-P"] = [16]
# params2["-T"] = [4]
# params2["--insert_before_range_delete"] = [0.9999]
# params2["--run_pq_during_insertion_interval"] = [20]
    
# tasks2 = set_B_E_list_to_task(params2, B_list = [4], E_list = [1024])
# #run_with_RD_sel('14', RD_list=[100], sel_list=[0.001, 0.005, 0.01], tasks=tasks2)
# #run_with_RD_sel('15', RD_list=[1000], sel_list=[0.0001, 0.0005, 0.001], tasks=tasks2)


params3 = deepcopy(params)
params3["-P"] = [16]
params3["-T"] = [4]
params3["--insert_before_range_delete"] = [0.999]
# params3["--insert_before_range_delete"] = [0.5]
# params3["--insert_before_range_delete"] = [0.8]
# params3["--insert_before_range_delete"] = [0.7]
# params3["--run_pq_during_insertion_interval"] = [20]

# params3["--gen_workload"] = [1]
params3["--gen_workload"] = [0]
params3["--load_pq_workload"] = [1]
#params3["-i"] = [200000]
params3["-i"] = [100000]
# params3["-i"] = [10000]
params3["--run_pq_during_insertion_interval"] = [200000000]
params3["--skip_reading_RD_blocks"] = [1]
# params3["--gen_workload"] = [0]
# params3["-i"] = [6400]
# params3["--insert_before_range_delete"] = [0.9]
# params3["--run_pq_during_insertion_interval"] = [200000]
params3["--system_check_test_on_all_PQ"] = [0] # for debugging, 0: off, 1: on
# params3["--system_check_test_on_all_PQ"] = [1] # for debugging, 0: off, 1: on
params3["--use_surf_base"] = [0] 
params3["--surf_base_store_key_to_k_diff"] = [1] 


B_list = [4]
E_list = [1024]
E = E_list[0]
# rd_list = [100, 100, 100]
# sel_list = [0.001, 0.005, 0.01]
rd_list = [100]
sel_list = [0.001]
#rd_list = [100, 100, 100, 100]
#sel_list = [0.0005, 0.001, 0.0015, 0.002]
workload_filename_list = [
    f"workload/workload6111.txt",
#    f"workload/workload6112.txt",
#    f"workload/workload6113.txt",
#    f"workload/workload6114.txt",
]

   


params3["--surf_use_condensed_digit_key"] = [0]
# params3["--surf_use_condensed_digit_key"] = [1]
# params3["-i"] = [1000]
# sel_list = [0.1,0.1,0.1]
# rd_list = [10,10,10]
# params3["--show_surf_compaction_info"] = [1] 
params3[ "--surf__key_len_in_bytes"] = [13]
# params3["--surf__key_len_in_bytes"] = [6]
# params3[ "--surf__key_len_in_bytes"] = [3]
# params3[ "--skip_reading_RD_blocks"] = [0]
params3["--flag_skip_compaction_trivial_move"] = [1]
# params3[ "--surf__key_len_in_bytes"] = [5]
params3["--skip_reading_RD_blocks"] = [1]
# params3[ "--show_surf_compaction_info"] = [1]
params3["--max_open_files"] = [1]
# params3["--number_of_PQ"] = [5000] # -1: for testing on all PQ, >=0 : sample #PQ from all PQ
params3["--number_of_PQ_on_existing_keys"] = [1] # -1: for testing on all PQ, >=0 : sample #PQ from all PQ
params3["--number_of_PQ_on_historic_existing_keys"] = [1] # -1: for testing on all PQ, >=0 : sample #PQ from all PQ
#params3["--number_of_PQ_on_currently_deleted_keys"] = [100000] # -1: for testing on all PQ, >=0 : sample #PQ from all PQ
params3["--number_of_PQ_on_currently_deleted_keys"] = [1] # -1: for testing on all PQ, >=0 : sample #PQ from all PQ
# params3["--number_of_PQ_on_currently_deleted_keys"] = [500000] # -1: for testing on all PQ, >=0 : sample #PQ from all PQ
params3["--number_of_PQ_on_currently_non_inserted_keys"] = [100000] # -1: for testing on all PQ, >=0 : sample #PQ from all PQ

if True:
# if False:
    for rd, sel, workload_filename in zip(rd_list, sel_list, workload_filename_list):
        print("Gen I/RD workload")
        gen_insertion_workload(
                insert=params3["-i"][0],
                rd=rd,
                sel=sel,
                E=E,
                rd_threshold=params3["--insert_before_range_delete"][0],
                key_size=params3["--key_size_to_insert"][0],
                using_string_key=params3["--use_string_key"][0],
                proportional_to_zero_result_point=1,
                n_pq = 100000*45,
                #n_pq = 100000,
                #n_pq = 0,
        )
        task = f"mv workload.txt {workload_filename}"
        os.system(task)
        
        print("Gen PQ workload")
        for number_of_PQ_on_currently_non_inserted_keys in [100000, 200000, 300000, 400000, 500000]:
            gen_PQ_workload(
                    file_path=workload_filename,
                    # number_of_PQ=params3["--number_of_PQ"][0],
                    number_of_PQ_on_existing_keys=params3["--number_of_PQ_on_existing_keys"][0],
                    number_of_PQ_on_historic_existing_keys=params3["--number_of_PQ_on_historic_existing_keys"][0],
                    number_of_PQ_on_currently_deleted_keys=params3["--number_of_PQ_on_currently_deleted_keys"][0],
                    #number_of_PQ_on_currently_non_inserted_keys=params3["--number_of_PQ_on_currently_non_inserted_keys"][0],
                    number_of_PQ_on_currently_non_inserted_keys=number_of_PQ_on_currently_non_inserted_keys,
                    using_string_key=params3["--use_string_key"][0],
            )

        task = f"grep \"^Q\" {workload_filename} > workload/pq_workload_on_currently_non_inserted_keys.txt"
        os.system(task)
        task = f"sed -i '/^Q/d' {workload_filename}"
        os.system(task)

        task = f"cd workload; python3 splitting_pq_workload.py"
        os.system(task)
        



# ["NONE_DUMMY,NONE_CACHE_RANGETOMBSTONE_TRACING,NONE,NONE2,PLRDF,SPLIT_PLRDF,TOP_LEVEL_RDF,SKYLINE_RDF,SuRF_LF_RDF,SuRF_LF_SPLIT_RDF,NONE_DUMMY"]
# rdf_types = ["NONE", "PLRDF", "SPLIT_PLRDF", "TOP_LEVEL_RDF", "SKYLINE_RDF", "SuRF_LF_RDF", "SuRF_LF_SPLIT_RDF"]
# rdf_types = ["NONE", "PLRDF", "SPLIT_PLRDF", "TOP_LEVEL_RDF", "SKYLINE_RDF", "SuRF_LF_RDF", "SuRF_LF_SPLIT_RDF"]
rdf_types = [
    # {"--using_rdf_types": ["NONE"], },
    {"--using_rdf_types": ["NONE_CACHE_RANGETOMBSTONE_TRACING"], },
    # {"--using_rdf_types": ["PLRDF"], },
    # {"--using_rdf_types": ["SPLIT_PLRDF"], },
    {"--using_rdf_types": ["PLRDF_STRING_KEY"],  "--skip_reading_RD_blocks":[0]},
    {"--using_rdf_types": ["SPLIT_PLRDF_STRING_KEY"],  "--skip_reading_RD_blocks":[0]},
    # {"--using_rdf_types": ["TOP_LEVEL_RDF"], },
    {"--using_rdf_types": ["TOP_LEVEL_RDF_STRING_KEY"],  "--skip_reading_RD_blocks":[0]},
    # {"--using_rdf_types": ["SKYLINE_RDF"], },
    {"--using_rdf_types": ["SuRF_LF_RDF"]},
    {"--using_rdf_types": ["SuRF_LF_SPLIT_RDF"]},
    {"--using_rdf_types": ["SuRF_LF_RDF"], "--surf__key_len_in_bytes": [3], "--use_surf_base": [1], "--skip_reading_RD_blocks":[0]},
    {"--using_rdf_types": ["SuRF_LF_SPLIT_RDF"], "--surf__key_len_in_bytes": [3], "--use_surf_base": [1], "--skip_reading_RD_blocks":[0]},
    # {"--using_rdf_types": ["SuRF_LF_RDF"], "--use_surf_base": [1], "--surf_base_store_key_to_k_diff": [1], "--skip_reading_RD_blocks":[0], "--surf_use_condensed_digit_key":[1]}, #9
    # {"--using_rdf_types": ["SuRF_LF_SPLIT_RDF"], "--use_surf_base": [1], "--surf_base_store_key_to_k_diff": [1], "--skip_reading_RD_blocks":[0], "--surf_use_condensed_digit_key":[1]}, #10
    
    {"--using_rdf_types": ["SuRF_LF_RDF"], "--surf__key_len_in_bytes": [4], "--use_surf_base": [1], "--skip_reading_RD_blocks":[0]}, #11
    {"--using_rdf_types": ["SuRF_LF_SPLIT_RDF"], "--surf__key_len_in_bytes": [4], "--use_surf_base": [1], "--skip_reading_RD_blocks":[0]}, #12
    # {"--using_rdf_types": ["SuRF_LF_RDF"], "--surf__key_len_in_bytes": [5], "--skip_reading_RD_blocks":[0]}, #13
    # {"--using_rdf_types": ["SuRF_LF_SPLIT_RDF"], "--surf__key_len_in_bytes": [5], "--skip_reading_RD_blocks":[0]}, #14
    # {"--using_rdf_types": ["SuRF_LF_RDF"], "--use_surf_base": [1], "--surf_base_store_key_to_k_diff": [2], "--skip_reading_RD_blocks":[0], "--surf_use_condensed_digit_key":[1]}, #15
    # {"--using_rdf_types": ["SuRF_LF_SPLIT_RDF"], "--use_surf_base": [1], "--surf_base_store_key_to_k_diff": [2], "--skip_reading_RD_blocks":[0], "--surf_use_condensed_digit_key":[1]}, #16

    # check total_time & insertion time
    #{"--using_rdf_types": ["SuRF_LF_RDF"], "--use_surf_base": [1], "--surf_base_store_key_to_k_diff": [2], "--skip_reading_RD_blocks":[0], "--surf_use_condensed_digit_key":[1]}, #17
    #{"--using_rdf_types": ["SuRF_LF_SPLIT_RDF"], "--use_surf_base": [1], "--surf_base_store_key_to_k_diff": [2], "--skip_reading_RD_blocks":[0], "--surf_use_condensed_digit_key":[1]}, #18
    #{"--using_rdf_types": ["NONE_CACHE_RANGETOMBSTONE_TRACING"], }, #19
    #{"--using_rdf_types": ["PLRDF"], }, #20
    #{"--using_rdf_types": ["SPLIT_PLRDF"], }, #21
    #{"--using_rdf_types": ["TOP_LEVEL_RDF"], }, #22
    #{"--using_rdf_types": ["SKYLINE_RDF"], }, #23

    
    #{"--using_rdf_types": ["SuRF_LF_RDF"]}, #24
    #{"--using_rdf_types": ["SuRF_LF_SPLIT_RDF"]}, #25
    #{"--using_rdf_types": ["SuRF_LF_RDF"], "--surf__key_len_in_bytes": [3], "--skip_reading_RD_blocks":[0]}, #26
    #{"--using_rdf_types": ["SuRF_LF_SPLIT_RDF"], "--surf__key_len_in_bytes": [3], "--skip_reading_RD_blocks":[0]}, #27
]
# rdf_types = {
#     "NONE": {},
#     "PLRDF": {},
#     "SPLIT_PLRDF": {},
#     "TOP_LEVEL_RDF": {},
#     "SKYLINE_RDF": {},
#     "SuRF_LF_RDF": {},
#     "SuRF_LF_SPLIT_RDF": {},
#     # "SuRF_LF_RDF": {"--surf__key_len_in_bytes": [3]},
#     # "SuRF_LF_SPLIT_RDF": {"--surf__key_len_in_bytes": [3]}
# }


# for number_of_PQ_on_currently_non_inserted_keys in [100000, 200000, 300000, 400000, 500000]:
#     params3["--number_of_PQ_on_currently_non_inserted_keys"] = [number_of_PQ_on_currently_non_inserted_keys]
#     # mkdir -p 
#     # os.system(f"mkdir -p log_{number_of_pq_on_currently_deleted_keys}")

#     # Construct the full path
#     log_dir = f"saved_result_digit_key_size_{params3['--key_size_to_insert'][0]}/log_{number_of_PQ_on_currently_non_inserted_keys}"

#     # Recursively create the directory
#     os.makedirs(log_dir, exist_ok=True)

#     # for i_rdf, (rdf_type, local_param) in enumerate(rdf_types.items()):
#     for i_rdf, rdf_param in enumerate(rdf_types):
#         # if i_rdf > 0:
#         #    break
#         # if i_rdf < 5:
#         # if i_rdf < 6:
#         # if i_rdf < 7:
#         # if i_rdf < 8:
#         # if i_rdf < 9:
#         # # if i_rdf < 10:
#         # # if i_rdf < 11:
#         #    continue
#         # if i_rdf > 10 and i_rdf < 15:
#         #     continue
#         # if i_rdf < 17:
#         # if i_rdf < 21:
#         #   continue

#         test_num = 61 + i_rdf
#         # params3["--using_rdf_types"] = [rdf_type]
#         params3_local = deepcopy(params3)
#         params3_local.update(rdf_param) # delta changes for different rdf_type
#         # print(params3_local)
#         tasks3 = set_B_E_list_to_task(params3_local, B_list = B_list, E_list = E_list)
#         tasks3 = get_task_with_parallelling_parameters(tasks=tasks3, param_dict={"--RD":rd_list, "--selectivity":sel_list,
#                                                                             "--workload_filename": workload_filename_list,
#                                                                             "--logging_filename": [
#                                                                               f"pq_result/{log_dir}/logging{test_num}11.txt",
#                                                                               #f"pq_result/logging{test_num}12.txt",
#                                                                               #f"pq_result/logging{test_num}13.txt",
#                                                                               #f"pq_result/logging{test_num}14.txt",
#                                                                             ],
#                                                                             ">":[
#                                                                                 f"{log_dir}/log{test_num}11",
#                                                                                 #f"{log_dir}/log{test_num}12",
#                                                                                 #f"{log_dir}/log{test_num}13",
#                                                                                 #f"{log_dir}/log{test_num}14",
#                                                                             ]
#                                                                             })
#         run_tasks(tasks3)
#         # run_with_RD_sel(str(test_num), RD_list=[100], sel_list=[0.001, 0.005, 0.01], tasks=tasks3)

