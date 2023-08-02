//  Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).
//
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#pragma once

#include <atomic>
#include <string>
#include <unordered_map>
#include <vector>

#include <iostream>
#include <iomanip>

//Self Added
#include <tuple>


#include "cache/cache_reservation_manager.h"
#include "db/memtable_list.h"
#include "db/table_cache.h"
#include "db/table_properties_collector.h"
#include "db/write_batch_internal.h"
#include "db/write_controller.h"
#include "options/cf_options.h"
#include "rocksdb/compaction_job_stats.h"
#include "rocksdb/db.h"
#include "rocksdb/env.h"
#include "rocksdb/options.h"
#include "trace_replay/block_cache_tracer.h"
#include "util/hash_containers.h"
#include "util/thread_local.h"

namespace ROCKSDB_NAMESPACE {

  //Self Added Start
  using pll = std::pair<long long, long long>; //[start, end)
  using t3ll = std::tuple<long long, long long, long long>; //([start, end), time)

  // class PerlevelRangeDeleteFilterByVector {  
  class PLRDF {  
    private:
      std::unordered_map<uint64_t, std::vector<pll>> rd_filter_level0; //for level 0, (file_num, RD_list), FileMetaData* -> fd .GetNumber();

      std::vector<std::vector<pll>> rd_filter; //for level > 0, list of range delete (start, end), all entries are non-overlapping
      std::vector<int> numbers_of_ranges_in_RDF_log; //for level > 0, number of ranges in RDF
      
      void addRangeDelete_internal(uint level, std::vector<pll> &range_delete_list_in);
      std::vector<pll> sortAndMerge(std::vector<pll> &range_delete_list_in);
      void addRangeDelete(std::vector<pll> &range_delete_list, std::vector<pll> &range_delete_list_in);
      void addRangeDelete(std::vector<pll> &range_delete_list, long long start, long long end);
      void print_internal();


      /*
       * adjust range deletes as per the compaction
       */
      void adjustRangeDeletesForLevel0Input(uint olevel, std::vector<uint64_t> file_numbers);
      void adjustRangeDeletes(uint clevel, uint olevel, std::vector<std::pair<long long, long long>> one_level_compaction_file_boundaries);

    public:
      // std::vector<pll> getRangeDeleteList();
      void insertRangeDeleteToLevel0(uint64_t file_num, std::vector<pll> &range_delete_list_in, std::vector<uint64_t> exist_level0_file_nums);

      void addRangeDelete(uint level, long long start, long long end);
      void addRangeDelete(uint level, std::vector<pll> &range_delete_list_in);
      void shiftRDFToOutputLevel(std::vector<std::tuple<int, int, std::vector<pll>, std::vector<uint64_t>>> *file_meta_data_vectors);
      void deleteLastLevelIfEqualsBottomLevel(uint bottom_level);
      // void deleteRDFAssociatedWithFilesAtCurrentLevel(std::tuple<int, const std::vector<FileMetaData*>*> *file_meta_data);
      void deleteRDFAssociatedWithFilesAtCurrentLevel(std::tuple<int, std::vector<pll>, std::vector<uint64_t>> *file_meta_data);

      std::vector<pll> getLevelRanges(int outlevel);
      void setLevelRanges(std::vector<pll> level_ranges_in, int outlevel);

      int getNumberOfTotalLevels();
      int getNumberOfTotalRanges();

      void printLevel0();
      void print();

      bool isEntryAlive(uint level, long long key);


      void splitRangesOnLevel(uint level, std::vector<long long> keys);
      // int getRangeDeleteCount();

      void logCurrentTotalNumbersOfRanges();
      std::vector<int> getNumbersOfRangesInRDFLog();
  };

  class SKyLineRDF {
    private:
      std::vector<t3ll> range_delete_list_in;

    public:      
      void addRangeDelete(std::vector<t3ll> &range_delete_list_in);
      bool isEntryAlive(long long key);
      void print();
  };
  //Self Added End



class Version;
class VersionSet;
class VersionStorageInfo;
class MemTable;
class MemTableListVersion;
class CompactionPicker;
class Compaction;
class InternalKey;
class InternalStats;
class ColumnFamilyData;
class DBImpl;
class LogBuffer;
class InstrumentedMutex;
class InstrumentedMutexLock;
struct SuperVersionContext;
class BlobFileCache;
class BlobSource;

extern const double kIncSlowdownRatio;
// This file contains a list of data structures for managing column family
// level metadata.
//
// The basic relationships among classes declared here are illustrated as
// following:
//
//       +----------------------+    +----------------------+   +--------+
//   +---+ ColumnFamilyHandle 1 | +--+ ColumnFamilyHandle 2 |   | DBImpl |
//   |   +----------------------+ |  +----------------------+   +----+---+
//   | +--------------------------+                                  |
//   | |                               +-----------------------------+
//   | |                               |
//   | | +-----------------------------v-------------------------------+
//   | | |                                                             |
//   | | |                      ColumnFamilySet                        |
//   | | |                                                             |
//   | | +-------------+--------------------------+----------------+---+
//   | |               |                          |                |
//   | +-------------------------------------+    |                |
//   |                 |                     |    |                v
//   |   +-------------v-------------+ +-----v----v---------+
//   |   |                           | |                    |
//   |   |     ColumnFamilyData 1    | | ColumnFamilyData 2 |    ......
//   |   |                           | |                    |
//   +--->                           | |                    |
//       |                 +---------+ |                    |
//       |                 | MemTable| |                    |
//       |                 |  List   | |                    |
//       +--------+---+--+-+----+----+ +--------------------++
//                |   |  |      |
//                |   |  |      |
//                |   |  |      +-----------------------+
//                |   |  +-----------+                  |
//                v   +--------+     |                  |
//       +--------+--------+   |     |                  |
//       |                 |   |     |       +----------v----------+
// +---> |SuperVersion 1.a +----------------->                     |
//       |                 +------+  |       | MemTableListVersion |
//       +---+-------------+   |  |  |       |                     |
//           |                 |  |  |       +----+------------+---+
//           |      current    |  |  |            |            |
//           |   +-------------+  |  |mem         |            |
//           |   |                |  |            |            |
//         +-v---v-------+    +---v--v---+  +-----v----+  +----v-----+
//         |             |    |          |  |          |  |          |
//         | Version 1.a |    | memtable |  | memtable |  | memtable |
//         |             |    |   1.a    |  |   1.b    |  |   1.c    |
//         +-------------+    |          |  |          |  |          |
//                            +----------+  +----------+  +----------+
//
// DBImpl keeps a ColumnFamilySet, which references to all column families by
// pointing to respective ColumnFamilyData object of each column family.
// This is how DBImpl can list and operate on all the column families.
// ColumnFamilyHandle also points to ColumnFamilyData directly, so that
// when a user executes a query, it can directly find memtables and Version
// as well as SuperVersion to the column family, without going through
// ColumnFamilySet.
//
// ColumnFamilySet points to the latest view of the LSM-tree (list of memtables
// and SST files) indirectly, while ongoing operations may hold references
// to a current or an out-of-date SuperVersion, which in turn points to a
// point-in-time view of the LSM-tree. This guarantees the memtables and SST
// files being operated on will not go away, until the SuperVersion is
// unreferenced to 0 and destoryed.
//
// The following graph illustrates a possible referencing relationships:
//
// Column       +--------------+      current       +-----------+
// Family +---->+              +------------------->+           |
//  Data        | SuperVersion +----------+         | Version A |
//              |      3       |   imm    |         |           |
// Iter2 +----->+              |  +-------v------+  +-----------+
//              +-----+--------+  | MemtableList +----------------> Empty
//                    |           |   Version r  |  +-----------+
//                    |           +--------------+  |           |
//                    +------------------+   current| Version B |
//              +--------------+         |   +----->+           |
//              |              |         |   |      +-----+-----+
// Compaction +>+ SuperVersion +-------------+            ^
//    Job       |      2       +------+  |                |current
//              |              +----+ |  |     mem        |    +------------+
//              +--------------+    | |  +--------------------->            |
//                                  | +------------------------> MemTable a |
//                                  |          mem        |    |            |
//              +--------------+    |                     |    +------------+
//              |              +--------------------------+
//  Iter1 +-----> SuperVersion |    |                          +------------+
//              |      1       +------------------------------>+            |
//              |              +-+  |        mem               | MemTable b |
//              +--------------+ |  |                          |            |
//                               |  |    +--------------+      +-----^------+
//                               |  |imm | MemtableList |            |
//                               |  +--->+   Version s  +------------+
//                               |       +--------------+
//                               |       +--------------+
//                               |       | MemtableList |
//                               +------>+   Version t  +-------->  Empty
//                                 imm   +--------------+
//
// In this example, even if the current LSM-tree consists of Version A and
// memtable a, which is also referenced by SuperVersion, two older SuperVersion
// SuperVersion2 and Superversion1 still exist, and are referenced by a
// compaction job and an old iterator Iter1, respectively. SuperVersion2
// contains Version B, memtable a and memtable b; SuperVersion1 contains
// Version B and memtable b (mutable). As a result, Version B and memtable b
// are prevented from being destroyed or deleted.

// ColumnFamilyHandleImpl is the class that clients use to access different
// column families. It has non-trivial destructor, which gets called when client
// is done using the column family
class ColumnFamilyHandleImpl : public ColumnFamilyHandle {
 public:
  // create while holding the mutex
  ColumnFamilyHandleImpl(ColumnFamilyData* cfd, DBImpl* db,
                         InstrumentedMutex* mutex);
  // destroy without mutex
  virtual ~ColumnFamilyHandleImpl();
  virtual ColumnFamilyData* cfd() const { return cfd_; }

