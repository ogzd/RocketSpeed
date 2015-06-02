// Copyright (c) 2014, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.
//
#pragma once

#include <memory>
#include <vector>
#include "include/Status.h"
#include "include/Types.h"
#include "src/port/Env.h"
#include "src/messages/messages.h"
#include "src/util/hostmap.h"
#include "src/util/storage.h"
#include "src/util/topic_uuid.h"
#include "src/util/common/statistics.h"
#include "src/util/common/thread_check.h"
#include "src/controltower/topic.h"

namespace rocketspeed {

class LogTailer;
class LogRouter;
class LogReader;
class MsgLoop;

class TopicTailer {
 friend class ControlTowerTest;
 public:
  /**
   * Create a TopicTailer.
   *
   * @param env Environment.
   * @param msg_loop MsgLoop this tailer belongs to.
   * @param worker_id Worker index tailer runs on.
   * @param log_tailer Tailer for logs. Will be manipulated by TopicTailer.
   * @param log_router For routing topics to logs.
   * @param info_log For logging.
   * @param on_message Callback for Deliver and Gap messages.
   * @param tailer Output parameter for created TopicTailer.
   * @return ok() if TopicTailer created, otherwise error.
   */
  static Status CreateNewInstance(
   BaseEnv* env,
   MsgLoop* msg_loop,
   int worker_id,
   LogTailer* log_tailer,
   std::shared_ptr<LogRouter> log_router,
   std::shared_ptr<Logger> info_log,
   std::function<void(std::unique_ptr<Message>,
                      std::vector<HostNumber>)> on_message,
   TopicTailer** tailer);

  /**
   * Initialize the TopicTailer first before using it.
   *
   * @param reader_id ID of reader on LogTailer.
   * @param max_subscription_lag Maximum number of sequence numbers that a
   *                             subscription can lag behind before being sent
   *                             a gap.
   * @return ok if successful, otherwise error code.
   */
  Status Initialize(size_t reader_id,
                    int64_t max_subscription_lag);

  /**
   * Adds a subscriber to a topic. This call is not thread-safe.
   */
  Status AddSubscriber(const TopicUUID& topic,
                       SequenceNumber start,
                       HostNumber hostnum);

  /**
   * Removes a subscriber from a topic. This call is not thread-safe.
   */
  Status RemoveSubscriber(const TopicUUID& topic,
                          HostNumber hostnum);

  /**
   * Process a data record from a log tailer, and forward to on_message.
   *
   * @param msg Log record message.
   * @param log_id ID of log record resides in.
   * @param reader_id ID of reader delivering the record.
   * @return OK() if record was sent for processing.
   */
  Status SendLogRecord(
    std::unique_ptr<MessageData> msg,
    LogID log_id,
    size_t reader_id);

  /**
   * Process a gap record from a log tailer, and forward to on_message.
   *
   * @param log_id ID of log containing the gap.
   * @param type The type of gap encountered.
   * @param from The first gap sequence number (inclusive).
   * @param to The last gap sequence number (inclusive).
   * @param reader_id ID of reader that encountered the gap.
   * @return OK() if record was sent for processing.
   */
  Status SendGapRecord(
    LogID log_id,
    GapType type,
    SequenceNumber from,
    SequenceNumber to,
    size_t reader_id);

  const Statistics& GetStatistics() const {
    return stats_.all;
  }

  /**
   * Get an estimate of tail seqno for a log, or 0 if unknown.
   */
  SequenceNumber GetTailSeqnoEstimate(LogID log_id) const;

  /**
   * Get human-readable information about a particular log.
   */
  std::string GetLogInfo(LogID log_id) const;

  /**
   * Get human-readable information about all logs.
   */
  std::string GetAllLogsInfo() const;

  ~TopicTailer();

 private:
  typedef std::function<void()> TopicTailerCommand;

