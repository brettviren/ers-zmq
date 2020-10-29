/*
 * This file is 100% generated.  Any manual edits will likely be lost.
 *
 * This contains struct and other type definitions for shema in 
 * namespace ez.
 */
#ifndef EZ_STRUCTS_HPP
#define EZ_STRUCTS_HPP

#include <cstdint>

#include <string>

namespace ez {

    // @brief 
    using ClassName = std::string;

    // @brief 
    using Path = std::string;

    // @brief 
    using Symbol = std::string;

    // @brief 
    using Host = std::string;

    // @brief 
    using Count = int32_t;

    // @brief 
    struct Context {

        // @brief 
        Path cwd;

        // @brief 
        Path file_name;

        // @brief 
        Symbol function_name;

        // @brief 
        Host host_name;

        // @brief 
        Count line_number;

        // @brief 
        Symbol package_naem;

        // @brief 
        Count process_id;

        // @brief 
        Count thread_id;

        // @brief 
        Count stack_size;

        // @brief 
        Count user_id;

        // @brief 
        Symbol user_name;

        // @brief 
        Symbol application_name;
    };

    // @brief 
    using Ident = uint64_t;

    // @brief 
    using Message = std::string;

    // @brief 
    enum class Severity: unsigned {
        Debug,
        Log,
        Information,
        Warning,
        Error,
        Fatal,
    };

    // @brief 
    struct Issue {

        // @brief 
        Context context;

        // @brief 
        Message message;

        // @brief 
        Severity severity;
    };

} // namespace ez

#endif // EZ_STRUCTS_HPP