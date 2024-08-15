# LSMRangeDeletes
Making range deletes better in LSMs

To Run:
    1. Go to directory ./development/rocksdb/
    2. run: 
        i) make clean             (Make sure running this command whenever your DB file relation has broken)
        ii) make static_lib -j9
    3. Go to directory ./development/rocksdb/self_RD
    4. run: make simple_example -j9
    5. run workload: 
	./simple_example -i 100000 --RD 100 -P 64 -B 4 -E 1024 -T 5 --selectivity 0.001 --workload_filename "workload/workload1.txt" --insert_before_range_delete 0.9 --gen_workload 0
    6. for gdb debugging: gdb --args 
	./simple_example -i 100000 --RD 100 -P 64 -B 4 -E 1024 -T 5 --selectivity 0.001 --workload_filename "workload/workload1.txt" --insert_before_range_delete 0.9 --gen_workload 1


TODO 
- [x] Implement PLRDF, Split-PLRDF, TopLevel RDF, Skyline RDF
- [ ] Deal with rare boundary case by checking whether the end of a file range is indeed the RD boundary
- [ ] Improve timing of the reading
- [ ] Normalizing results
- [ ] Run experiments
- [ ] Complete paper
