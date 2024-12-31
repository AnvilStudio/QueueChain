#include "QueueChain.h"

int main() {

    QueueChain Queuechain;
    Queuechain.Start();

    anv_log::LogCreateInfo i;
    i.consoleOutput = false;
    i.fileOutput = true;
    i.logFilePath = "Log.txt";

    anv_log::AnvLog::Init(i);

    // Run tests
    StressTest_HighVolume(Queuechain);
    StressTest_RapidSwapping(Queuechain);
    StressTest_ConcurrentTaskSubmission(Queuechain);
    StressTest_EmptyQueue(Queuechain);
    StressTest_MixedOperations(Queuechain);
    StressTest_ContinuousExecution(Queuechain, 30); // 30 seconds

    std::cout << "Testing completed\n";

    Queuechain.Stop();
    return 0;

}