  virtual uint32_t GetID() const override;
  virtual const std::string& GetName() const override;
  virtual Status GetDescriptor(ColumnFamilyDescriptor* desc) override;
  virtual const Comparator* GetComparator() const override;

 private:
  ColumnFamilyData* cfd_;
  DBImpl* db_;
  InstrumentedMutex* mutex_;
};

// Does not ref-count ColumnFamilyData
// We use this dummy ColumnFamilyHandleImpl because sometimes MemTableInserter
// calls DBImpl methods. When this happens, MemTableInserter need access to
// ColumnFamilyHandle (same as the client would need). In that case, we feed
// MemTableInserter dummy ColumnFamilyHandle and enable it to call DBImpl
// methods
class ColumnFamilyHandleInternal : public ColumnFamilyHandleImpl {
 public:
  ColumnFamilyHandleInternal()
      : ColumnFamilyHandleImpl(nullptr, nullptr, nullptr),
        internal_cfd_(nullptr) {}

  void SetCFD(ColumnFamilyData* _cfd) { internal_cfd_ = _cfd; }
  virtual ColumnFamilyData* cfd() const override { return internal_cfd_; }

 private:
  ColumnFamilyData* internal_cfd_;
};

//Self Added
// using PL_RDF = PLRDF;
// holds references to memtable, all immutable memtables and version
struct SuperVersion {
  // Accessing members of this class is not thread-safe and requires external
  // synchronization (ie db mutex held or on write thread).
  ColumnFamilyData* cfd;
  MemTable* mem;
  MemTableListVersion* imm;
  Version* current;
  MutableCFOptions mutable_cf_options;
  // Version number of the current SuperVersion
  uint64_t version_number;
  WriteStallCondition write_stall_condition;

  // should be called outside the mutex
  SuperVersion() = default;
  ~SuperVersion();
  SuperVersion* Ref();
  // If Unref() returns true, Cleanup() should be called with mutex held
  // before deleting this SuperVersion.
  bool Unref();

  // call these two methods with db mutex held
  // Cleanup unrefs mem, imm and current. Also, it stores all memtables
  // that needs to be deleted in to_delete vector. Unrefing those
  // objects needs to be done in the mutex
  void Cleanup();
  void Init(ColumnFamilyData* new_cfd, MemTable* new_mem,
            MemTableListVersion* new_imm, Version* new_current);

  // The value of dummy is not actually used. kSVInUse takes its address as a
  // mark in the thread local storage to indicate the SuperVersion is in use
  // by thread. This way, the value of kSVInUse is guaranteed to have no
  // conflict with SuperVersion object address and portable on different
  // platform.
  static int dummy;
  static void* const kSVInUse;
  static void* const kSVObsolete;

  //Self Added

  // void storeRange2RDFTest(RangeTombstone tombstone){
  //   RDF_test.push_back(std::make_pair( std::stoll(tombstone.start_key_.ToString()), std::stoll(tombstone.end_key_.ToString()) ));
  // }

  // void printRDFTest(){
  //   std::cout << "SuperVision @column_family.h" << std::endl;
  //   for(auto x: RDF_test){
  //     std::cout << x.first << " " << x.second << std::endl;
  //   }
  //   std::cout << std::endl << std::endl;
  // }

  // void printRDFTest2(){
  //   std::cout << "SuperVision @column_family.h" << std::endl;
  //   for(auto x: RDF_test2){
  //     std::cout << x.first << " " << x.second << std::endl;
  //   }
  //   std::cout << std::endl << std::endl;
  // }

  // std::vector<std::pair<long long, long long>> getRDFTest(){return this->RDF_test;}

  // void setRDFTest(std::vector<std::pair<long long, long long>> RDF_test_in){this->RDF_test = RDF_test_in;}
  // void setRDFTest2(std::vector<std::pair<long long, long long>> RDF_test_in){this->RDF_test2 = RDF_test_in;}

 private:
  //Self Added
  // PL_RDF per_level_RDF; //Self Added, ranges don't split when inserts come//added by ychaung
  // std::vector<std::pair<long long, long long>> RDF_test, RDF_test2; //Self Added

