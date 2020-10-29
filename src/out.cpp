/*
 * This provides an ERS output stream plugin that will send ERS issues
 * from a ZeroMQ socket.
 */

#include <ers/OutputStream.h>
#include <ers/StreamFactory.h>

#include "util.hpp"

/** Stream ERS issues by sending from a ZeroMQ socket
 * 
 * Configure as:
 *
 * \li ezout(<spec>)
 *
 * The <spec> is described elsewhere
 */
class EZOut : public ers::OutputStream
{
    zmq::context_t ctx;
    ez::socket_set_t socks;

  public:
    explicit EZOut( const std::string& spec) {
        auto links = ez::parse(spec, true);
        ez::create(ctx, links, socks);
    }

    virtual ~EZOut() {
        // sockets and context should take care of themselves.
    }

    virtual void write( const ers::Issue & issue ) {
        auto msg = ez::convert(issue);
        for (auto& one : socks) {
            ez::send_mp(one.second, msg);
        }
    }

};
