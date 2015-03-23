// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from rocketspeed.djinni

#include "src-gen/djinni/cpp/jni/NativeReceiveCallbackImpl.hpp"  // my header
#include "HBinary.hpp"
#include "HI64.hpp"
#include "HString.hpp"

namespace djinni_generated {

NativeReceiveCallbackImpl::NativeReceiveCallbackImpl() : djinni::JniInterface<::rocketspeed::djinni::ReceiveCallbackImpl, NativeReceiveCallbackImpl>() {}

NativeReceiveCallbackImpl::JavaProxy::JavaProxy(jobject obj) : JavaProxyCacheEntry(obj) {}

void NativeReceiveCallbackImpl::JavaProxy::JavaProxy::Call(std::string c_namespace_id, std::string c_topic_name, int64_t c_sequence_number, std::vector<uint8_t> c_contents) {
    JNIEnv * const jniEnv = djinni::jniGetThreadEnv();
    djinni::JniLocalScope jscope(jniEnv, 10);
    djinni::LocalRef<jstring> j_namespace_id(jniEnv, ::djinni::HString::toJava(jniEnv, c_namespace_id));
    djinni::LocalRef<jstring> j_topic_name(jniEnv, ::djinni::HString::toJava(jniEnv, c_topic_name));
    jlong j_sequence_number = ::djinni::HI64::Unboxed::toJava(jniEnv, c_sequence_number);
    djinni::LocalRef<jbyteArray> j_contents(jniEnv, ::djinni::HBinary::toJava(jniEnv, c_contents));
    const auto & data = djinni::JniClass<::djinni_generated::NativeReceiveCallbackImpl>::get();
    jniEnv->CallVoidMethod(getGlobalRef(), data.method_Call, j_namespace_id.get(), j_topic_name.get(), j_sequence_number, j_contents.get());
    djinni::jniExceptionCheck(jniEnv);
};

}  // namespace djinni_generated
