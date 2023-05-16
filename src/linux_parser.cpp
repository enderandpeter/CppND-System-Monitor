#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "processor.h"

using std::stof;
using std::stol;
using std::string;
using std::to_string;
using std::vector;
using std::string;

string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

/**
 * Return a string stream of the line that contains the requested info.
 * 
 * By default, a string stream for the first line of the file at the filePath is returned.
 * If searchToken is given, a string steam for the line that contains that searchToken is returned.
 * 
 * If the file was not opened, a string stream for an empty string is returned
*/
std::istringstream LinuxParser::GetLineStream(string filePath, string searchToken = ""){
  string line;
  std::ifstream stream(filePath);

  if (stream.is_open()) {
    if(!searchToken.empty()){
      while(std::getline(stream, line)){
        if(line.find(searchToken) != string::npos){
          return std::istringstream(line);
        }
      }

    } else {
      std::getline(stream, line);
      return std::istringstream(line);
    }
  }

  return std::istringstream("");
}

string LinuxParser::Kernel() {
  auto linestream = GetLineStream(kProcDirectory + kVersionFilename);

  string os, kernel, version_word;
  linestream >> os >> version_word >> kernel;

  return kernel;
}

vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

float LinuxParser::MemoryUtilization() { 
  auto mem_total_linestream = GetLineStream(kProcDirectory + kMeminfoFilename, kMeminfoMemTotalLabel);
  auto mem_free_linestream = GetLineStream(kProcDirectory + kMeminfoFilename, kMeminfoMemFreeLabel);

  string mem_total_word, mem_total_value, mem_free_word, mem_free_value;

  mem_total_linestream >> mem_total_word >> mem_total_value;
  mem_free_linestream >> mem_free_word >> mem_free_value;

  return (std::stoi(mem_total_value) - std::stoi(mem_free_value)) / stof(mem_total_value); 
}

long LinuxParser::UpTime() { 
  auto linestream = GetLineStream(kProcDirectory + kUptimeFilename);

  string rawuptime;

  linestream >> rawuptime;

  return std::stol(rawuptime); 
}

float LinuxParser::CpuUtilization() { 
  Processor p;
  return p.Utilization(); 
}

float LinuxParser::CpuUtilization(int pid){
  long system_uptime = UpTime();
  long total_time;
  float seconds;

  auto linestream = GetLineStream(kProcDirectory + to_string(pid) + kStatFilename);
  string utime, stime, cutime, cstime, starttime;

  for(int i = 1; i <= 14; i++){
    linestream >> utime;
  }
  linestream >> stime >> cutime >> cstime;

  for(int i = 18; i <= 22; i++){
    linestream >> starttime;
  }

  long hertz = sysconf(_SC_CLK_TCK);

  total_time = stol(utime) + stol(stime) + stol(cutime) + stol(cstime);
  seconds = system_uptime - (stof(starttime) / hertz);

  return total_time / hertz / seconds;
}

int LinuxParser::TotalProcesses() { 
  auto linestream = GetLineStream(kProcDirectory + kStatFilename, kStatProcessesLabel);

  string processes_word, processes_value;

  linestream >> processes_word >> processes_value;

  return std::stoi(processes_value); 
}

int LinuxParser::RunningProcesses() { 
  auto linestream = GetLineStream(kProcDirectory + kStatFilename, kStatProcsRunningLabel);

  string procs_running_word, procs_running_value;

  linestream >> procs_running_word >> procs_running_value;

  return std::stoi(procs_running_value);

}

string LinuxParser::Command(int pid) { 
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kCmdlineFilename);

  std::stringstream s;

  if(stream.is_open()){
    while(std::getline(stream, line , '\0')){
      s << line << ' ';
    }    
  }

  return s.str(); 
}

string LinuxParser::Ram(int pid) { 
  auto linestream = GetLineStream(kProcDirectory + to_string(pid) + kStatusFilename, kStatusRamLabel);
  string ram_word, ram_value;

  linestream >> ram_word >> ram_value;

  return to_string(stol(ram_value) / 1000); 
}

string LinuxParser::Uid(int pid) { 
  auto linestream = GetLineStream(kProcDirectory + to_string(pid) + kStatFilename, kStatUidLabel);

  string uid_word, uid_value;

  linestream >> uid_word >> uid_value;

  return uid_value; 
}

string LinuxParser::User(int pid) { 
  string uid = Uid(pid);
  string username;

  auto linestream = GetLineStream(kPasswordPath, "x:" + uid);
  string line = linestream.str();
  std::replace(line.begin(), line.end(), ':', ' ');

  std::istringstream s(line);
  s >> username;

  return username; 
}

long LinuxParser::UpTime(int pid) { 
  auto linestream = GetLineStream(kProcDirectory + to_string(pid) + kStatFilename);
  string uptime_value;

  const int UPTIME_POSITION = 22;

  for(int i = 1; i <= UPTIME_POSITION; i++){
    linestream >> uptime_value;
  }

  long hertz = sysconf(_SC_CLK_TCK);

  return std::stol(uptime_value) / hertz;
}
