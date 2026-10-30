#include "../Board/CHardwareTimer.h"

#ifdef __APPLE__
#include <cstdlib>
#include <pthread.h>
#include <sched.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#endif

#ifdef __linux__
#include <climits>
#define SEC_TO_NSEC_SCALER 1000000000
#define USEC_TO_NSEC_SCALER 1000
#endif

using namespace Ignite;

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

CHardwareTimer::CHardwareTimer(HardwareTimerCb cb, void* context)
    : _callback(cb)
    , _context(context)
{
    _id = std::rand() % 100;
}

CHardwareTimer::~CHardwareTimer()
{
    stop();
}
bool CHardwareTimer::start(const uint16_t intervalus)
{
    _intervalus = intervalus;
#ifdef __linux__
    struct timespec ts;
    if (clock_gettime (CLOCK_MONOTONIC, &ts) < 0)
    {
        perror("clock_gettime");
        exit(1);
    }
    printf ("Start : Cur time %llu : %llu\n", ts.tv_sec, ts.tv_nsec);
    long long unsigned int currentTimeNs = (ts.tv_sec * SEC_TO_NSEC_SCALER) + ts.tv_nsec;
    long long unsigned int nextExpiryNs = currentTimeNs + (_intervalus * USEC_TO_NSEC_SCALER);
    _timepoint.tv_sec = nextExpiryNs / SEC_TO_NSEC_SCALER;
    _timepoint.tv_nsec = nextExpiryNs % SEC_TO_NSEC_SCALER;
    printf("_intervalus = %u\n", _intervalus);
    printf("Start : Next expiry %llu : %llu\n", _timepoint.tv_sec, _timepoint.tv_nsec);
    if (_running)
    {
        _activeCB = true;// thread we are not stoping
        return true;
    }
    struct sched_param param;
    int policy = SCHED_FIFO; // Use SCHED_FIFO for real-time scheduling
    int max_priority = sched_get_priority_max(policy);
    param.sched_priority = max_priority;

    if (pthread_create(&_thread, nullptr, &CHardwareTimer::loop, this) != 0)
    {
        std::cerr << "Failed to create thread" << std::endl;
        return EXIT_FAILURE;
    }
    _running = true;
    _activeCB = true;//updating below may not reach, as thread started safe to update now
    if (pthread_setschedparam(_thread, policy, &param) != 0)
    {
        std::cerr << "pthread_setschedparam failed: " << strerror(errno) << std::endl;
    }
    else
        std::cout << "pthread setschedparam success" << max_priority << std::endl;

    /* disabled CPU set - disabled bez thread timer has more distraction */
    //cpu_set_t cpuset;
    //CPU_ZERO(&cpuset);
    //CPU_SET(3, &cpuset); // Set affinity to CPU 0

    //if (pthread_setaffinity_np(_thread, sizeof(cpu_set_t), &cpuset) != 0) {
    //    std::cerr << "pthread_setaffinity_np failed: " << strerror(errno) << std::endl;
    //    return EXIT_FAILURE;
    //}
    //else {
    //    std::cout << "pthread_setaffinity_np success" << std::endl;
    //}
    pthread_detach(_thread);

#elif defined  _WIN32
    if (_running)
    {
        _activeCB = true;
        return false;
    }
    // Create and start the timer thread
    _thread = std::thread(&CHardwareTimer::hiResTimerThread, this);
    _thread.detach();
#elif defined __APPLE__
    _nextExpiry = std::chrono::steady_clock::now() + std::chrono::microseconds(_intervalus);
    if (_running)
    {
        _activeCB = true; // thread keeps running, just re-arm the callback
        return true;
    }
    _thread = std::thread(&CHardwareTimer::posixTimerThread, this);
    _thread.detach();
#endif
     _running = true;
    _activeCB = true;
    return true;
}

bool CHardwareTimer::stop(void)
{
    if (_running)
    {
    //_running = false; -- not stoping
    _activeCB = false;// disable the Call back
#ifdef __linux__
    //pthread_join(_thread, nullptr);
#endif
    }
    return true;
}

bool CHardwareTimer::changeInterval(const uint16_t intervalus)
{
    if (!_running)
    {
        start(intervalus);
    }
    else
    {
        _intervalus = intervalus;

#ifdef __linux__
        struct timespec ts;
        if (clock_gettime (CLOCK_MONOTONIC, &ts) < 0)
        {
            perror ("clock_gettime");
            return false;
        }
        //printf ("Change : Cur time %llu : %llu\n", ts.tv_sec, ts.tv_nsec);
        long long unsigned int currentTimeNs = (ts.tv_sec * SEC_TO_NSEC_SCALER) + ts.tv_nsec;
        long long unsigned int nextExpiryNs = currentTimeNs + (_intervalus * USEC_TO_NSEC_SCALER);
        _timepoint.tv_sec = nextExpiryNs / SEC_TO_NSEC_SCALER;
        _timepoint.tv_nsec = nextExpiryNs % SEC_TO_NSEC_SCALER;
        DBG("_intervalus :" + String(_intervalus));
        //printf ("Change : Next expiry %llu : %llu\n", _timepoint.tv_sec, _timepoint.tv_nsec);
        //std::cout << _intervalus << std::endl;;
#elif defined __APPLE__
        _nextExpiry = std::chrono::steady_clock::now() + std::chrono::microseconds(_intervalus);
#endif
    }
    return true;
}

uint32_t CHardwareTimer::getFrequency(void) const
{
    return 1000000;
}

int8_t CHardwareTimer::getId(void) const
{
    return _id;
}


