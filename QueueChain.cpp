#include "QueueChain.h"


QueueChain::QueueChain() : m_StopProc(false), m_MainRdy(false), m_ProcRdy(false) 
{
    m_Front  = new CmdQueue();
    m_Middle = new CmdQueue();
    m_Back   = new CmdQueue();
}

QueueChain::~QueueChain() {
    Stop();
}

void QueueChain::Start() {
    m_StopProc = false;
    m_ProcThread = std::thread(&QueueChain::ProcessFrontQueue, this);
}

void QueueChain::Stop() {
    {
        std::unique_lock<std::mutex> lock(m_QueueMutex);
        m_StopProc = true;
        m_QueueCondition.notify_all();
    }
    if (m_ProcThread.joinable()) {
        m_ProcThread.join();
    }
}

void QueueChain::WriteToBack(const std::function<void()>& task) {
    std::lock_guard<std::mutex> lock(m_QueueMutex);
    m_Back->push(task);
    //std::cout << "[Main] Task added to back queue.\nNew size: " << back.size() << '\n' << std::flush;
}

void QueueChain::NotifyMainDone() {
    std::unique_lock<std::mutex> lock(m_QueueMutex);
    m_MainRdy = true;
    //std::cout << "[Main] Setting main_ready to true and notifying condition.\n";
    m_QueueCondition.notify_all();
}

void QueueChain::WaitForProcessComplete() {
    std::unique_lock<std::mutex> lock(m_QueueMutex);
    m_QueueCondition.wait(lock, [this]() { return m_ProcRdy; });
    m_ProcRdy = false;
    //std::cout << "[Main] Processing thread has completed.\n" << std::flush;
}

void QueueChain::Swap() {
    std::unique_lock<std::mutex> lock(m_QueueMutex);

    //std::cout << "[Main] Before swap:\n"
    //    << "front size: " << front.size() << "\n"
    //    << "middle size: " << middle.size() << "\n"
    //    << "back size: " << back.size() << "\n";

    if (m_Back->empty()) {
        //std::cout << "[Main] Back queue is empty. No swap performed.\n";
        return;
    }

    // Rotate queues
    std::swap(m_Front, m_Middle);
    std::swap(m_Middle, m_Back);

    //std::cout << "[Main] After swap:\n"
    //    << "front size: " << front.size() << "\n"
    //    << "middle size: " << middle.size() << "\n"
    //    << "back size: " << back.size() << "\n";

    m_MainRdy = true;  // Notify that the front queue is ready
    m_QueueCondition.notify_all();
    //std::cout << "[Main] Queues swapped successfully.\n";
}


// Worker thread
void QueueChain::ProcessFrontQueue() {
    while (!m_StopProc) {
        std::unique_lock<std::mutex> lock(m_QueueMutex);

        //std::cout << "[Processing] Waiting for condition. main_ready: "
        //    << main_ready << ", front size: " << front.size() << "\n";

        m_QueueCondition.wait(lock, [this]() {
            return ( this->m_MainRdy) || this->m_StopProc;
        });

        if (m_StopProc) {
            break;
        }

        //std::cout << "[Processing] Woke up, processing front queue.\n";

        if (m_Front->empty())
        {
                
        }

        else{
            while (!m_Front->empty()) {
                auto task = m_Front->front();
                m_Front->pop();
                lock.unlock();
                //std::cout << "[Processing] Running cmd\n";
                if (task) task();
                lock.lock();
            }
        }

        //std::cout << "[Processing] Finished processing front queue.\n";

        m_ProcRdy = true;
        m_QueueCondition.notify_all();
    }
}




// Test adding a large number of tasks to the queue to ensure the system 
// can handle high loads without crashes or performance degradation
void StressTest_HighVolume(QueueChain& swapchain) {
    const int taskCount = 100000; // Large number of tasks
    ANV_LOG_INFO("[Test] High Volume Stress Test Started")
    for (int i = 0; i < taskCount; ++i) {
        swapchain.WriteToBack([i]() {
            std::cout << "[Task] Processing task: " << i << "\n";
            });
    }
    swapchain.Swap();
    swapchain.NotifyMainDone();
    swapchain.WaitForProcessComplete();
    ANV_LOG_INFO("[Test] High Volume Stress Test Completed.")
}

