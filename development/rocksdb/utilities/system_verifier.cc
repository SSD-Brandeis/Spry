/*
 *  Created on: April 23, 2023
 *  Author: yucheng
 */

#include "include/rocksdb/system_verifier.h"
using namespace std;
using namespace checking;


SystemVerifier* SystemVerifier::system_verifier;
CacheTombstoneTracer* CacheTombstoneTracer::cache_tombstone_tracer;
std::mutex CacheTombstoneTracer::init_mutex;
// std::shared_mutex CacheTombstoneTracer::rw_mutex;