  std::atomic<uint32_t> refs;
  // We need to_delete because during Cleanup(), imm->Unref() returns
  // all memtables that we need to free through this vector. We then
  // delete all those memtables outside of mutex, during destruction
  autovector<MemTable*> to_delete;
};

extern Status CheckCompressionSupported(const ColumnFamilyOptions& cf_options);

extern Status CheckConcurrentWritesSupported(
    const ColumnFamilyOptions& cf_options);

extern Status CheckCFPathsSupported(const DBOptions& db_options,
                                    const ColumnFamilyOptions& cf_options);

extern ColumnFamilyOptions SanitizeOptions(const ImmutableDBOptions& db_options,
                                           const ColumnFamilyOptions& src);
// Wrap user defined table properties collector factories `from cf_options`
// into internal ones in int_tbl_prop_collector_factories. Add a system internal
// one too.
extern void GetIntTblPropCollectorFactory(
    const ImmutableCFOptions& ioptions,
    IntTblPropCollectorFactories* int_tbl_prop_collector_factories);

class ColumnFamilySet;

// This class keeps all the data that a column family needs.
// Most methods require DB mutex held, unless otherwise noted
class ColumnFamilyData {
 public:
  ~ColumnFamilyData();

  // thread-safe
  uint32_t GetID() const { return id_; }
  // thread-safe
  const std::string& GetName() const { return name_; }

  // Ref() can only be called from a context where the caller can guarantee
  // that ColumnFamilyData is alive (while holding a non-zero ref already,
  // holding a DB mutex, or as the leader in a write batch group).
  void Ref() { refs_.fetch_add(1); }

  // UnrefAndTryDelete() decreases the reference count and do free if needed,
  // return true if this is freed else false, UnrefAndTryDelete() can only
  // be called while holding a DB mutex, or during single-threaded recovery.
  bool UnrefAndTryDelete();

  // SetDropped() can only be called under following conditions:
  // 1) Holding a DB mutex,
  // 2) from single-threaded write thread, AND
  // 3) from single-threaded VersionSet::LogAndApply()
  // After dropping column family no other operation on that column family
  // will be executed. All the files and memory will be, however, kept around
  // until client drops the column family handle. That way, client can still
  // access data from dropped column family.
  // Column family can be dropped and still alive. In that state:
  // *) Compaction and flush is not executed on the dropped column family.
  // *) Client can continue reading from column family. Writes will fail unless
  // WriteOptions::ignore_missing_column_families is true
  // When the dropped column family is unreferenced, then we:
  // *) Remove column family from the linked list maintained by ColumnFamilySet
  // *) delete all memory associated with that column family
  // *) delete all the files associated with that column family
  void SetDropped();
  bool IsDropped() const { return dropped_.load(std::memory_order_relaxed); }

  // thread-safe
  int NumberLevels() const { return ioptions_.num_levels; }

  void SetLogNumber(uint64_t log_number) { log_number_ = log_number; }
  uint64_t GetLogNumber() const { return log_number_; }

  // thread-safe
  const FileOptions* soptions() const;
  const ImmutableOptions* ioptions() const { return &ioptions_; }
  // REQUIRES: DB mutex held
  // This returns the MutableCFOptions used by current SuperVersion
  // You should use this API to reference MutableCFOptions most of the time.
  const MutableCFOptions* GetCurrentMutableCFOptions() const {
    return &(super_version_->mutable_cf_options);
  }
  // REQUIRES: DB mutex held
  // This returns the latest MutableCFOptions, which may be not in effect yet.
  const MutableCFOptions* GetLatestMutableCFOptions() const {
    return &mutable_cf_options_;
  }

  // REQUIRES: DB mutex held
  // Build ColumnFamiliesOptions with immutable options and latest mutable
  // options.
  ColumnFamilyOptions GetLatestCFOptions() const;

  bool is_delete_range_supported() { return is_delete_range_supported_; }

  // Validate CF options against DB options
  static Status ValidateOptions(const DBOptions& db_options,
                                const ColumnFamilyOptions& cf_options);
  // REQUIRES: DB mutex held
  Status SetOptions(
      const DBOptions& db_options,
      const std::unordered_map<std::string, std::string>& options_map);

  InternalStats* internal_stats() { return internal_stats_.get(); }

  MemTableList* imm() { return &imm_; }
  MemTable* mem() { return mem_; }

  bool IsEmpty() {
    return mem()->GetFirstSequenceNumber() == 0 && imm()->NumNotFlushed() == 0;
  }

  Version* current() { return current_; }
  Version* dummy_versions() { return dummy_versions_; }
  void SetCurrent(Version* _current);
  uint64_t GetNumLiveVersions() const;    // REQUIRE: DB mutex held
  uint64_t GetTotalSstFilesSize() const;  // REQUIRE: DB mutex held
  uint64_t GetLiveSstFilesSize() const;   // REQUIRE: DB mutex held
  uint64_t GetTotalBlobFileSize() const;  // REQUIRE: DB mutex held
  void SetMemtable(MemTable* new_mem) {
    uint64_t memtable_id = last_memtable_id_.fetch_add(1) + 1;
    new_mem->SetID(memtable_id);
    mem_ = new_mem;
  }

  // calculate the oldest log needed for the durability of this column family
  uint64_t OldestLogToKeep();

  // See Memtable constructor for explanation of earliest_seq param.
  MemTable* ConstructNewMemtable(const MutableCFOptions& mutable_cf_options,
                                 SequenceNumber earliest_seq);
  void CreateNewMemtable(const MutableCFOptions& mutable_cf_options,
                         SequenceNumber earliest_seq);

  TableCache* table_cache() const { return table_cache_.get(); }
  BlobSource* blob_source() const { return blob_source_.get(); }

  // See documentation in compaction_picker.h
  // REQUIRES: DB mutex held
  bool NeedsCompaction() const;
  // REQUIRES: DB mutex held
  Compaction* PickCompaction(const MutableCFOptions& mutable_options,
                             const MutableDBOptions& mutable_db_options,
                             LogBuffer* log_buffer);

  // Check if the passed range overlap with any running compactions.
  // REQUIRES: DB mutex held
  bool RangeOverlapWithCompaction(const Slice& smallest_user_key,
                                  const Slice& largest_user_key,
                                  int level) const;

  // Check if the passed ranges overlap with any unflushed memtables
  // (immutable or mutable).
  //
  // @param super_version A referenced SuperVersion that will be held for the
  //    duration of this function.
  //
  // Thread-safe
  Status RangesOverlapWithMemtables(const autovector<Range>& ranges,
                                    SuperVersion* super_version,
                                    bool allow_data_in_errors, bool* overlap);

  // A flag to tell a manual compaction is to compact all levels together
  // instead of a specific level.
  static const int kCompactAllLevels;
  // A flag to tell a manual compaction's output is base level.
  static const int kCompactToBaseLevel;
  // REQUIRES: DB mutex held
  Compaction* CompactRange(const MutableCFOptions& mutable_cf_options,
                           const MutableDBOptions& mutable_db_options,
                           int input_level, int output_level,
                           const CompactRangeOptions& compact_range_options,
                           const InternalKey* begin, const InternalKey* end,
                           InternalKey** compaction_end, bool* manual_conflict,
                           uint64_t max_file_num_to_ignore,
                           const std::string& trim_ts);

