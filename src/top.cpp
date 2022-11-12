#include "top.hpp"

#include <chrono>
#include <iostream>
#include <thread>
#include <unordered_map>

#include <boost/filesystem.hpp>

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp> // for text, gauge, operator|, flex, hbox, Element
#include <ftxui/screen/screen.hpp> // for Screen

namespace {

struct Thread {
  pid_t pid;
  std::string name;
  long cpu_usage_ms;
};

struct Snapshot {
  std::vector<Thread> threads;
  std::unordered_map<pid_t, size_t> by_pid;
};

std::string ReadProcPidName(const std::string &pid) {
  std::ifstream comm(pid + "/comm");
  std::string name;
  comm >> name;

  return name;
}

double ReadProcPidCpuUsage(const std::string &pid) {
  std::ifstream ifs(pid + "/sched");
  std::string line;

  // First 2 lines are garbage
  std::getline(ifs, line);
  std::getline(ifs, line);

  while (true) {
    std::getline(ifs, line);

    auto pos = line.find(' ');
    auto name = std::string_view(line).substr(0, pos);
    if (name != "se.sum_exec_runtime")
      continue;

    auto pos2 = line.rfind(' ');
    auto value = std::string_view(line).substr(pos2 + 1);

    return std::stod(std::string(value));
  }
}

/// Some func
Snapshot MakeSnapshot(pid_t pid) {
  Snapshot sn;

  auto tasks_dirpath = "/proc/" + std::to_string(pid) + "/task";
  boost::filesystem::directory_iterator it(tasks_dirpath);
  for (const auto &dir_entry : it) {
    auto pid_path = dir_entry.path().string();

    auto name = ReadProcPidName(pid_path);
    double cpu_usage = ReadProcPidCpuUsage(pid_path);
    pid_t pid = std::stoll(dir_entry.path().filename().string());

    sn.threads.push_back({pid, name, int(cpu_usage)});
    sn.by_pid.emplace(pid, sn.threads.size() - 1);
  }

  return sn;
}

void DiffSnapshots(Snapshot *sn, const Snapshot &sn_new) {
  for (auto &thrd : sn->threads) {
    auto pid = thrd.pid;
    auto idx = sn_new.by_pid.at(pid);
    auto diff_cpu_usage_ms =
        sn_new.threads[idx].cpu_usage_ms - thrd.cpu_usage_ms;

    thrd.cpu_usage_ms = diff_cpu_usage_ms;
  }
}

} // namespace

void TopWindow(pid_t pid) {
  using namespace ftxui;

  std::stoul("1665992"); // TODO
  auto sn = MakeSnapshot(pid);

  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(1));

    auto sn_new = MakeSnapshot(pid);
    DiffSnapshots(&sn, sn_new);

    std::vector<Elements> lines;
    for (const auto &thrd : sn.threads) {
      auto perc = thrd.cpu_usage_ms / 1000.0;

      lines.push_back(
          {text(thrd.name), text(" (" + std::to_string(thrd.pid) + ") "),
           gauge(perc) | flex, text(" " + std::to_string(100 * perc) + "%")});
    }
    auto screen = Screen::Create(Dimension::Full(), Dimension::Full());

    auto document = gridbox(std::move(lines));

    Render(screen, document);
    screen.Print(); // TODO: loop

    sn = std::move(sn_new);
  }
}
