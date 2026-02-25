#ifndef UTILS_VERIFICATION_DURING_RUN_H
#define UTILS_VERIFICATION_DURING_RUN_H

#include <iostream>
#include <map>
#include <string>

#include "rocksdb/status.h"
#include "rocksdb/system_verifier.h"


namespace verification {

inline void verifyPointQuery(const std::string& key, const rocksdb::Status& s,
                             const std::string& value,
                             checking::SystemVerifier* verifier) {
  bool gt_exists = verifier->isKeyExist(key);
  std::string gt_value = verifier->get(key);

  if (s.ok() != gt_exists) {
    std::cerr
        << "Verification Error: Point Query Existence Inconsistency for key "
        << key << " (Result: " << (s.ok() ? "Exists" : "Not Found")
        << ", GT: " << (gt_exists ? "Exists" : "Not Found") << ")" << std::endl;
  } else if (gt_exists && value != gt_value) {
    std::cerr << "Verification Error: Point Query Value Inconsistency for key "
              << key << " (Result: " << value << ", GT: " << gt_value << ")"
              << std::endl;
  }
}

inline void verifyScan(
    const std::string& start_key, const std::string& end_key,
    const std::vector<std::pair<std::string, std::string>>& scan_results,
    checking::SystemVerifier* verifier) {
  auto gt = verifier->getGroundTruth();
  auto it_gt = gt.lower_bound(start_key);
  size_t result_idx = 0;

  while (it_gt != gt.end() && it_gt->first < end_key) {
    if (result_idx >= scan_results.size()) {
      std::cerr << "Verification Error: Scan Missing Result for range ["
                << start_key << ", " << end_key << ")"
                << " Expected key: " << it_gt->first << std::endl;
      return;
    }

    if (scan_results[result_idx].first != it_gt->first) {
      std::cerr << "Verification Error: Scan Key Mismatch for range ["
                << start_key << ", " << end_key << ")"
                << " Expected: " << it_gt->first
                << ", Found: " << scan_results[result_idx].first << std::endl;
      return;
    }

    if (scan_results[result_idx].second != it_gt->second) {
      std::cerr << "Verification Error: Scan Value Mismatch for key "
                << it_gt->first << " Expected: " << it_gt->second
                << ", Found: " << scan_results[result_idx].second << std::endl;
    }

    it_gt++;
    result_idx++;
  }

  if (result_idx < scan_results.size()) {
    std::cerr << "Verification Error: Scan Extra Result for range ["
              << start_key << ", " << end_key << ")"
              << " Found unexpected key: " << scan_results[result_idx].first
              << std::endl;
  }
}

}  // namespace verification

#endif  // UTILS_VERIFICATION_DURING_RUN_H