  CompactionPicker* compaction_picker() { return compaction_picker_.get(); }
  // thread-safe
  const Comparator* user_comparator() const {
    return internal_comparator_.user_comparator();
  }
  // thread-safe
  const InternalKeyComparator& internal_comparator() const {
    return internal_comparator_;
  }

  const IntTblPropCollectorFactories* int_tbl_prop_collector_factories() const {
    return &int_tbl_prop_collector_factories_;
  }

  SuperVersion* GetSuperVersion() { return super_version_; }
  // thread-safe
  // Return a already referenced SuperVersion to be used safely.
  SuperVersion* GetReferencedSuperVersion(DBImpl* db);
  // thread-safe
  // Get SuperVersion stored in thread local storage. If it does not exist,
  // get a reference from a current SuperVersion.
  SuperVersion* GetThreadLocalSuperVersion(DBImpl* db);
  // Try to return SuperVersion back to thread local storage. Return true on
  // success and false on failure. It fails when the thread local storage
  // contains anything other than SuperVersion::kSVInUse flag.
  bool ReturnThreadLocalSuperVersion(SuperVersion* sv);
  // thread-safe
  uint64_t GetSuperVersionNumber() const {
    return super_version_number_.load();
  }

  // Self Added Start
  void updateRDF2NewVersion(int opt, bool split_flag); // 1: flush, 2: compact, 3: for compaction direcly deleted flie
  // Self Added End


  // will return a pointer to SuperVersion* if previous SuperVersion
  // if its reference count is zero and needs deletion or nullptr if not
  // As argument takes a pointer to allocated SuperVersion to enable
  // the clients to allocate SuperVersion outside of mutex.
  // IMPORTANT: Only call this from DBImpl::InstallSuperVersion()
  void InstallSuperVersion(SuperVersionContext* sv_context,
                           const MutableCFOptions& mutable_cf_options);
  void InstallSuperVersion(SuperVersionContext* sv_context,
                           InstrumentedMutex* db_mutex);

  void ResetThreadLocalSuperVersions();

  // Protected by DB mutex
  void set_queued_for_flush(bool value) { queued_for_flush_ = value; }
  void set_queued_for_compaction(bool value) { queued_for_compaction_ = value; }
  bool queued_for_flush() { return queued_for_flush_; }
  bool queued_for_compaction() { return queued_for_compaction_; }

  static std::pair<WriteStallCondition, WriteStallCause>
  GetWriteStallConditionAndCause(
      int num_unflushed_memtables, int num_l0_files,
      uint64_t num_compaction_needed_bytes,
      const MutableCFOptions& mutable_cf_options,
      const ImmutableCFOptions& immutable_cf_options);

  // Recalculate some stall conditions, which are changed only during
  // compaction, adding new memtable and/or recalculation of compaction score.
  WriteStallCondition RecalculateWriteStallConditions(
      const MutableCFOptions& mutable_cf_options);

  void set_initialized() { initialized_.store(true); }

  bool initialized() const { return initialized_.load(); }

  const ColumnFamilyOptions& initial_cf_options() {
    return initial_cf_options_;
  }

  Env::WriteLifeTimeHint CalculateSSTWriteHint(int level);

  // created_dirs remembers directory created, so that we don't need to call
  // the same data creation operation again.
  Status AddDirectories(
      std::map<std::string, std::shared_ptr<FSDirectory>>* created_dirs);

  FSDirectory* GetDataDir(size_t path_id) const;

  // full_history_ts_low_ can only increase.
  void SetFullHistoryTsLow(std::string ts_low) {
    assert(!ts_low.empty());
    const Comparator* ucmp = user_comparator();
    assert(ucmp);
    if (full_history_ts_low_.empty() ||
        ucmp->CompareTimestamp(ts_low, full_history_ts_low_) > 0) {
      full_history_ts_low_ = std::move(ts_low);
    }
  }

  const std::string& GetFullHistoryTsLow() const {
    return full_history_ts_low_;
  }

  ThreadLocalPtr* TEST_GetLocalSV() { return local_sv_.get(); }
  WriteBufferManager* write_buffer_mgr() { return write_buffer_manager_; }
  std::shared_ptr<CacheReservationManager>
  GetFileMetadataCacheReservationManager() {
    return file_metadata_cache_res_mgr_;
  }

  SequenceNumber GetFirstMemtableSequenceNumber() const;

  static const uint32_t kDummyColumnFamilyDataId;

  // Keep track of whether the mempurge feature was ever used.
  void SetMempurgeUsed() { mempurge_used_ = true; }
  bool GetMempurgeUsed() { return mempurge_used_; }

  // Allocate and return a new epoch number
  uint64_t NewEpochNumber() { return next_epoch_number_.fetch_add(1); }

  // Get the next epoch number to be assigned
  uint64_t GetNextEpochNumber() const { return next_epoch_number_.load(); }

  // Set the next epoch number to be assigned
  void SetNextEpochNumber(uint64_t next_epoch_number) {
    next_epoch_number_.store(next_epoch_number);
  }

  // Reset the next epoch number to be assigned
  void ResetNextEpochNumber() { next_epoch_number_.store(1); }

  // Recover the next epoch number of this CF and epoch number
  // of its files (if missing)
  void RecoverEpochNumbers();


  //Self Added
  // void storeRange2RDFTest(RangeTombstone tombStone){
  //   RDF_test.push_back(std::make_pair( std::stoll(tombStone.start_key_.ToString()), std::stoll(tombStone.end_key_.ToString()) ));
  // }
  // void storeRange2RDFTest2(RangeTombstone tombStone){
  //   RDF_test2.push_back(std::make_pair( std::stoll(tombStone.start_key_.ToString()), std::stoll(tombStone.end_key_.ToString()) ));
  // }

  // void printRDFTest(){
  //   std::cout << "ColumnFamilyData --- RDF_test1 @column_family.h" << std::endl;
  //   for(auto x: RDF_test){
  //     std::cout << x.first << " " << x.second << std::endl;
  //   }
  //   std::cout << std::endl << std::endl;
  // }

  // void printRDFTest2(){
  //   std::cout << "ColumnFamilyData --- RDF_test2 @column_family.h" << std::endl;
  //   for(auto x: RDF_test2){
  //     std::cout << x.first << " " << x.second << std::endl;
  //   }
  //   std::cout << std::endl << std::endl;
  // }


  void inc_flush_install_count(){
    flush_install_count_clr += 1;
  }

  
  void inc_split__flush_install_count(){
    split__flush_install_count_clr += 1;
  }

  int get_flush_install_count_clr(){
    return flush_install_count_clr;
  }

  int get_split__flush_install_count_clr(){
    return split__flush_install_count_clr;
  }

  void inc_compaction_install_count(){
    compaction_install_count_clr += 1;
  }
  void inc_split__compaction_install_count(){
    split__compaction_install_count_clr += 1;
  }

