# LSMRangeDeletes
Making range deletes better in LSMs.

## Building the Project
1. **Build RocksDB Core**:
   Go to `./development/rocksdb/` and run:
   ```bash
   make clean             # Run if DB file relation is broken
   cp ../make_config ./
   make static_lib -j8
   ```

2. **Build Examples**:
   Go to experiment directory `./development/rocksdb/self_rd_e[number]_xxx` and run:
   ```bash
   make simple_example -j8
   ```

## Running Experiments
Run the `simple_example` with specific parameters:
```bash
python run_task_256B_string_key.py
```

## Progress
The project has implemented several advanced range delete filter mechanisms in RocksDB to optimize lookup performance in LSM-trees under frequent range deletions.

### Implemented Filters
1.  **RaTer-Vec (Range Delete Filter with Vector / PLRDF_STRINGKEY)**:
    - **Concept**: A per-level filter that tracks range tombstones at each level of the LSM-tree.
    - **Mechanism**: Manages a set of non-overlapping range tombstones `[start, end)` per level. It uses a specialized `isEntryAlive` check to determine if a key is covered by any range tombstone at a given level.
    - **Optimization**: Features automatic **Adjacent Range Merging** (e.g., merging `[10, 20)` and `[20, 30)` into `[10, 30)`) to minimize filter size and lookup overhead.

2.  **RaTer-Vec-Frag (Range Delete Filter with Vector and Fragmentation operation / SPLIT_PLRDF_STRINGKEY)**:
    - An extension of PLRDF that supports key-space splitting (by keeping all the newly arrived point queries that fall within older range tombstones) to further refine range tracking within a level, improving filtering efficiency on range deleted keys.

3. **Top-Level-RaTer (Top Level Range Delete Filter / TOP_LEVEL_RDF)**:
    - **Concept**: A variant of RaTer-Vec-Frag that keeps all the rangetombstones on level 1 in the buffer. Theoretically, the fastest filter for point queries on range deleted keys.
    
4. **RaTer-Trie (Range Delete Filter with Trie / SuRF_RDF)**:
    - **Concept**: A trie-based range delete filter which utilized LOUDS representation to compress the trie.
    - **Mechanism**: Uses a compressed trie to store range tombstones inside the buffer for each file. It allows for efficient "is deleted" checks by traversing the trie. 
    - **Optimization**: Compresses the trie to store range tombstones.

5. **RaTer-Trie-Frag (Range Delete Filter with Trie and Fragmentation operation / SPLIT_SuRF_RDF)**:
    - An extension of RaTer-Trie that supports key-space splitting (by keeping all the newly arrived point queries that fall within older range tombstones) to further refine range tracking within a level, improving filtering efficiency on range deleted keys.It reaches the balance between memory footprint and speed.

6.  **SkyLineRDF (Skyline Range Delete Filter)**:
    - **Concept**: A more sophisticated filter that accounts for the sequence numbers (timestamps) of range deletions.
    - **Mechanism**: Uses a "skyline" approach to track the maximum sequence number for any given key range. It allows for precise "is alive" checks by comparing a point key's sequence number against the skyline's maximum at that position.
    - **Optimization**: Efficiently aggregates overlapping range tombstones into a minimal set of non-overlapping "skyline" segments.


### Core Integration Refinements
- **SST Boundary Tracking**: Modified `FileMetaData` and the MANIFEST to track precise point key and range tombstone boundaries separately, enabling accurate clipping of filters at the SST level.
- **Flush-time Aggregation**: Range tombstones from multiple memtables are consolidated during the flush process to ensure consistent RDF state in Level 0.
- **Compaction Management**: Implemented a "Gather & Shift/Clip" strategy to efficiently update RDF metadata during compaction, ensuring that range deletes are correctly propagated or clipped across SST boundaries.


