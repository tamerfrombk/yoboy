#pragma once

#include <cstdio>
#include <cstdlib>

#define YB_UNUSED(x) (void)x

#define YB_MEM_SIZE (0xFFFF) 

namespace yb {

    template <class... Args>
    inline void error(const char *msg, Args&&... args)
    {
        std::fprintf(stderr, msg, std::forward<Args>(args)...);
    }

    inline void error(const char *msg)
    {
        std::fputs(msg, stderr);
    }

    template <class... Args>
    inline void exit(const char *msg, Args&&... args)
    {
        std::fprintf(stderr, msg, std::forward<Args>(args)...);
        std::exit(1);
    }

    inline void exit(const char *msg)
    {
        std::fputs(msg, stderr);
        std::exit(1);
    }

    template <class... Args>
    inline void log(const char *msg, Args&&... args)
    {
        std::printf(msg, std::forward<Args>(args)...);
    }

    inline void log(const char *msg)
    {
        std::puts(msg);
    }

}

