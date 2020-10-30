/*
 * ZeroMQ receiver test using ez internals
 *
 */

// these are internal plugin headers
#define ZMQ_BUILD_DRAFT_API 1
#include "zmq.hpp"
#include "zmq_addon.hpp"
#include "util.hpp"
#include <nlohmann/json.hpp>

#include <cstdlib>
#include <vector>
#include <iostream>

using json = nlohmann::json;

// usage: <spec> 
int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "usage: test_ez_recv <spec> <runtime(def=1s)>" << std::endl;
        return 0;
    }
    int runtime_s = 1;
    if (argc > 2) {
        runtime_s = atoi(argv[2]);
    }
    auto links = ez::parse(argv[1], false);
    zmq::context_t ctx;
    ez::socket_set_t socks;
    ez::create(ctx, links, socks);

    zmq::poller_t<> poller;
    for (auto& one : socks) {
        poller.add(one.second, zmq::event_flags::pollin);
    };

    const auto nsocks = socks.size();
    std::vector<zmq::poller_event<>> events(nsocks);
    auto now = std::chrono::steady_clock::now();
    auto quit = now + std::chrono::seconds(runtime_s);
    
    while (now < quit) {
        auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(quit - now);
        std::cerr << "wait at most " << dt.count() << " ms" << std::endl;
        const int nevents = poller.wait_all(events, dt);
        std::cerr << "got " << nevents << " hits" << std::endl;

        if (!nevents) {
            now = std::chrono::steady_clock::now();
            continue;
        }

        for (int iev = 0; iev < nevents; ++iev) {
            auto& sock = events[iev].socket;
            zmq::message_t msg;
            auto res = sock.recv(msg, zmq::recv_flags::none);
            auto fmt = msg.to_string();
            if (!msg.more()) {
                std::cerr << "no payload to message type " << fmt << std::endl;
                continue;
            }
            if (fmt == "json") {
                res = sock.recv(msg, zmq::recv_flags::none);
                auto dat = msg.to_string();
                std::cerr << "JSON: " << dat << std::endl;
                continue;
            }
            if (fmt == "msgp") {
                res = sock.recv(msg, zmq::recv_flags::none);
                uint8_t* data = msg.data<uint8_t>();
                std::vector<uint8_t> dat(data, data + msg.size());
                auto jdat = json::from_msgpack(dat);
                std::cerr << "MSGP: " << jdat.dump() << std::endl;
                continue;
            }
            std::cerr << "unknown format: " << fmt << std::endl;
        }

        now = std::chrono::steady_clock::now();

    }
    
    return 0;
}
