// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from rocketspeed.djinni

#pragma once

#include <functional>

namespace rocketspeed { namespace djinni {

enum class LogLevel : int {
    DEBUG_LEVEL,
    INFO_LEVEL,
    WARN_LEVEL,
    ERROR_LEVEL,
    FATAL_LEVEL,
    VITAL_LEVEL,
    NONE_LEVEL,
    NUM_INFO_LOG_LEVELS,
};

} }  // namespace rocketspeed::djinni

namespace std {

template <>
struct hash<::rocketspeed::djinni::LogLevel> {
    size_t operator()(::rocketspeed::djinni::LogLevel type) const {
        return std::hash<int>()(static_cast<int>(type));
    }
};

}  // namespace std
