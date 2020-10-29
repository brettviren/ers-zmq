/**
 * This is a private header */

#ifndef ERS_ZMQ_UTIL_HPP
#define ERS_ZMQ_UTIL_HPP

#include "zmq_addon.hpp"

// We define some custom ERS Issues but as this is a plugin library
// the app may at best catch via the ers::Issue base type.
#include <ers/Issue.h>

#include <string>
#include <vector>
#include <map>

// As this is a plugin there's little point to have a namespace but
// let's use one just to be a little internally tidy.

namespace ez {


/* An ERS-ZMQ <spec> describes one or more socket configurations.  
 *
 * It is a string of elements separated by comma, space or newline.
 *
 * A spec element is either a URL as described below or a file path.
 * Named files are idenfitied as lacking any ":" character and are
 * assumed to hold a spec.
 *
 * URLs may be provided following standard ZeroMQ URL address format
 * with the extension that the following URL query parameters are
 * supported:
 *
 *  \li borc=[bind|connect] will set if the socket associated to the
 *  URL will bind() or connect().  Default is for an output to bind
 *  and an input to connect.  
 *
 *  \li type=[<socket type>] set the socket type to use as a canonical
 *  ZeroMQ socket type name, case insensitive ("pub", "pull", etc).
 *  Default is for output to use PUB and input to use SUB.
 *
 *  \li name=<name> a free form but simple identifier string
 *  (alphanumeric) for a socket.  It allows multiple bind or connect
 *  on the same socket.  If unset an unamed socket is implicitly
 *  defined.  If two or more entries identify different socket types
 *  for the same name, first one listed wins.
 *
 *  \li fmt=[json,msgp] default is json.  set to msgp to serialise
 *  using message pack.  the format is sent as first message frame.
 *
 * It is up to the user to assure consistent socket types (eg, one
 * side binds PUB, other side connects SUB).
 *
 * Example:
 *
 *  \li tcp://127.0.0.1:12345/?borc=connect&type=push&name=logz
 */

/// Parsed spec info for one socket
    struct spec_t {
        /// A socket identifier 
        std::string ident{""};
        
        /// A socket type
        int type{-1};           // -1 marks undefined
        zmq::socket_type ztype() const {
            return static_cast<zmq::socket_type>(type);
        }

        /// ZeroMQ addresses this socket shall bind.
        std::vector<std::string> binds;

        /// ZeroMQ addresses this socket shall connect.
        std::vector<std::string> conns;

    };


/// Map socket ident to its specs
    using linkmap_t = std::map<std::string, spec_t>;


    int to_type(std::string stype);

    using queries_t = std::map<std::string, std::string>;
    queries_t parse_queries(std::string qs);

    std::vector<std::string> load_spec(const std::string& spec);
    linkmap_t parse(const std::string& spec, bool sender);

    std::string get(const queries_t& qps, const std::string& key,
                    const std::string& def="");

    std::vector<std::string> slurp(const std::string& filename);

    // Add spec to links.  An input should set sender=false
    bool add(linkmap_t& links, const std::string& spec, bool sender=true);
    
    // Map socket ident/name to the socket.  Socket is held by value here.
    using socket_set_t = std::map<std::string, zmq::socket_t>;

    void create(zmq::context_t& ctx, const linkmap_t& links, socket_set_t& socks);

    zmq::multipart_t convert(const ers::Issue& issue, std::string fmt="json");

    // Send multipart but don't clear it.  Return true if okay.
    bool send_mp(zmq::socket_t& sock, zmq::multipart_t& mp);

}

// These must be caught in the app as base ers::Issue

ERS_DECLARE_ISSUE(ez, UnknownSocketType,
                  "Unknown socket type: " << stype,
                  ((const char*) stype))
ERS_DECLARE_ISSUE(ez, UnknownLinkType,
                  "Unknown link type: " << ltype,
                  ((const char*) ltype))
ERS_DECLARE_ISSUE(ez, UnknownFormat,
                  "Unknown format: " << ftype,
                  ((const char*) ftype))


#endif
