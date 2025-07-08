#include "Time.h"
#include <string>
#include <iostream>

using namespace std;

Time::Time()
    : day(1), month(1), year(1970),
      hour(0), minute(0), second(0), zone(0.0) {}

Time::Time(const string& time_str) {
    year   = stoi(time_str.substr(0, 4));
    month  = stoi(time_str.substr(5, 2));
    day    = stoi(time_str.substr(8, 2));
    hour   = stoi(time_str.substr(11, 2));
    minute = stoi(time_str.substr(14, 2));
    second = stoi(time_str.substr(17, 2));

    char sign = time_str[20];  
    int zone_hour   = stoi(time_str.substr(21, 2));
    int zone_minute = stoi(time_str.substr(24, 2));

    zone = zone_hour + (zone_minute / 60.0);
    if (sign == '-') {
        zone *= -1;
    }
}

Time::~Time() {}

unsigned long long Time::toSeconds() const {
    unsigned long long totalSeconds = 0;
    totalSeconds += static_cast<unsigned long long>(year) * 365 * 86400ULL;
    totalSeconds += static_cast<unsigned long long>(month) * 30 * 86400ULL;
    totalSeconds += static_cast<unsigned long long>(day) * 86400ULL;
    totalSeconds += static_cast<unsigned long long>(hour) * 3600ULL;
    totalSeconds += static_cast<unsigned long long>(minute) * 60ULL;
    totalSeconds += second;

    long long zoneOffset = static_cast<long long>(zone * 3600.0);
    return static_cast<long long>(totalSeconds) - zoneOffset;
}

bool Time::operator==(const Time& other) const {
    return toSeconds() == other.toSeconds();
}

bool Time::operator<(const Time& other) const {
    return toSeconds() < other.toSeconds();
}

bool Time::operator<=(const Time& other) const {
    return toSeconds() <= other.toSeconds();
}

bool Time::operator>=(const Time& other) const {
    return toSeconds() >= other.toSeconds();
}

bool Time::operator>(const Time& other) const {
    return toSeconds() > other.toSeconds();
}

// Getters
unsigned int Time::getDay() const { return day; }
int Time::getMonth() const { return month; }
int Time::getYear() const { return year; }
unsigned int Time::getHour() const { return hour; }
unsigned int Time::getMinute() const { return minute; }
unsigned int Time::getSecond() const { return second; }
double Time::getTimeZone() const { return zone; }

// Setters
void Time::setDay(unsigned int d) { day = d; }
void Time::setMonth(int m) { month = m; }
void Time::setYear(int y) { year = y; }
void Time::setHour(unsigned int h) { hour = h; }
void Time::setMinute(unsigned int m) { minute = m; }
void Time::setSecond(unsigned int s) { second = s; }
void Time::setTimeZone(double z) { zone = z; }

// Overload <<
ostream& operator<<(ostream& os, const Time t) {
    os << t.year << "-"
       << (t.month < 10 ? "0" : "") << t.month << "-"
       << (t.day < 10 ? "0" : "") << t.day << " "
       << (t.hour < 10 ? "0" : "") << t.hour << ":"
       << (t.minute < 10 ? "0" : "") << t.minute << ":"
       << (t.second < 10 ? "0" : "") << t.second;

    int zoneHour = static_cast<int>(abs(t.zone));
    int zoneMinute = static_cast<int>((abs(t.zone) - zoneHour) * 60 + 0.5); 
    char sign = (t.zone >= 0) ? '+' : '-';

    os << " " << sign
       << (zoneHour < 10 ? "0" : "") << zoneHour << ":"
       << (zoneMinute < 10 ? "0" : "") << zoneMinute;

    return os;
}

// Methods

string Time::toString() const {
    stringstream ss;
    ss << *this; 
    return ss.str();
}