#pragma once
#include <stdint.h>
//#ifdef BUILD_SIMULATOR
#include <chrono>
#include <thread>
//#endif
#ifdef __linux__
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#endif

namespace Ignite
{

    using HardwareTimerCb = void (*)(void*);

    class CHardwareTimer
    {
        public:
            explicit CHardwareTimer(HardwareTimerCb cb, void* context);
            virtual ~CHardwareTimer();
            bool start(const uint16_t intervalus);
            bool stop(void);
            bool changeInterval(const uint16_t intervalus);
            uint32_t getFrequency(void) const;
            int8_t getId(void) const;

        private:
            int8_t _id = -1;

#ifdef __linux__
            pthread_t _thread;
            static void* loop(void* arg);
            void run(void);
#endif
#ifdef _WIN32
            std::thread _thread;
			void hiResTimerThread(void);
#endif
#ifdef __APPLE__
            std::thread _thread;
            void posixTimerThread(void);
            std::chrono::steady_clock::time_point _nextExpiry;
#endif
            uint32_t _intervalus = 0;
            HardwareTimerCb _callback;
            void* _context;
            bool _running = false;
            bool _activeCB = false;
#ifdef __linux__
            struct timespec _timepoint;
#endif
    };
} // namespace Ignite
