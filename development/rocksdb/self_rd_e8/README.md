# RocksDB Range Delete Baseline & Verification (self_rd_e8)

**Author:** YuCheng
**Date:** 12/13/2025

## Purpose
The purpose of `self_rd_e8` is to evaluate how range tombstones affect query performance, specifically on never-inserted keys. This experiment uses 12B int key. The workload involves generating data with range tombstones inserted in the last percentage (`range_delete_threshold`) of the operation sequence. Data is fully inserted and allowed to settle before performing query experiments.

The system configures RocksDB to allow only 1 SSTable to remain at Level 0. This constraint prevents early merging and forces SSTables containing range tombstones to propagate down the LSM tree, allowing us to observe their impact at deeper levels.

The query workload covers four scenarios:
1.  Query on existing keys.
2.  Query on historically inserted keys (keys that existed but were overwritten or deleted).
3.  Query on deleted keys.
4.  Query on never-inserted keys.


## Workload & Parameters

### Example Parameters
```bash
--insert=100000 --range_delete=100 --range_delete_selectivity=0.001 --entry_size=1024 --range_delete_threshold=0.5
```
*   **Key Size**: 12B int Key.
*   **Entry Size (E)**: 1KB (1024 bytes).
*   **Entries per Page (B)**: 4 (Block Size = 4KB).
*   **Buffer Size in Pages (P)**: 16.
*   **Selectivity (sel)**: 0.001.
*   **Max Open Files**: 1.
*   **Range Delete Selectivity (sel)**: 0.001.
*   **Range Delete (R)**: 100.
*   **Insert (I)**: 100,000.
*   **Total Size (Total)**: 100,000  = 100,000 entries (approx).

### Workload Steps
1.  **Step 1**: Perform 100,000 Inserts.
2.  **Step 2**: Perform 100 Inserts interleaved with 100 Range Deletes.
3.  **Step 3**: Perform point queries on never-inserted keys (range 100K to 500K).

---

## 1. Prerequisites

Ensure `zstd` is installed on your system, as it is required for compression in RocksDB.

```bash
sudo apt-get install zstd
# or
yum install zstd
```

## 2. Compilation

To compile `simple_example`, which is the main binary used by the python scripts:

```bash
# Copy the make configuration to the RocksDB root (adjust paths if necessary)
cp ../make_config.mk ../../

# specific commands to compile simple_example
make simple_example -j8
```

## 3. Running Experiments

Experiments are orchestrated via Python scripts found in this directory.

### Run with 12B int Keys
```bash
python run_task.py
```

<!-- ### Run with 256B String Keys
```bash
python run_task_256B_string_key.py
``` -->

## 4. Results and Output

The verification performance logs and statistics turn into output files located in directories named with the following pattern:

`saved_result_<int/string>_key_size_<KeySize>/log_<NumPQ>/`

For example:
`saved_result_int_key_size_12/log_100000/log6111`

## 5. Directory Structure Explanation

*   **`K-V-Workload-Generator-master`**: Contains the logic and binary for generating Insertion workloads (`load_gen`).
*   **`workload`**: Stores the generated insertion and query workload files (text format).
*   **`gen_pq_workload`**: Source code (`main_gen_workload.cpp`) for generating Point Query (PQ) workload files.
*   **`utils`**: Utility headers and scripts used to:
    *   Generate workloads (`gen_pq`, `gen_insertion`).
    *   Handle file I/O (`read_write`).
    *   Run the main verification loop and performance timing (`utils_run_verification.h`).
    *   Calculate and print statistics.
*   **`output_statistics`**: Contains system-level statistics collected during the running of each workload.
*   **`saved_result_*`**: The main output directories containing:
    *   PQ performance statistics.
    *   Verification results (correctness checks).
    *   Timing logs.
*   **`tools_for_saved_result`**: Scripts for parsing results from text files to CSV.
    *   **Usage**: Copy `parsing_log_file.py` and `sed_command` into the `saved_result_*` directory (or wherever your logs are).
    *   **Run**: Execute `./sed_command` to process the logs.
    *   **Customize**: You can modify the parsing logic in `parsing_log_file.py` or the batch commands in `sed_command` to suit your specific data extraction needs.