  int get_compaction_install_count_clr(){
    return compaction_install_count_clr;
  }

  int get_split__compaction_install_count_clr(){
    return split__compaction_install_count_clr;
  }

  void clear_flush_install_count_clr(){
    flush_install_count_clr = 0;
  }

  void clear_split__flush_install_count_clr(){
    split__flush_install_count_clr = 0;
  }

  void clear_compaction_install_count_clr(){
    compaction_install_count_clr = 0;
  }
  
  void clear_split__compaction_install_count_clr(){
    split__compaction_install_count_clr = 0;
  }

  void inc_call_before_install_superversion_count(){
    call_before_install_superversion_count += 1;
  }
  int get_call_before_install_superversion_count(){
    return call_before_install_superversion_count;
  }
  void clear_call_before_install_superversion_count(){
    call_before_install_superversion_count = 0;
  }
  
  void inc_split__call_before_install_superversion_count(){
    split__call_before_install_superversion_count += 1;
  }
  int get_split__call_before_install_superversion_count(){
    return split__call_before_install_superversion_count;
  }
  void clear_split__call_before_install_superversion_count(){
    split__call_before_install_superversion_count = 0;
  }

  void set_flush_to_level0_RD_vector(std::tuple<uint64_t, std::vector<pll>, std::vector<uint64_t>> &flush_to_level0_RD_vector_in){
    auto file_num = std::get<0>(this->flush_to_level0_RD_vector);
    if((int)file_num != -1){
      std::cerr << "flush_to_level0_RD_vector is not empty" << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    }

    flush_to_level0_RD_vector = flush_to_level0_RD_vector_in;
  }
  
  void set_split__flush_to_level0_RD_vector(std::tuple<uint64_t, std::vector<pll>, std::vector<uint64_t>> &flush_to_level0_RD_vector_in){
    auto file_num = std::get<0>(this->split__flush_to_level0_RD_vector);
    if((int)file_num != -1){
      std::cerr << "split__flush_to_level0_RD_vector is not empty" << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    }

    split__flush_to_level0_RD_vector = flush_to_level0_RD_vector_in;
  }

  std::tuple<uint64_t, std::vector<pll>, std::vector<uint64_t>> get_flush_to_level0_RD_vector(){
    return flush_to_level0_RD_vector;
  }

  std::tuple<uint64_t, std::vector<pll>, std::vector<uint64_t>> get_split__flush_to_level0_RD_vector(){
    return split__flush_to_level0_RD_vector;
  }

  void set_compaction_moving_RD_vector(std::vector<std::tuple<int, int, std::vector<pll>, std::vector<uint64_t>>>  &compaction_moving_RD_vector_in){
    auto len = compaction_moving_RD_vector.size();
    if(len != 0){
      std::cerr << "compaction_moving_RD_vector is not empty" << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    }
    compaction_moving_RD_vector = compaction_moving_RD_vector_in;
  }

  void set_split__compaction_moving_RD_vector(std::vector<std::tuple<int, int, std::vector<pll>, std::vector<uint64_t>>>  &compaction_moving_RD_vector_in){
    auto len = split__compaction_moving_RD_vector.size();
    if(len != 0){
      std::cerr << "split__compaction_moving_RD_vector is not empty" << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    }
    split__compaction_moving_RD_vector = compaction_moving_RD_vector_in;
  }

  std::vector<std::tuple<int, int, std::vector<pll>, std::vector<uint64_t>>>  get_compaction_moving_RD_vector(){
    
    return compaction_moving_RD_vector;
  }

  std::vector<std::tuple<int, int, std::vector<pll>, std::vector<uint64_t>>>  get_split__compaction_moving_RD_vector(){
    
    return split__compaction_moving_RD_vector;
  }

  void set_compaction_direct_delete_RD_vector(std::tuple<int, std::vector<pll>, std::vector<uint64_t>> &compaction_direct_delete_RD_vector_in){
    auto out_lvl = std::get<0>(this->compaction_direct_delete_RD_vector);
    if((int)out_lvl != -1){
      std::cerr << "compaction_direct_delete_RD_vector is not empty" << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    }
    compaction_direct_delete_RD_vector = compaction_direct_delete_RD_vector_in;
  }

  void set_split__compaction_direct_delete_RD_vector(std::tuple<int, std::vector<pll>, std::vector<uint64_t>> &compaction_direct_delete_RD_vector_in){
    auto out_lvl = std::get<0>(this->split__compaction_direct_delete_RD_vector);
    if((int)out_lvl != -1){
      std::cerr << "split__compaction_direct_delete_RD_vector is not empty" << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    }
    split__compaction_direct_delete_RD_vector = compaction_direct_delete_RD_vector_in;
  }

  std::tuple<int, std::vector<pll>, std::vector<uint64_t>> get_compaction_direct_delete_RD_vector(){
    return compaction_direct_delete_RD_vector;
  }
  
  std::tuple<int, std::vector<pll>, std::vector<uint64_t>> get_split__compaction_direct_delete_RD_vector(){
    return split__compaction_direct_delete_RD_vector;
  }
  




  void set_top_level__trivial_move__delete_RD_vector(std::tuple<int, std::vector<pll>, std::vector<uint64_t>> &compaction_direct_delete_RD_vector_in){
    auto out_lvl = std::get<0>(this->top_level__trivial_move__delete_RD_vector);
    if((int)out_lvl != -1){
      std::cerr << "split__compaction_direct_delete_RD_vector is not empty" << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    }
    top_level__trivial_move__delete_RD_vector = compaction_direct_delete_RD_vector_in;
  }
  
  // void set_top_level__direct_delete__delete_RD_vector(std::tuple<int, std::vector<pll>, std::vector<uint64_t>> &compaction_direct_delete_RD_vector_in){
  //   auto out_lvl = std::get<0>(this->top_level__direct_delete__delete_RD_vector);
  //   if((int)out_lvl != -1){
  //     std::cerr << "split__compaction_direct_delete_RD_vector is not empty" << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
  //   }
  //   top_level__direct_delete__delete_RD_vector = compaction_direct_delete_RD_vector_in;
  // }

  std::tuple<int, std::vector<pll>, std::vector<uint64_t>> get_top_level__trivial_move__delete_RD_vector(){
    return top_level__trivial_move__delete_RD_vector;
  }

  // std::tuple<int, std::vector<pll>, std::vector<uint64_t>> get_top_level__direct_delete__delete_RD_vector(){
  //   return top_level__direct_delete__delete_RD_vector;
  // }





  void printPLRDF(){
    std::cout << "cfd --- PLRDF " << __FILE__ << ":" << __LINE__  << " " << __FUNCTION__ << std::endl << std::flush;
    plrdf_prime.printLevel0();
    plrdf_prime.print();
  }

  void printSplitPLRDF(){
    std::cout << "cfd --- split_PLRDF " << __FILE__ << ":" << __LINE__  << " " << __FUNCTION__ << std::endl << std::flush;
    split_plrdf_prime.printLevel0();
    split_plrdf_prime.print();
  }
  
