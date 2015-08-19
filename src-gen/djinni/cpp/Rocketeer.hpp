// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from rocketspeed.djinni

#pragma once

#include "src-gen/djinni/cpp/InboundID.hpp"
#include "src-gen/djinni/cpp/SubscriptionParameters.hpp"

namespace rocketspeed { namespace djinni {

class Rocketeer {
public:
    virtual ~Rocketeer() {}

    virtual void HandleNewSubscription(InboundID inbound_id, SubscriptionParameters params) = 0;

    virtual void HandleTermination(InboundID inbound_id) = 0;
};

} }  // namespace rocketspeed::djinni