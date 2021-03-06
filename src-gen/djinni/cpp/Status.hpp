// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from rocketspeed.djinni

#pragma once

#include "src-gen/djinni/cpp/StatusCode.hpp"
#include <string>
#include <utility>

namespace rocketspeed { namespace djinni {

struct Status final {
    StatusCode code;
    std::string state;

    Status(StatusCode code,
           std::string state)
    : code(std::move(code))
    , state(std::move(state))
    {}
};

} }  // namespace rocketspeed::djinni