  void printTopLevelRDF(){
    std::cout << "cfd --- top_level_RDF " << __FILE__ << ":" << __LINE__  << " " << __FUNCTION__ << std::endl << std::flush;
    top_level_rdf_prime.printLevel0();
    top_level_rdf_prime.print();
  }

  //split_start --- split_range --- split_end should be called in a sequence
  void split_start(int out_lvl){
    //Split RDF
    split__level_update_mtx.lock();

    auto len = split__level_ranges.size();
    auto len2 = split__level_ranges_updated.size();
    if((int)len != 0){
      std::cerr << "split__level_ranges is not empty" << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    }
    if((int)len2 != 0){
      std::cerr << "split__level_ranges_updated is not empty" << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl
                << "len2 = " << len2 << std::endl;
    }
    split__level_ranges = split_plrdf_prime.getLevelRanges(out_lvl);
    split__count += 1;
    split__out_level = out_lvl;
    split__level_range_idx = 0;
    split__level_points.clear();

    if(split__fin_flag != 0){
      std::cerr << "Error: split__fin_flag is not 0. Previous one haven't written into split_plrdf "
                << "before this one yet at function installSuperversion!! " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl
                << "split__fin_flag = " << split__fin_flag << std::endl;
    }

    //Top Level RDF
    top_level__level_ranges = top_level_rdf_prime.getLevelRanges(1);
    top_level__level_points.clear();
    top_level__level_range_idx = 0;
  }

  void split_range(long long key_in){
    //Top Level RDF
    // level 0 -> entries are not ordered with time
    top_level__level_points.push_back(key_in);

    // int &idx2 = top_level__level_range_idx;
    // int len2 = top_level__level_ranges.size();
    // if(idx2 < len2){
    //   while(idx2 < len2 && top_level__level_ranges[idx2].second <= key_in){
    //     idx2 += 1;
    //   }
    //   if(idx2 < len2 && top_level__level_ranges[idx2].first <= key_in && top_level__level_ranges[idx2].second  >  key_in){
    //     top_level__level_points.push_back(key_in);
    //   }
    // }

    //Split RDF
    split__level_points.push_back(key_in);
    int &idx = split__level_range_idx;
    int len = split__level_ranges.size();
    if(idx >= len){return;}

    while(idx < len && split__level_ranges[idx].second <= key_in){
      split__level_ranges_updated.push_back(split__level_ranges[idx]);
      idx += 1;
    }
    if(idx < len && split__level_ranges[idx].first > key_in){
      return;
    }


    // if(idx < len && split__level_ranges[idx].first <= key_in && split__level_ranges[idx].second  >  key_in){
    //   split__level_points.push_back(key_in);
    // }


    if(idx < len && split__level_ranges[idx].first == key_in){
      if(key_in + 1 >= split__level_ranges[idx].second){
        idx += 1;
      }else{
        split__level_ranges[idx].first = key_in + 1;
      }
      return;
    }

    if(idx < len && split__level_ranges[idx].first < key_in && split__level_ranges[idx].second  >  key_in){
      auto tmp = split__level_ranges[idx];
      tmp.second = key_in;
      split__level_ranges_updated.push_back(tmp);
      if(key_in + 1 >= split__level_ranges[idx].second){
        idx += 1;
      }else{
        split__level_ranges[idx].first = key_in + 1;
      }
      return;
    }
    


    if(idx >= len){return;}


    std::cerr << "Error: condition not checked. " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl
              << "split__level_ranges[idx].first: " << split__level_ranges[idx].first << std::endl
              << "split__level_ranges[idx].second: " << split__level_ranges[idx].second << std::endl
              << "key_in: " << key_in << std::endl;
    exit(1);
  }

  //not called in: updateRDF2NewVersion()
  void split_end(){
    int &idx = split__level_range_idx;
    int len = split__level_ranges.size();
    while(idx < len){
      split__level_ranges_updated.push_back(split__level_ranges[idx]);
      idx += 1;
    }

    split__fin_flag = 1;
    split__level_ranges.clear();
    split__level_range_idx = 0;

    split__level_update_mtx.unlock();
  }

  void clear_split__level_ranges_updated(){
    std::lock_guard<std::mutex> guard(split__level_update_mtx);
    split__level_ranges_updated.clear();
  }


  void clear_split__count(){
      std::lock_guard<std::mutex> guard(split__level_update_mtx);
      split__count = 0;
  }

  void clear_split__fin_flag(){
      std::lock_guard<std::mutex> guard(split__level_update_mtx);
      split__fin_flag = 0;
  }

  void clear_split__out_level(){
      std::lock_guard<std::mutex> guard(split__level_update_mtx);
      split__out_level = -1;
  }

  int get_split__count(){
    std::lock_guard<std::mutex> guard(split__level_update_mtx);
    return split__count;
  }

  int get_split__fin_flag(){
    std::lock_guard<std::mutex> guard(split__level_update_mtx);
    return split__fin_flag;
  }

  int get_split__out_level(){
    std::lock_guard<std::mutex> guard(split__level_update_mtx);
    return split__out_level;
  }



  void set_flush_in_file_num(uint64_t num){
    flush_in_file_num = num;
  }
  uint64_t get_flush_in_file_num(){
    return flush_in_file_num;
  }


  void logCurrentTotalNumbersOfRangesInEachRDF(){
    plrdf_prime.logCurrentTotalNumbersOfRanges();
    split_plrdf_prime.logCurrentTotalNumbersOfRanges();
    top_level_rdf_prime.logCurrentTotalNumbersOfRanges();
  }
  std::vector<int> getLogOfNumbersOfRangesInPLRDF(){
    return plrdf_prime.getNumbersOfRangesInRDFLog();
  }
  std::vector<int> getLogOfNumbersOfRangesInSplitPLRDF(){
    return split_plrdf_prime.getNumbersOfRangesInRDFLog();
  }
  std::vector<int> getLogOfNumbersOfRangesInTopLevelRDF(){
    return top_level_rdf_prime.getNumbersOfRangesInRDFLog();
  }

 private:
  // //Self Added Start
  Version* update_RDF_version_pre = nullptr;
  Version* install_version_pre = nullptr;
  PLRDF plrdf_prime, split_plrdf_prime;
  PLRDF top_level_rdf_prime;
  
  // for plrdf_prime
  std::tuple<uint64_t, std::vector<pll>, std::vector<uint64_t>> flush_to_level0_RD_vector = std::make_tuple(-1, std::vector<pll>(), std::vector<uint64_t>());
  std::vector<std::tuple<int, int, std::vector<pll>, std::vector<uint64_t>>>  compaction_moving_RD_vector;
  std::tuple<int, std::vector<pll>, std::vector<uint64_t>> compaction_direct_delete_RD_vector = std::make_tuple(-1, std::vector<pll>(), std::vector<uint64_t>());
  
