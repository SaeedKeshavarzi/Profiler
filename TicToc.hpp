#ifndef __TIC_TOC_HPP__
#define __TIC_TOC_HPP__

#include <cstdio>

#include <chrono>

#define TIC(w) w.tic(__FILE__, __LINE__)
#define TOC(w) w.toc(__FILE__, __LINE__)

struct TicToc
{
    inline void tic(const char * file, size_t line) {
        this->line = line;  this->file = file;  time = std::chrono::system_clock::now();
    }
    inline void toc(const char * file, size_t line) {
        using namespace std::chrono;
        printf("[%s:%lu]->[%s:%lu]:\t%lu ms\n", this->file, this->line, file, line, duration_cast<milliseconds>(system_clock::now() - time).count());
    }
    const char * file;
    size_t line;
    std::chrono::system_clock::time_point time;
};

//// Example
// int main()
// {
//     TicToc tt;
//     TIC(tt);
//     std::this_thread::sleep_for(std::chrono::milliseconds(5));
//     TOC(tt);
//     std::this_thread::sleep_for(std::chrono::milliseconds(10));
//     TOC(tt);
//     return 0;
// }

#endif // __TIC_TOC_HPP__