  // private constructor
  TopicTailer(BaseEnv* env,
              MsgLoop* msg_loop,
              int worker_id,
              LogTailer* log_tailer,
              std::shared_ptr<LogRouter> log_router,
              std::shared_ptr<Logger> info_log,
              std::function<void(std::unique_ptr<Message>,
                                 std::vector<HostNumber>)> on_message);

  bool Forward(std::function<void()> command);

  void AddTailSubscriber(const TopicUUID& topic,
                         HostNumber hostnum,
                         LogID logid,
                         SequenceNumber seqno);

  ThreadCheck thread_check_;

  BaseEnv* env_;

  // Message loop that we belong to, and our worker index.
  MsgLoop* msg_loop_;
  const int worker_id_;

  // The log tailer.
  LogTailer* log_tailer_;

  // Topic to log router.
  std::shared_ptr<LogRouter> log_router_;

  // Information log
  std::shared_ptr<Logger> info_log_;

  std::unique_ptr<LogReader> log_reader_;

  // Callback for outgoing messages.
  std::function<void(std::unique_ptr<Message>,
                     std::vector<HostNumber>)> on_message_;

  // Subscription information per topic
  std::unordered_map<LogID, TopicManager> topic_map_;

  // Cached tail sequence number per log.
  std::unordered_map<LogID, SequenceNumber> tail_seqno_cached_;

  struct Stats {
    Stats() {
      log_records_received =
        all.AddCounter("topic_tailer.log_records_received");
      new_tail_records_sent =
        all.AddCounter("topic_tailer.new_tail_records_sent");
      log_records_with_subscriptions =
        all.AddCounter("topic_tailer.log_records_with_subscriptions");
      log_records_without_subscriptions =
        all.AddCounter("topic_tailer.log_records_without_subscriptions");
      log_records_out_of_order =
        all.AddCounter("topic_tailer.log_records_out_of_order");
      bumped_subscriptions =
        all.AddCounter("topic_tailer.bumped_subscriptions");
      gap_records_received =
        all.AddCounter("topic_tailer.gap_records_received");
      gap_records_out_of_order =
        all.AddCounter("topic_tailer.gap_records_out_of_order");
      gap_records_with_subscriptions =
        all.AddCounter("topic_tailer.gap_records_with_subscriptions");
      gap_records_without_subscriptions =
        all.AddCounter("topic_tailer.gap_records_without_subscriptions");
      benign_gaps_received =
        all.AddCounter("topic_tailer.benign_gaps_received");
      malignant_gaps_received =
        all.AddCounter("topic_tailer.malignant_gaps_received");
      add_subscriber_requests =
        all.AddCounter("topic_tailer.add_subscriber_requests");
      add_subscriber_requests_at_0 =
        all.AddCounter("topic_tailer.add_subscriber_requests_at_0");
      add_subscriber_requests_at_0_fast =
        all.AddCounter("topic_tailer.add_subscriber_requests_at_0_fast");
      add_subscriber_requests_at_0_slow =
        all.AddCounter("topic_tailer.add_subscriber_requests_at_0_slow");
      updated_subscriptions =
        all.AddCounter("topic_tailer.updated_subscriptions");
      remove_subscriber_requests =
        all.AddCounter("topic_tailer.remove_subscriber_requests");
    }

    Statistics all;
    Counter* log_records_received;
    Counter* new_tail_records_sent;
    Counter* log_records_with_subscriptions;
    Counter* log_records_without_subscriptions;
    Counter* log_records_out_of_order;
    Counter* bumped_subscriptions;
    Counter* gap_records_received;
    Counter* gap_records_out_of_order;
    Counter* gap_records_with_subscriptions;
    Counter* gap_records_without_subscriptions;
    Counter* benign_gaps_received;
    Counter* malignant_gaps_received;
    Counter* add_subscriber_requests;
    Counter* add_subscriber_requests_at_0;
    Counter* add_subscriber_requests_at_0_fast;
    Counter* add_subscriber_requests_at_0_slow;
    Counter* updated_subscriptions;
    Counter* remove_subscriber_requests;
  } stats_;
};

}  // namespace rocketspeed
