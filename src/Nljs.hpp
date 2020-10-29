/*
 * This file is 100% generated.  Any manual edits will likely be lost.
 *
 * This contains functions struct and other type definitions for shema in 
 * namespace ez to be serialized via nlohmann::json.
 */
#ifndef EZ_NLJS_HPP
#define EZ_NLJS_HPP


#include "Structs.hpp"


#include <nlohmann/json.hpp>

namespace ez {

    using data_t = nlohmann::json;

    NLOHMANN_JSON_SERIALIZE_ENUM( Severity, {
            { ez::Severity::Debug, "Debug" },
            { ez::Severity::Log, "Log" },
            { ez::Severity::Information, "Information" },
            { ez::Severity::Warning, "Warning" },
            { ez::Severity::Error, "Error" },
            { ez::Severity::Fatal, "Fatal" },
        })

    
    inline void to_json(data_t& j, const Context& obj) {
        j["cwd"] = obj.cwd;
        j["file_name"] = obj.file_name;
        j["function_name"] = obj.function_name;
        j["host_name"] = obj.host_name;
        j["line_number"] = obj.line_number;
        j["package_naem"] = obj.package_naem;
        j["process_id"] = obj.process_id;
        j["thread_id"] = obj.thread_id;
        j["stack_size"] = obj.stack_size;
        j["user_id"] = obj.user_id;
        j["user_name"] = obj.user_name;
        j["application_name"] = obj.application_name;
    }
    
    inline void from_json(const data_t& j, Context& obj) {
        if (j.contains("cwd"))
            j.at("cwd").get_to(obj.cwd);    
        if (j.contains("file_name"))
            j.at("file_name").get_to(obj.file_name);    
        if (j.contains("function_name"))
            j.at("function_name").get_to(obj.function_name);    
        if (j.contains("host_name"))
            j.at("host_name").get_to(obj.host_name);    
        if (j.contains("line_number"))
            j.at("line_number").get_to(obj.line_number);    
        if (j.contains("package_naem"))
            j.at("package_naem").get_to(obj.package_naem);    
        if (j.contains("process_id"))
            j.at("process_id").get_to(obj.process_id);    
        if (j.contains("thread_id"))
            j.at("thread_id").get_to(obj.thread_id);    
        if (j.contains("stack_size"))
            j.at("stack_size").get_to(obj.stack_size);    
        if (j.contains("user_id"))
            j.at("user_id").get_to(obj.user_id);    
        if (j.contains("user_name"))
            j.at("user_name").get_to(obj.user_name);    
        if (j.contains("application_name"))
            j.at("application_name").get_to(obj.application_name);    
    }
    
    inline void to_json(data_t& j, const Issue& obj) {
        j["context"] = obj.context;
        j["message"] = obj.message;
        j["severity"] = obj.severity;
    }
    
    inline void from_json(const data_t& j, Issue& obj) {
        if (j.contains("context"))
            j.at("context").get_to(obj.context);    
        if (j.contains("message"))
            j.at("message").get_to(obj.message);    
        if (j.contains("severity"))
            j.at("severity").get_to(obj.severity);    
    }
    
    // fixme: add support for MessagePack serializers (at least)

} // namespace ez

#endif // EZ_NLJS_HPP