// Test swapping the queues rapidly to ensure thread synchronization remains intact
void StressTest_RapidSwapping(QueueChain& swapchain) {
    const int swapCount = 1000; // Rapid swaps
    ANV_LOG_INFO("[Test] Rapid Swapping Stress Test Started.")
    for (int i = 0; i < swapCount; ++i) {
        swapchain.WriteToBack([i]() {
            std::cout << "[Task] Task for swap iteration: " << i << "\n";
            });
        swapchain.Swap();
    }
    swapchain.NotifyMainDone();
    swapchain.WaitForProcessComplete();
    ANV_LOG_INFO("[Test] Rapid Swapping Stress Test Completed.")
}

// Test multiple threads adding tasks to the back queue concurrently
void StressTest_ConcurrentTaskSubmission(QueueChain& swapchain) {
    const int threadCount = 10;  // Number of threads adding tasks
    const int tasksPerThread = 1000; // Tasks per thread

    ANV_LOG_INFO("[Test] Concurrent Task Submission Started.");
    std::vector<std::thread> threads;

    for (int t = 0; t < threadCount; ++t) {
        threads.emplace_back([t, tasksPerThread, &swapchain]() {
            for (int i = 0; i < tasksPerThread; ++i) {
                swapchain.WriteToBack([t, i]() {
                    std::cout << "[Task] Thread " << t << ", Task " << i << "\n";
                    });
            }
            });
    }

    for (auto& thread : threads) {
        if (thread.joinable()) thread.join();
    }

    swapchain.Swap();
    swapchain.NotifyMainDone();
    swapchain.WaitForProcessComplete();
    ANV_LOG_INFO("[Test] Concurrent Task Submission Completed")
}


// Test scenarios where the front, middle, or back queue is empty
void StressTest_EmptyQueue(QueueChain& swapchain) {
    ANV_LOG_INFO("[Test] Empty Queue Stress Test Started.")

    // Swap and process with empty queues
    swapchain.Swap();
    swapchain.NotifyMainDone();
    swapchain.WaitForProcessComplete();

    // Add a task and repeat
    swapchain.WriteToBack([]() {
        std::cout << "[Task] Single task processed.\n";
        });
    swapchain.Swap();
    swapchain.NotifyMainDone();
    swapchain.WaitForProcessComplete();

    ANV_LOG_INFO("[Test] Empty Queue Stress Test Completed.")
}

// Test a mix of task submissions, swapping, and waiting for processing in random order
void StressTest_MixedOperations(QueueChain& swapchain) {
    ANV_LOG_INFO("[Test] Mixed Operations Stress Test Started.")

    for (int i = 0; i < 100; ++i) {
        if (i % 2 == 0) {
            swapchain.WriteToBack([i]() {
                std::cout << "[Task] Mixed operation task: " << i << "\n";
                });
        }
        else if (i % 3 == 0) {
            swapchain.Swap();
        }
        else {
            swapchain.NotifyMainDone();
            swapchain.WaitForProcessComplete();
        }
    }

    swapchain.Swap();
    swapchain.NotifyMainDone();
    swapchain.WaitForProcessComplete();

    ANV_LOG_INFO("[Test] Mixed Operations Stress Test Completed.")
}

// Run the system continuously for a specified duration to test stability under prolonged use
void StressTest_ContinuousExecution(QueueChain& swapchain, int durationSeconds) {
    ANV_LOG_INFO("[Test] Continuous Execution Stress Test Started")
    auto endTime = std::chrono::steady_clock::now() + std::chrono::seconds(durationSeconds);

    int taskCount = 0;
    while (std::chrono::steady_clock::now() < endTime) {
        swapchain.WriteToBack([taskCount]() {
            std::cout << "[Task] Continuous task: " << taskCount << "\n";
            mat A = GenerateRandomMatrix(4, 4);
            mat B = GenerateRandomMatrix(4, 4);
            mat loc_{ {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1} };
            MultiplyMatrices(A, B, loc_);
            std::cout << "Result from task: " << taskCount << '\n' << loc_ << '\n';
        });
        swapchain.Swap();
        swapchain.NotifyMainDone();
        swapchain.WaitForProcessComplete();
        ++taskCount;
    }

    ANV_LOG_INFO("[Test] Continuous Execution Stress Test Completed. Total tasks processed: %i", taskCount);
}




