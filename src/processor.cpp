#include <string>
#include <iostream>

#include "processor.h"
#include "linux_parser.h"

using std::string;
using std::stoi;

float Processor::Utilization() { 
    auto linestream = LinuxParser::GetLineStream(LinuxParser::kProcDirectory + LinuxParser::kStatFilename, "cpu");

    string cpu_word, user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;

    linestream >> cpu_word >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;

    int idle_time = stoi(idle) + stoi(iowait);
    int system_time = stoi(system) + stoi(irq) + stoi(softirq);
    int virt_time = stoi(guest) + stoi(guest_nice);
    int total_time = stoi(user) + stoi(nice) + system_time + idle_time + stoi(steal) + virt_time;

    return (total_time - idle_time) / static_cast<float>(total_time); 
}