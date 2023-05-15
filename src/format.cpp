#include <string>
#include <iomanip>
#include <sstream>

#include "format.h"

using std::string;
using std::setw;
using std::setfill;
using std::stringstream;

string Format::ElapsedTime(long seconds) {
    long hours = seconds / (60 * 60);
    seconds -= hours * (60 * 60);
    long minutes = seconds / 60;
    seconds -= minutes * 60;    

    stringstream h, m, s;

    h << setw(2) << setfill('0') << hours;
    m << setw(2) << setfill('0') << minutes;
    s << setw(2) << setfill('0') << seconds;

    return h.str() + ":" + m.str() + ":" + s.str(); 
}