  int flush_install_count_clr = 0;
  int compaction_install_count_clr = 0;
  int call_before_install_superversion_count = 0;

  //for split_plrdf_prime
  std::tuple<uint64_t, std::vector<pll>, std::vector<uint64_t>> split__flush_to_level0_RD_vector = std::make_tuple(-1, std::vector<pll>(), std::vector<uint64_t>());
  std::vector<std::tuple<int, int, std::vector<pll>, std::vector<uint64_t>>>  split__compaction_moving_RD_vector;
  std::tuple<int, std::vector<pll>, std::vector<uint64_t>> split__compaction_direct_delete_RD_vector = std::make_tuple(-1, std::vector<pll>(), std::vector<uint64_t>());
  std::vector<pll> split__level_ranges;
  std::vector<pll> split__level_ranges_updated;
  std::vector<long long> split__level_points;
  std::mutex split__level_update_mtx;

  int split__flush_install_count_clr = 0;
  int split__compaction_install_count_clr = 0;
  int split__call_before_install_superversion_count = 0;
  int split__count = 0;
  int split__out_level = -1;
  int split__fin_flag = 0;
  int split__level_range_idx = 0;

  uint64_t flush_in_file_num = 0;
  std::vector<pll> top_level__level_ranges;
  std::vector<long long> top_level__level_points;
  std::tuple<int, std::vector<pll>, std::vector<uint64_t>> top_level__trivial_move__delete_RD_vector = std::make_tuple(-1, std::vector<pll>(), std::vector<uint64_t>());
  // std::tuple<int, std::vector<pll>, std::vector<uint64_t>> top_level__direct_delete__delete_RD_vector = std::make_tuple(-1, std::vector<pll>(), std::vector<uint64_t>());
  int top_level__level_range_idx = 0;
  // std::vector<PL_RDF> per_level_RDF; //Self Added, ranges don't split when inserts come//added by ychaung
  // // std::vector<std::pair<long long, long long>> RDF_test, RDF_test2; //Self Added
  // //Self Added End


  friend class ColumnFamilySet;
  ColumnFamilyData(uint32_t id, const std::string& name,
                   Version* dummy_versions, Cache* table_cache,
                   WriteBufferManager* write_buffer_manager,
                   const ColumnFamilyOptions& options,
                   const ImmutableDBOptions& db_options,
                   const FileOptions* file_options,
                   ColumnFamilySet* column_family_set,
                   BlockCacheTracer* const block_cache_tracer,
                   const std::shared_ptr<IOTracer>& io_tracer,
                   const std::string& db_id, const std::string& db_session_id);

  std::vector<std::string> GetDbPaths() const;

  uint32_t id_;
  const std::string name_;
  Version* dummy_versions_;  // Head of circular doubly-linked list of versions.
  Version* current_;         // == dummy_versions->prev_

  std::atomic<int> refs_;  // outstanding references to ColumnFamilyData
  std::atomic<bool> initialized_;
  std::atomic<bool> dropped_;  // true if client dropped it

  const InternalKeyComparator internal_comparator_;
  IntTblPropCollectorFactories int_tbl_prop_collector_factories_;

  const ColumnFamilyOptions initial_cf_options_;
  const ImmutableOptions ioptions_;
  MutableCFOptions mutable_cf_options_;

  const bool is_delete_range_supported_;

  std::unique_ptr<TableCache> table_cache_;
  std::unique_ptr<BlobFileCache> blob_file_cache_;
  std::unique_ptr<BlobSource> blob_source_;

  std::unique_ptr<InternalStats> internal_stats_;

  WriteBufferManager* write_buffer_manager_;

  MemTable* mem_;
  MemTableList imm_;
  SuperVersion* super_version_;

  // An ordinal representing the current SuperVersion. Updated by
  // InstallSuperVersion(), i.e. incremented every time super_version_
  // changes.
  std::atomic<uint64_t> super_version_number_;

  // Thread's local copy of SuperVersion pointer
  // This needs to be destructed before mutex_
  std::unique_ptr<ThreadLocalPtr> local_sv_;

  // pointers for a circular linked list. we use it to support iterations over
  // all column families that are alive (note: dropped column families can also
  // be alive as long as client holds a reference)
  ColumnFamilyData* next_;
  ColumnFamilyData* prev_;

  // This is the earliest log file number that contains data from this
  // Column Family. All earlier log files must be ignored and not
  // recovered from
  uint64_t log_number_;

  // An object that keeps all the compaction stats
  // and picks the next compaction
  std::unique_ptr<CompactionPicker> compaction_picker_;

  ColumnFamilySet* column_family_set_;

  std::unique_ptr<WriteControllerToken> write_controller_token_;

  // If true --> this ColumnFamily is currently present in DBImpl::flush_queue_
  bool queued_for_flush_;

  // If true --> this ColumnFamily is currently present in
  // DBImpl::compaction_queue_
  bool queued_for_compaction_;

  uint64_t prev_compaction_needed_bytes_;

  // if the database was opened with 2pc enabled
  bool allow_2pc_;

  // Memtable id to track flush.
  std::atomic<uint64_t> last_memtable_id_;

  // Directories corresponding to cf_paths.
  std::vector<std::shared_ptr<FSDirectory>> data_dirs_;

  bool db_paths_registered_;

  std::string full_history_ts_low_;

  // For charging memory usage of file metadata created for newly added files to
  // a Version associated with this CFD
  std::shared_ptr<CacheReservationManager> file_metadata_cache_res_mgr_;
  bool mempurge_used_;

  std::atomic<uint64_t> next_epoch_number_;
};

// ColumnFamilySet has interesting thread-safety requirements
// * CreateColumnFamily() or RemoveColumnFamily() -- need to be protected by DB
// mutex AND executed in the write thread.
// CreateColumnFamily() should ONLY be called from VersionSet::LogAndApply() AND
// single-threaded write thread. It is also called during Recovery and in
// DumpManifest().
// RemoveColumnFamily() is only called from SetDropped(). DB mutex needs to be
// held and it needs to be executed from the write thread. SetDropped() also
// guarantees that it will be called only from single-threaded LogAndApply(),
// but this condition is not that important.
// * Iteration -- hold DB mutex. If you want to release the DB mutex in the
// body of the iteration, wrap in a RefedColumnFamilySet.
// * GetDefault() -- thread safe
// * GetColumnFamily() -- either inside of DB mutex or from a write thread
// * GetNextColumnFamilyID(), GetMaxColumnFamily(), UpdateMaxColumnFamily(),
// NumberOfColumnFamilies -- inside of DB mutex
class ColumnFamilySet {
 public:
  // ColumnFamilySet supports iteration
  class iterator {
   public:
    explicit iterator(ColumnFamilyData* cfd) : current_(cfd) {}
    // NOTE: minimum operators for for-loop iteration
    iterator& operator++() {
      current_ = current_->next_;
      return *this;
    }
    bool operator!=(const iterator& other) const {
      return this->current_ != other.current_;
    }
    ColumnFamilyData* operator*() { return current_; }

