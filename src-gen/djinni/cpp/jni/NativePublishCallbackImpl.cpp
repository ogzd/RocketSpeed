// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from rocketspeed.djinni

#include "src-gen/djinni/cpp/jni/NativePublishCallbackImpl.hpp"  // my header
#include "HI64.hpp"
#include "HString.hpp"
#include "src-gen/djinni/cpp/jni/NativeMsgIdImpl.hpp"
#include "src-gen/djinni/cpp/jni/NativeStatus.hpp"

namespace djinni_generated {

NativePublishCallbackImpl::NativePublishCallbackImpl() : djinni::JniInterface<::rocketspeed::djinni::PublishCallbackImpl, NativePublishCallbackImpl>() {}

NativePublishCallbackImpl::JavaProxy::JavaProxy(jobject obj) : JavaProxyCacheEntry(obj) {}

void NativePublishCallbackImpl::JavaProxy::JavaProxy::Call(::rocketspeed::djinni::Status c_status, std::string c_namespace_id, std::string c_topic_name, ::rocketspeed::djinni::MsgIdImpl c_message_id, int64_t c_sequence_number) {
    JNIEnv * const jniEnv = djinni::jniGetThreadEnv();
    djinni::JniLocalScope jscope(jniEnv, 10);
    djinni::LocalRef<jobject> j_status(jniEnv, NativeStatus::toJava(jniEnv, c_status));
    djinni::LocalRef<jstring> j_namespace_id(jniEnv, ::djinni::HString::toJava(jniEnv, c_namespace_id));
    djinni::LocalRef<jstring> j_topic_name(jniEnv, ::djinni::HString::toJava(jniEnv, c_topic_name));
    djinni::LocalRef<jobject> j_message_id(jniEnv, NativeMsgIdImpl::toJava(jniEnv, c_message_id));
    jlong j_sequence_number = ::djinni::HI64::Unboxed::toJava(jniEnv, c_sequence_number);
    const auto & data = djinni::JniClass<::djinni_generated::NativePublishCallbackImpl>::get();
    jniEnv->CallVoidMethod(getGlobalRef(), data.method_Call, j_status.get(), j_namespace_id.get(), j_topic_name.get(), j_message_id.get(), j_sequence_number);
    djinni::jniExceptionCheck(jniEnv);
};

}  // namespace djinni_generated
