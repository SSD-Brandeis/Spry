#ifndef UTILS_PROFILING_TIMER_H
#define UTILS_PROFILING_TIMER_H

#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <iomanip>

// Simple manual profiling timer
// Usage:
//   {
//      ScopedTimer t("Initialization");
//      // code...
//   }
//
// OR
//   Profiler::Start("Phase1");
//   // code...
//   Profiler::Stop("Phase1");

class Profiler {
public:
    static Profiler& Instance() {
        static Profiler instance;
        return instance;
    }

    void Start(const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex_);
        starts_[name] = std::chrono::high_resolution_clock::now();
    }

    void Stop(const std::string& name) {
        auto end_time = std::chrono::high_resolution_clock::now();
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = starts_.find(name);
        if (it != starts_.end()) {
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - it->second).count();
            durations_[name] += duration; // Accumulate
            calls_[name]++;
            starts_.erase(it);
        }
    }

    void PrintResults() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << "\n=== Profiling Results ===\n";
        std::cout << std::left << std::setw(30) << "Name" 
                  << std::right << std::setw(15) << "Total (ms)" 
                  << std::setw(10) << "Calls" 
                  << std::setw(15) << "Avg (ms)" << "\n";
        std::cout << std::string(70, '-') << "\n";
        
        for (const auto& pair : durations_) {
            const std::string& name = pair.first;
            double total_ms = pair.second / 1000.0;
            long count = calls_[name];
            double avg_ms = count > 0 ? total_ms / count : 0.0;

            std::cout << std::left << std::setw(30) << name 
                      << std::right << std::setw(15) << std::fixed << std::setprecision(3) << total_ms 
                      << std::setw(10) << count 
                      << std::setw(15) << avg_ms << "\n";
        }
        std::cout << "=========================\n\n";
    }

private:
    std::mutex mutex_;
    std::map<std::string, std::chrono::high_resolution_clock::time_point> starts_;
    std::map<std::string, long long> durations_; // microseconds
    std::map<std::string, long> calls_;
};

class ScopedTimer {
public:
    ScopedTimer(const std::string& name) : name_(name) {
        Profiler::Instance().Start(name_);
    }
    ~ScopedTimer() {
        Profiler::Instance().Stop(name_);
    }
private:
    std::string name_;
};

#endif // UTILS_PROFILING_TIMER_H
