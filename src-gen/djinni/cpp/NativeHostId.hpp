// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from rocketspeed.djinni

#pragma once

#include "djinni_support.hpp"
#include "src-gen/djinni/cpp/HostId.hpp"

namespace djinni_generated {

class NativeHostId final {
public:
    using CppType = ::rocketspeed::djinni::HostId;
    using JniType = jobject;

    using Boxed = NativeHostId;

    ~NativeHostId();

    static CppType toCpp(JNIEnv* jniEnv, JniType j);
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c);

private:
    NativeHostId();
    friend ::djinni::JniClass<NativeHostId>;

    const ::djinni::GlobalRef<jclass> clazz { ::djinni::jniFindClass("org/rocketspeed/HostId") };
    const jmethodID jconstructor { ::djinni::jniGetMethodID(clazz.get(), "<init>", "(Ljava/lang/String;I)V") };
    const jfieldID field_host { ::djinni::jniGetFieldID(clazz.get(), "host", "Ljava/lang/String;") };
    const jfieldID field_port { ::djinni::jniGetFieldID(clazz.get(), "port", "I") };
};

}  // namespace djinni_generated
