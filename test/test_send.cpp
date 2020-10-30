/**
 * Send some ERS Issues
 */

#include <ers/SampleIssues.h>
#include <ers/OutputStream.h>
#include <ers/StreamManager.h>

#include <ers/ers.h>

#include <cstdlib>
#include <chrono>
#include <thread>

void sender(int id, int period_ms, int runtime_s)
{
    // fixme: add some randomness
    auto zzz = std::chrono::milliseconds(period_ms);
    auto now = std::chrono::steady_clock::now();
    auto quit = now + std::chrono::seconds(runtime_s);
    int count = 0;
    while (now < quit) {
        auto dt = std::chrono::duration_cast<std::chrono::microseconds>(quit - now).count();
        ERS_INFO("sender " << id << ": #" << count << " @" << dt);
        std::this_thread::sleep_for(zzz);
        now = std::chrono::steady_clock::now();
        ++count;
    }
}

int main(int argc, char* argv[])
{
    int nsenders = 2;
    if (argc > 1) {
        nsenders = atoi(argv[1]);
    }
    int period_ms = 100;
    if (argc > 2) {
        period_ms = atoi(argv[1]);
    }
    int runtime_s = 1;
    if (argc > 3) {
        runtime_s = atoi(argv[2]);
    }

    std::vector<std::thread> senders;
    for (int ind=0; ind<nsenders; ++ind) {
        senders.emplace_back(([id=ind, p=period_ms, r=runtime_s]() {
            sender(id, p, r);
        }));
    }
    for (auto &sender : senders) {
        sender.join();
    }
    return 0;
}
