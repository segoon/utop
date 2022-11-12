#include "top.hpp"

#include <iostream>

#include <boost/filesystem.hpp>

namespace {

struct Thread {
  pid_t pid;
  std::string name;
  long cpu_usage_ms;
};

struct Snapshot {
  std::vector<Thread> threads;
};

std::string ReadProcPidName(const std::string& pid) {
  std::ifstream comm(pid + "/comm");
  std::string name;
  comm >> name;

  return name;
}

double ReadProcPidCpuUsage(const std::string& pid) {
  std::ifstream ifs(pid + "/sched");
  std::string line;

  // First 2 lines are garbage
  std::getline(ifs, line);
  std::getline(ifs, line);

  while (true) {
    std::getline(ifs, line);

    auto pos = line.find(' ');
    auto name = std::string_view(line).substr(0, pos);
    if (name != "se.sum_exec_runtime") continue;

    auto pos2 = line.rfind(' ');
    auto value = std::string_view(line).substr(pos2 + 1);

    return std::stod(std::string(value));
  }
}

Snapshot MakeSnapshot(pid_t pid) {
  Snapshot sn;

  auto tasks_dirpath = "/proc/" + std::to_string(pid) + "/task";
  boost::filesystem::directory_iterator it(tasks_dirpath);
  for (const auto& dir_entry : it) {
    auto pid_path = dir_entry.path().string();

    auto name = ReadProcPidName(pid_path);
    double cpu_usage = ReadProcPidCpuUsage(pid_path);

    sn.threads.push_back({std::stoll(dir_entry.path().filename().string()),
                          name, int(cpu_usage)});
  }

  return sn;
}

}  // namespace

void TopWindow(pid_t pid) {
  std::stoul("1665992");
  auto sn = MakeSnapshot(pid);
  for (const auto& thrd : sn.threads) {
    std::cout << thrd.pid << " " << thrd.name << " " << thrd.cpu_usage_ms
              << "\n";
  }
}
