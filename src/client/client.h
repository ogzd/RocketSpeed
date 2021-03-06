// Copyright (c) 2014, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.
//
#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <set>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#include "include/Slice.h"
#include "include/Status.h"
#include "include/Types.h"
#include "include/RocketSpeed.h"
#include "src/client/publisher.h"
#include "src/client/smart_wake_lock.h"
#include "src/messages/messages.h"
#include "src/messages/stream_socket.h"
#include "src/util/common/base_env.h"
#include "src/util/common/statistics.h"

namespace rocketspeed {

class ClientEnv;
class Subscriber;
class MessageReceived;
class MsgLoop;
class Logger;
class WakeLock;

/** Implementation of the client interface. */
class ClientImpl : public Client {
 public:
  static Status Create(ClientOptions client_options,
                       std::unique_ptr<ClientImpl>* client,
                       bool is_internal = false);

  ClientImpl(ClientOptions options,
             std::unique_ptr<MsgLoop> msg_loop,
             bool is_internal);

  virtual ~ClientImpl();

  void SetDefaultCallbacks(SubscribeCallback subscription_callback,
                           MessageReceivedCallback deliver_callback,
                           DataLossCallback data_loss_callback)
      override;

  virtual PublishStatus Publish(const TenantID tenant_id,
                                const Topic& name,
                                const NamespaceID& namespaceId,
                                const TopicOptions& options,
                                const Slice& data,
                                PublishCallback callback,
                                const MsgId messageId) override;

  SubscriptionHandle Subscribe(SubscriptionParameters parameters,
                               MessageReceivedCallback deliver_callback,
                               SubscribeCallback subscription_callback,
                               DataLossCallback data_loss_callback)
      override;

  SubscriptionHandle Subscribe(
      TenantID tenant_id,
      NamespaceID namespace_id,
      Topic topic_name,
      SequenceNumber start_seqno,
      MessageReceivedCallback deliver_callback = nullptr,
      SubscribeCallback subscription_callback = nullptr,
      DataLossCallback data_loss_callback = nullptr) override {
    return Subscribe({tenant_id,
                      std::move(namespace_id),
                      std::move(topic_name),
                      start_seqno},
                     std::move(deliver_callback),
                     std::move(subscription_callback),
                     std::move(data_loss_callback));
  }

  Status Unsubscribe(SubscriptionHandle sub_handle) override;

  Status Acknowledge(const MessageReceived& message) override;

  void SaveSubscriptions(SaveSubscriptionsCallback save_callback) override;

  Status RestoreSubscriptions(
      std::vector<SubscriptionParameters>* subscriptions) override;

  Statistics GetStatisticsSync();

  /**
   * Stop the event loop processing, and wait for thread join.
   * Client callbacks will not be invoked after this point.
   * Stop() is idempotent.
   */
  void Stop();

 private:
  /** Options provided when creating the Client. */
  ClientOptions options_;

  /** A wake lock used on mobile devices. */
  SmartWakeLock wake_lock_;

  std::unique_ptr<MsgLoop> msg_loop_;
  BaseEnv::ThreadId msg_loop_thread_;
  bool msg_loop_thread_spawned_;

  /** State of the Client, sharded by workers. */
  std::vector<std::unique_ptr<Subscriber>> worker_data_;

  // If this is an internal client, then we will skip TenantId
  // checks and namespaceid checks.
  const bool is_internal_;

  /** The publisher object, which handles write path in the client. */
  PublisherImpl publisher_;

  /** Default callback for announcing subscription status. */
  SubscribeCallback subscription_cb_fallback_;
  /** Default callbacks for delivering messages. */
  MessageReceivedCallback deliver_cb_fallback_;
  /** Default callback for data loss */
  DataLossCallback data_loss_callback_;

  /** Next subscription ID seed to be used for new subscription ID. */
  std::atomic<uint64_t> next_sub_id_;

  /** Starts the client. */
  Status Start();

  /**
   * Returns a new subscription handle. This method is thread-safe.
   *
   * @param worker_id A worker this subscription will be bound to.
   * @return A handle, if fails to allocate returns a null-handle.
   */
  SubscriptionHandle CreateNewHandle(int worker_id);

  /**
   * Extracts worker ID from provided subscription handle.
   * In case of error, returned worker ID is negative.
   */
  int GetWorkerID(SubscriptionHandle sub_handle) const;

  template <typename M>
  std::function<void(std::unique_ptr<Message>, StreamID)> CreateCallback();
};

}  // namespace rocketspeed