#ifdef __linux__
void* CHardwareTimer::loop(void* arg) {
    CHardwareTimer* timer = static_cast<CHardwareTimer*>(arg);
    if (arg == nullptr) {
        std::cerr << "CHardwareTimer::loop Error: arg is nullptr" << std::endl;
        return nullptr;
    }
    timer->run();
    return nullptr;
}

void CHardwareTimer::run(void)
{
    while (_running)
    {
        struct timespec ts;
        if (!_activeCB)
        {
            std::this_thread::yield();
            continue;
        }
        if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0)
        {
            perror("clock_gettime");
            continue;
        }
        long long unsigned int currentTimeNs = (ts.tv_sec * SEC_TO_NSEC_SCALER) + ts.tv_nsec;
        long long unsigned int expiryTimeNs = (_timepoint.tv_sec * SEC_TO_NSEC_SCALER) + _timepoint.tv_nsec;
        //std::cout << "currentTimeNs: " << currentTimeNs << ", expiryTimeNs: " << expiryTimeNs << " _activeCB " << _activeCB << std::endl;
        //if (currentTimeNs == 0) {
        //    std::cout << "currentTimeNs is zero, likely due to system boot or clock initialization." << std::endl;
        //    continue;
        //}
        if ((currentTimeNs >= expiryTimeNs) && _callback)
        {
            long long unsigned int nextExpiryNs = expiryTimeNs + (_intervalus * USEC_TO_NSEC_SCALER);
            _timepoint.tv_sec = nextExpiryNs / SEC_TO_NSEC_SCALER;
            _timepoint.tv_nsec = nextExpiryNs % SEC_TO_NSEC_SCALER;
            _callback(_context);
        }
    }
}
#endif /* __linux__ */

#ifdef _WIN32

void PrintThreadPriority(HANDLE threadHandle)
{
    DWORD threadId = GetCurrentThreadId();
    int priority = GetThreadPriority(threadHandle);
    if (priority == THREAD_PRIORITY_ERROR_RETURN)
    {
        juce::Logger::outputDebugString("Error getting thread priority: " + std::to_string(GetLastError()));
    }
    else
    {
        juce::Logger::outputDebugString("Current thread ID: " + std::to_string(threadId) + "-Priority " + std::to_string(priority));
    }
}

void CHardwareTimer::hiResTimerThread(void)
{
    // Set the thread priority high
    HANDLE currentThread = GetCurrentThread();
    if (!SetThreadPriority(currentThread, THREAD_PRIORITY_TIME_CRITICAL))
    {
        std::cerr << "Error setting thread priority: " << GetLastError() << std::endl;
    }

    LARGE_INTEGER frequency, start, end;
    QueryPerformanceFrequency(&frequency);

    double intervalCounts = (_intervalus * frequency.QuadPart) / 1000000.0;

    QueryPerformanceCounter(&start);

    PrintThreadPriority(currentThread);

    while (_running)
    {
        QueryPerformanceCounter(&end);
        double elapsedCounts = static_cast<double>(end.QuadPart - start.QuadPart);
        if (elapsedCounts >= intervalCounts)
        {
            //MyFunction();
            _callback(_context);

            // Calculate the actual elapsed time and adjust intervalCounts
            QueryPerformanceCounter(&end);
            double actualElapsedCounts = static_cast<double>(end.QuadPart - start.QuadPart);
            double deviationCounts = static_cast<double>(actualElapsedCounts - intervalCounts);

            // Adjust intervalCounts for the next cycle
            intervalCounts = (_intervalus * frequency.QuadPart) / 1000000.0 - deviationCounts;

            // reset start time
            start = end;
        }
    }
}

#endif /* _WIN32 */

#ifdef __APPLE__
// macOS has neither the Linux SCHED_FIFO loop nor the Windows QueryPerformanceCounter
// thread above, so without this the sequencer clock never ticked on desktop Macs.
// A steady_clock thread sleeps until shortly before the next tick, then spins.
void CHardwareTimer::posixTimerThread(void)
{
    using namespace std::chrono;

    // Ask for a high scheduling priority; failure only loosens the timing.
    sched_param param {};
    const int policy = SCHED_RR;
    param.sched_priority = sched_get_priority_max(policy);
    if (pthread_setschedparam(pthread_self(), policy, &param) != 0)
        std::cerr << "CHardwareTimer: pthread_setschedparam failed: " << strerror(errno) << std::endl;

    const bool traceTicks = (std::getenv("IGNITE_TRACE") != nullptr);

    while (_running)
    {
        if (!_activeCB)
        {
            std::this_thread::sleep_for(milliseconds(1));
            continue;
        }

        const auto now = steady_clock::now();
        if (now >= _nextExpiry)
        {
            _nextExpiry += microseconds(_intervalus);
            if (_nextExpiry < now) // fell far behind (e.g. after system sleep): resync instead of bursting
                _nextExpiry = now + microseconds(_intervalus);
            if (_callback)
                _callback(_context);

            if (traceTicks)
            {
                static auto batchStart = now;
                static uint32_t ticks = 0;
                if (++ticks == 96)
                {
                    std::cout << "CHardwareTimer: 96 ticks in "
                              << duration_cast<milliseconds>(steady_clock::now() - batchStart).count()
                              << " ms, interval " << _intervalus << " us" << std::endl;
                    ticks = 0;
                    batchStart = steady_clock::now();
                }
            }
        }
        else
        {
            const auto remaining = _nextExpiry - now;
            if (remaining > microseconds(300))
                std::this_thread::sleep_for(remaining - microseconds(200));
            else
                std::this_thread::yield();
        }
    }
}
#endif /* __APPLE__ */
