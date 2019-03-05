#ifndef COMMON_H
#define COMMON_H

#include <sys/timeb.h>
#include <iostream>
using namespace std;

typedef int GPS_STATE;


struct Time {
    int hour, minute, second, millisecond;
    Time() {}
    Time(int h, int m, int s, int milli)
        : hour(h), minute(m), second(s), millisecond(milli) {}
};
struct Latitude {
    char hemisphere;
    int degree;
    int minute;
    int second;
    Latitude() {}
    Latitude(int hemi, int d, int m, int s)
        : degree(d), minute(m), second(s) {
            if (hemi)   // hemi == 1
                hemisphere = 'N';
            else
                hemisphere = 'S';
            
        }
};
struct Longitude {
    char hemisphere;
    int degree;
    int minute;
    int second;
    Longitude() {}
    Longitude(int hemi, int d, int m, int s)
        : degree(d), minute(m), second(s) {
            if (hemi)
                hemisphere = 'E';
            else
                hemisphere = 'W';
            
        }
};
struct gps_signal {
    int id;
    Time *t;
    Latitude *latitude;
    Longitude *longitude;
    GPS_STATE state;
    int satellite_count;
    double hdop;
    double altitude;
    double relative_height;
    int differential_time;
    int differential_station_id;

    ~gps_signal() {
        cout << "now delete generated signal memory" << endl;
        delete t;
        delete latitude;
        delete longitude;
    }
};

#endif