   private:
    ColumnFamilyData* current_;
  };

  ColumnFamilySet(const std::string& dbname,
                  const ImmutableDBOptions* db_options,
                  const FileOptions& file_options, Cache* table_cache,
                  WriteBufferManager* _write_buffer_manager,
                  WriteController* _write_controller,
                  BlockCacheTracer* const block_cache_tracer,
                  const std::shared_ptr<IOTracer>& io_tracer,
                  const std::string& db_id, const std::string& db_session_id);
  ~ColumnFamilySet();

  ColumnFamilyData* GetDefault() const;
  // GetColumnFamily() calls return nullptr if column family is not found
  ColumnFamilyData* GetColumnFamily(uint32_t id) const;
  ColumnFamilyData* GetColumnFamily(const std::string& name) const;
  // this call will return the next available column family ID. it guarantees
  // that there is no column family with id greater than or equal to the
  // returned value in the current running instance or anytime in RocksDB
  // instance history.
  uint32_t GetNextColumnFamilyID();
  uint32_t GetMaxColumnFamily();
  void UpdateMaxColumnFamily(uint32_t new_max_column_family);
  size_t NumberOfColumnFamilies() const;

  ColumnFamilyData* CreateColumnFamily(const std::string& name, uint32_t id,
                                       Version* dummy_version,
                                       const ColumnFamilyOptions& options);

  iterator begin() { return iterator(dummy_cfd_->next_); }
  iterator end() { return iterator(dummy_cfd_); }

  Cache* get_table_cache() { return table_cache_; }

  WriteBufferManager* write_buffer_manager() { return write_buffer_manager_; }

  WriteController* write_controller() { return write_controller_; }

 private:
  friend class ColumnFamilyData;
  // helper function that gets called from cfd destructor
  // REQUIRES: DB mutex held
  void RemoveColumnFamily(ColumnFamilyData* cfd);

  // column_families_ and column_family_data_ need to be protected:
  // * when mutating both conditions have to be satisfied:
  // 1. DB mutex locked
  // 2. thread currently in single-threaded write thread
  // * when reading, at least one condition needs to be satisfied:
  // 1. DB mutex locked
  // 2. accessed from a single-threaded write thread
  UnorderedMap<std::string, uint32_t> column_families_;
  UnorderedMap<uint32_t, ColumnFamilyData*> column_family_data_;

  uint32_t max_column_family_;
  const FileOptions file_options_;

  ColumnFamilyData* dummy_cfd_;
  // We don't hold the refcount here, since default column family always exists
  // We are also not responsible for cleaning up default_cfd_cache_. This is
  // just a cache that makes common case (accessing default column family)
  // faster
  ColumnFamilyData* default_cfd_cache_;

  const std::string db_name_;
  const ImmutableDBOptions* const db_options_;
  Cache* table_cache_;
  WriteBufferManager* write_buffer_manager_;
  WriteController* write_controller_;
  BlockCacheTracer* const block_cache_tracer_;
  std::shared_ptr<IOTracer> io_tracer_;
  const std::string& db_id_;
  std::string db_session_id_;
};

// A wrapper for ColumnFamilySet that supports releasing DB mutex during each
// iteration over the iterator, because the cfd is Refed and Unrefed during
// each iteration to prevent concurrent CF drop from destroying it (until
// Unref).
class RefedColumnFamilySet {
 public:
  explicit RefedColumnFamilySet(ColumnFamilySet* cfs) : wrapped_(cfs) {}

  class iterator {
   public:
    explicit iterator(ColumnFamilySet::iterator wrapped) : wrapped_(wrapped) {
      MaybeRef(*wrapped_);
    }
    ~iterator() { MaybeUnref(*wrapped_); }
    inline void MaybeRef(ColumnFamilyData* cfd) {
      if (cfd->GetID() != ColumnFamilyData::kDummyColumnFamilyDataId) {
        cfd->Ref();
      }
    }
    inline void MaybeUnref(ColumnFamilyData* cfd) {
      if (cfd->GetID() != ColumnFamilyData::kDummyColumnFamilyDataId) {
        cfd->UnrefAndTryDelete();
      }
    }
    // NOTE: minimum operators for for-loop iteration
    inline iterator& operator++() {
      ColumnFamilyData* old = *wrapped_;
      ++wrapped_;
      // Can only unref & potentially free cfd after accessing its next_
      MaybeUnref(old);
      MaybeRef(*wrapped_);
      return *this;
    }
    inline bool operator!=(const iterator& other) const {
      return this->wrapped_ != other.wrapped_;
    }
    inline ColumnFamilyData* operator*() { return *wrapped_; }

   private:
    ColumnFamilySet::iterator wrapped_;
  };

  iterator begin() { return iterator(wrapped_->begin()); }
  iterator end() { return iterator(wrapped_->end()); }

 private:
  ColumnFamilySet* wrapped_;
};

// We use ColumnFamilyMemTablesImpl to provide WriteBatch a way to access
// memtables of different column families (specified by ID in the write batch)
class ColumnFamilyMemTablesImpl : public ColumnFamilyMemTables {
 public:
  explicit ColumnFamilyMemTablesImpl(ColumnFamilySet* column_family_set)
      : column_family_set_(column_family_set), current_(nullptr) {}

  // Constructs a ColumnFamilyMemTablesImpl equivalent to one constructed
  // with the arguments used to construct *orig.
  explicit ColumnFamilyMemTablesImpl(ColumnFamilyMemTablesImpl* orig)
      : column_family_set_(orig->column_family_set_), current_(nullptr) {}

  // sets current_ to ColumnFamilyData with column_family_id
  // returns false if column family doesn't exist
  // REQUIRES: use this function of DBImpl::column_family_memtables_ should be
  //           under a DB mutex OR from a write thread
  bool Seek(uint32_t column_family_id) override;

  // Returns log number of the selected column family
  // REQUIRES: under a DB mutex OR from a write thread
  uint64_t GetLogNumber() const override;

  // REQUIRES: Seek() called first
  // REQUIRES: use this function of DBImpl::column_family_memtables_ should be
  //           under a DB mutex OR from a write thread
  virtual MemTable* GetMemTable() const override;

  // Returns column family handle for the selected column family
  // REQUIRES: use this function of DBImpl::column_family_memtables_ should be
  //           under a DB mutex OR from a write thread
  virtual ColumnFamilyHandle* GetColumnFamilyHandle() override;

  // Cannot be called while another thread is calling Seek().
  // REQUIRES: use this function of DBImpl::column_family_memtables_ should be
  //           under a DB mutex OR from a write thread
  virtual ColumnFamilyData* current() override { return current_; }

 private:
  ColumnFamilySet* column_family_set_;
  ColumnFamilyData* current_;
  ColumnFamilyHandleInternal handle_;
};

extern uint32_t GetColumnFamilyID(ColumnFamilyHandle* column_family);

extern const Comparator* GetColumnFamilyUserComparator(
    ColumnFamilyHandle* column_family);

}  // namespace ROCKSDB_NAMESPACE
