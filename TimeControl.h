#ifndef TIMECONTROL_H
#define TIMECONTROL_H

#include <string>
#include <stdexcept>
#include <ostream>

#include <regex>
#include <sstream>
#include <iostream>


using namespace std;

enum class TimeControlType {
    Bullet,
    Blitz,
    Rapid,
    Classical,
    Unknown
};

class TimeControl{
    friend ostream& operator<<(ostream& os, const TimeControl& timecontrol);

    protected:
        TimeControlType type;
        unsigned int base_m;
        unsigned int increment_s;
    public:
        TimeControl();
        TimeControl(const string& timecontrol_str); 

        virtual ~TimeControl();

        void setBaseMinutes(const unsigned int bm);
        void setIncrementSeconds(const unsigned int is);

        TimeControlType getType() const;
        unsigned int getBaseMinutes() const;
        unsigned int getIncrementSeconds() const;    

        string toString() const;
};

TimeControlType stringToTimeControlType(const string& tc_str);
string timeControlTypeToString(TimeControlType tc);

#endif // TIMECONTROL_H