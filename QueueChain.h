//////////////////////////////////////////////////////////////////////
/// QueueChain is a Swapchain like class that is designed to       ///
/// take a high volume of complex tasks away from the main thread. ///
///                                                                ///
/// Usage:                                                         ///
/// Mainly for rendering. but could also be applied to physics     ///
/// The Back Queue in the chain gets written to while the Front    ///
/// Queue gets processed by a separate thread.                     ///
//////////////////////////////////////////////////////////////////////


#include <iostream>
#include <queue>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "AnvLog.h"
#include "Mat.h"

class QueueChain {
public:

    using CmdQueue = std::queue<std::function<void()>>;

    QueueChain();

    ~QueueChain();

    void Start();

    void Stop();

    void WriteToBack(const std::function<void()>& task);

    void NotifyMainDone();

    void WaitForProcessComplete();

    void Swap();


private:
    CmdQueue* m_Front  = nullptr;
    CmdQueue* m_Middle = nullptr;
    CmdQueue* m_Back   = nullptr;

    std::mutex m_QueueMutex;
    std::condition_variable m_QueueCondition;

    std::thread m_ProcThread;
    std::atomic<bool> m_StopProc;

    bool m_MainRdy;
    bool m_ProcRdy;

    // Worker thread
    void ProcessFrontQueue();
};


// Test adding a large number of tasks to the queue to ensure the system 
// can handle high loads without crashes or performance degradation
void StressTest_HighVolume(QueueChain& swapchain);

// Test swapping the queues rapidly to ensure thread synchronization remains intact
void StressTest_RapidSwapping(QueueChain& swapchain);

// Test multiple threads adding tasks to the back queue concurrently
void StressTest_ConcurrentTaskSubmission(QueueChain& swapchain);


// Test scenarios where the front, middle, or back queue is empty
void StressTest_EmptyQueue(QueueChain& swapchain);

// Test a mix of task submissions, swapping, and waiting for processing in random order
void StressTest_MixedOperations(QueueChain& swapchain);

// Run the system continuously for a specified duration to test stability under prolonged use
void StressTest_ContinuousExecution(QueueChain& swapchain, int durationSeconds);