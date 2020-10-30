
#include "util.hpp"
#include "Nljs.hpp"

#include <ers/SampleIssues.h>

#include <algorithm>
#include <cctype>
#include <string>
#include <regex>
#include <fstream>
#include <iostream>             // durring dev

using namespace ez;

// The great Nlohmann's JSON.
using json = nlohmann::json;

queries_t ez::parse_queries(std::string qs)
{
    queries_t ret;
    if (qs.empty()) {
        return ret;
    }
    while(true) {
        auto sep = qs.find_first_of("&");
        std::string one;
        if (sep == qs.npos) {
            one = qs;
        }
        else {
            one = qs.substr(0, sep);
        }
        auto kitr = one.find_first_of("=");
        if (kitr == one.npos) {
            ret[one] = "";
        }
        else {
            auto key = one.substr(0,kitr);
            auto val = one.substr(kitr+1);
            ret[key] = val;
        }
        if (sep == qs.npos) {
            break;
        }
        qs = qs.substr(sep+1);
    }
    return ret;
}

std::string ez::get(const queries_t& qps, const std::string& key,
                    const std::string& def)
{
    const auto itr = qps.find(key);
    if (itr == qps.end()) {
        return def;
    }
    return itr->second;
}

std::vector<std::string> ez::load_spec(const std::string& spec)
{
    std::vector<std::string> urls;
    auto const re = std::regex{R"([,\r\n\s]+)"};
    auto const parts = std::vector<std::string>(
        std::sregex_token_iterator{begin(spec), end(spec), re, -1},
        std::sregex_token_iterator{});

    for (const auto& one : parts) {
        if (one.find_first_of(":") != one.npos) { // a URL
            urls.push_back(one);
            continue;
        }
        // a file
        auto many = slurp(one);
        urls.insert(urls.end(), many.begin(), many.end());
    }
    return urls;
}

// Read named file, return parts
std::vector<std::string> ez::slurp(const std::string& filename)
{
    std::ifstream fstr(filename);
    if (!fstr) {
        throw ers::CantOpenFile( ERS_HERE, filename.c_str() );
    }
    std::ostringstream sstr;
    sstr << fstr.rdbuf();
    return load_spec(sstr.str());
}

int ez::to_type(std::string stype)
{
    std::transform(stype.begin(), stype.end(), stype.begin(),
                   [](unsigned char c){ return std::toupper(c); });
    static const char* names[] = {
        "PAIR",   "PUB",  "SUB",    "REQ",     "REP",
        "DEALER",  // 5
        "ROUTER",  // 6
        "PULL",   "PUSH", "XPUB",   "XSUB",    "STREAM",
        "SERVER",  // 12
        "CLIENT",  // 13
        "RADIO",  "DISH", "GATHER", "SCATTER", "DGRAM",  0};
    int itype=0;
    while(names[itype]) {
        if (names[itype] == stype) {
            return itype;
        }
        ++itype;
    }
    throw UnknownSocketType(ERS_HERE, "stype");
}


// Add spec to links.
linkmap_t ez::parse(const std::string& spec, bool sender)
{
    linkmap_t links;
    std::vector<std::string> urls = load_spec(spec);

    for (const auto& url : urls) {
        auto q = url.find_first_of("?");
        auto addr = url.substr(0, q);
        auto qps = parse_queries(url.substr(q+1));

        std::string name = get(qps, "name", "");
        std::string borc = get(qps, "borc", sender ? "bind" : "connect");
        std::string stype = get(qps, "type", sender ? "pub" : "sub");

        auto& link = links[name];
        link.ident = name;

        if (link.type < 0) {
            link.type = to_type(stype);
        }

        if (borc == "connect") {
            link.conns.push_back(addr);
        }
        else if (borc == "bind") {
            link.binds.push_back(addr);
        }
        else {
            throw UnknownLinkType(ERS_HERE, borc.c_str());
        }
    }        
    return links;
}


void ez::create(zmq::context_t& ctx, const linkmap_t& links,
                socket_set_t& socks)
{
    for (const auto& link : links) {
        const spec_t& spec = link.second;
        socks.emplace(std::piecewise_construct,
                      std::forward_as_tuple(spec.ident),
                      std::forward_as_tuple(ctx, spec.type));
        zmq::socket_t& sock = socks[spec.ident];

        // fixme: add query parameter to provide a subscription prefix
        if (spec.ztype() == zmq::socket_type::sub) {
            sock.set(zmq::sockopt::subscribe, "");
        }

        std::cerr << "socket " << link.first << " is a " << spec.type << std::endl;
        for (const auto& one : spec.binds) {
            std::cerr << "bind " << link.first << " to " << one << std::endl;
            sock.bind(one);
        }
        for (const auto& one : spec.conns) {
            std::cerr << "connect " << link.first << " to " << one << std::endl;
            sock.connect(one);
        }
    }
}


zmq::multipart_t ez::convert(const ers::Issue& issue, std::string fmt)
{
    const auto& ctx = issue.context();

    ez::Issue eziss{
        {                       // context
            ctx.cwd(),
            ctx.file_name(),
            ctx.function_name(),
            ctx.host_name(),
            ctx.line_number(),
            ctx.package_name(),
            ctx.process_id(),
            ctx.thread_id(),
            // stack_symbols are void**, go fish
            ctx.stack_size(),
            ctx.user_id(),
            ctx.user_name(),
            ctx.application_name()
        },
        issue.message(),
        static_cast<ez::Severity>(static_cast<int>(issue.severity())),
    };
    json jiss = eziss;
    if (fmt == "json") {
        std::string siss = jiss.dump();
        zmq::multipart_t ret(fmt);
        ret.append(zmq::message_t(siss));
        return ret;
    }
    if (fmt == "msgp") {
        std::vector<std::uint8_t> dat = json::to_msgpack(jiss);
        zmq::multipart_t ret(fmt);
        ret.append(zmq::message_t(dat.data(), dat.size()));
        return ret;
    }    
    throw UnknownFormat(ERS_HERE, fmt.c_str());
}

bool ez::send_mp(zmq::socket_t& sock, zmq::multipart_t& mp)
{
    const auto size = mp.size();

    for (size_t ind=0; ind<size; ++ind) {
        zmq::message_t msg;
        msg.copy(mp[ind]);
        auto flags = zmq::send_flags::sndmore;
        if (ind == size-1) {
            flags = zmq::send_flags::none;
        }
        if (!sock.send(msg, flags)) {
            return false;
        }
    }
    return true;
}
