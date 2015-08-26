
#ifndef __HELPER_H__
#define __HELPER_H__

#include <vector>
#include <string>

#include <ctime>
#include <sys/time.h>

class Timer {
public:
    Timer() :_sum(0.0) {}

    void begin() {
        gettimeofday(&_begin_tv, NULL);
    }

    void end() {
        gettimeofday(&_end_tv, NULL);
        _sum += (_end_tv.tv_sec - _begin_tv.tv_sec) + (_end_tv.tv_usec - _begin_tv.tv_usec) * 0.000001f;
    }

    /* return unit : seconds.*/
    float cost_time() const {
        return _sum;
    }

private:
    float   _sum;

    timeval _begin_tv;
    timeval _end_tv;
};

inline void split(char* s, const char* token, std::vector<std::string>& out) {
    char* p;
    out.clear();
    char* f = strtok_r(s, token, &p);
    while (f) {
        out.push_back(f);
        f = strtok_r(NULL, token, &p); 
    }
    return ;
}

#ifndef LOG_NOTICE
#define LOG_NOTICE(format, ...) {fprintf(stderr, "NOTICE: " format "\n", ##__VA_ARGS__);}
#endif 

#ifndef LOG_ERROR
#define LOG_ERROR(format, ...) {fprintf(stderr, " ERROR: " format "\n", ##__VA_ARGS__);}
#endif

#endif
