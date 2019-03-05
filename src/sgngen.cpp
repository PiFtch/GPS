#include "../include/common.h"
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <cmath>
#include <string>
using namespace std;

extern void numToChar(int value, char *buf);

void sgngen(gps_signal * signal) {
    // get system time
    timeb tb;
    ftime(&tb);
    {
        int hour, minute, second;
        hour = (tb.time / 3600) % 24;
        minute = tb.time / 60 - (tb.time / 3600) * 60;
        second = tb.time % 60;
        signal->t = new Time(hour, minute, second, tb.millitm);
    }

    // get latitude
    {
        srand(time(NULL));
        int hemi, degree, minute, second;
        hemi = rand() % 2;
        degree = rand() % 60;
        minute = rand() % 60;
        second = rand() % 60;
        signal->latitude = new Latitude(hemi, degree, minute, second);
    }

    // get longitude
    {
        // srand(rand());
        int hemi, degree, minute, second;
        hemi = rand() % 2;
        degree = rand() % 60;
        minute = rand() % 60;
        second = rand() % 60;
        signal->longitude = new Longitude(hemi, degree, minute, second);
    }

    // get GPS state
    int state[4] = {0, 1, 2, 6};
    signal->state = state[rand() % 4];

    // satellite count
    signal->satellite_count = rand() % 13;

    // hdop
    signal->hdop = (double)(rand() % 100) + rand() % 10 / 10.0;

    // get altitude
    double sgn = pow(-1, rand() % 2);
    if (sgn > 0)
        signal->altitude = sgn * ((double)(rand() % 100000) + rand() % 10 / 10.0);
    else
        signal->altitude = sgn * ((double)(rand() % 10000) + rand() % 10 / 10.0);
    
    // get relative height

    // get differential time

    // get differential station

    
    // clear memory
}

void gen_message(char *buf, gps_signal *signal) {
    char cstr[128];
    memset(cstr, 0, sizeof(cstr));
    // strcat(cstr, "$GPGGA");
    char temp[10];
    double minute;
    minute = signal->latitude->minute + signal->latitude->second / 60.0;
    sprintf(cstr, "$GPGGA,%02d%02d%02d.%03d,%02d%07.4lf,%c,%02d%07.4lf,%c,%01d,%02d,%.1lf,%.1lf,M,", signal->t->hour, signal->t->minute, signal->t->second, signal->t->millisecond, signal->latitude->degree, minute, signal->latitude->hemisphere, signal->longitude->degree,(double)signal->longitude->minute+signal->longitude->second/60.0, signal->longitude->hemisphere, signal->state, signal->satellite_count, signal->hdop,signal->altitude);

    strcpy(buf, cstr);
}

void get_gps_signal(char *buf) {
    gps_signal sig;
    memset(&sig, 0, sizeof(sig));
    sgngen(&sig);
    gen_message(buf, &sig);
}
/*
int main() {
    gps_signal sig;
    memset(&sig, 0, sizeof(sig));
    sgngen(&sig);
    cout << sig.value_10 << endl;
    cout << sig.t->hour << ':' << sig.t->minute << ':' << sig.t->second << ':' << sig.t->millisecond << endl;
    char buf[512];
    gen_message(buf, &sig);
    cout << buf << endl;
    string str("a");
    cout << str.length() << endl;
}
*/
