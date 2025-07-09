#include "TimeControl.h"

// Constructors
TimeControl::TimeControl() : type(TimeControlType::Unknown), base_m(0), increment_s(0) {}

TimeControl::TimeControl(const string& timecontrol_str) {

    unsigned int baseMinutes = 0;
    unsigned int incrementSeconds = 0;

    string str = timecontrol_str;
    str.erase(remove(str.begin(), str.end(), ' '), str.end());

    size_t plusPos = str.find('+');
    if (plusPos != string::npos) {
        string minPart = str.substr(0, plusPos);
        string secPart = str.substr(plusPos + 1);

        size_t minPos = minPart.find("min");
        if (minPos != string::npos)
            minPart = minPart.substr(0, minPos);
        else if ((minPos = minPart.find("m")) != string::npos)
            minPart = minPart.substr(0, minPos);

            size_t secPos = secPart.find("sec");
            if (secPos != string::npos)
                secPart = secPart.substr(0, secPos);
            else if ((secPos = secPart.find("s")) != string::npos)
                secPart = secPart.substr(0, secPos);

            baseMinutes = minPart.empty() ? 0 : stoi(minPart);
            incrementSeconds = secPart.empty() ? 0 : stoi(secPart);
        }

        setBaseMinutes(baseMinutes);
        setIncrementSeconds(incrementSeconds);
    }

   


// Destructor
TimeControl::~TimeControl() = default;

// Setters
void TimeControl::setBaseMinutes(const unsigned int bm) {
    base_m = bm;
    int totalSeconds = base_m * 60 + 40 * increment_s;
    if (totalSeconds < 180) type = TimeControlType::Bullet;
    else if (totalSeconds < 600) type = TimeControlType::Blitz;
    else if (totalSeconds < 1500) type = TimeControlType::Rapid;
    else type = TimeControlType::Classical;
}

void TimeControl::setIncrementSeconds(const unsigned int is) {
    increment_s = is;
    int totalSeconds = base_m * 60 + 40 * increment_s;
    if (totalSeconds < 180) type = TimeControlType::Bullet;
    else if (totalSeconds < 600) type = TimeControlType::Blitz;
    else if (totalSeconds < 1500) type = TimeControlType::Rapid;
    else type = TimeControlType::Classical;
}

// Getters
TimeControlType TimeControl::getType() const {
    return type;
}

unsigned int TimeControl::getBaseMinutes() const {
    return base_m;
}

unsigned int TimeControl::getIncrementSeconds() const {
    return increment_s;
}

TimeControlType stringToTimeControlType(const string& tc_str) {
    string lower_tc;
    for (char c : tc_str) lower_tc += tolower(c);

    if (lower_tc == "bullet") return TimeControlType::Bullet;
    if (lower_tc == "blitz") return TimeControlType::Blitz;
    if (lower_tc == "rapid") return TimeControlType::Rapid;
    if (lower_tc == "classical") return TimeControlType::Classical;
    return TimeControlType::Unknown;
}

string timeControlTypeToString(TimeControlType tc) {
    switch (tc) {
        case TimeControlType::Bullet: return "Bullet";
        case TimeControlType::Blitz: return "Blitz";
        case TimeControlType::Rapid: return "Rapid";
        case TimeControlType::Classical: return "Classical";
        default: return "Unknown";
    }
}

ostream& operator<<(ostream& os, const TimeControl& timecontrol) {
    os << to_string(timecontrol.base_m) + "+" + to_string(timecontrol.increment_s) << " (" << timeControlTypeToString(timecontrol.getType()) << ")";
    return os;
}

string TimeControl::toString() const{
    return to_string(base_m) + "+" + to_string(increment_s);
}
