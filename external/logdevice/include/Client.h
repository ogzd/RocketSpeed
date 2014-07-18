/* -*- Mode: C++; tab-width: 2; c-basic-offset: 2; indent-tabs-mode: nil -*- */
#pragma once

#include <memory>
#include <string>
#include <chrono>
#include <utility>

#include "logdevice/include/AsyncReader.h"
#include "logdevice/include/ClientSettings.h"
#include "logdevice/include/Err.h"
#include "logdevice/include/Reader.h"
#include "logdevice/include/Record.h"
#include "logdevice/include/types.h"

/**
 * @file LogDevice client class. "Client" is a generic name, we expect
 *       application code to use namespaces, possibly aliasing
 *       facebook::logdevice to something shorter, like ld.
 *       See also README.
 */

namespace facebook { namespace logdevice {

class ClientImpl; // private implementation


/**
 * Type of callback that is called when a non-blocking append completes.
 *
 * @param st   E::OK on success. On failure this will be one of the error
 *             codes defined for Client::appendSync().
 *
 * @param r    contains the log id and payload passed to the async append
 *             call. If the operation succeeded (st==E::OK), it will also
 *             contain the LSN and timestamp assigned to the new record.
 *             If the operation failed, the LSN will be set to LSN_INVALID,
 *             timestamp to the time the record was accepted for delivery.
 */
typedef std::function<void(Status st, const DataRecord& r)> append_callback_t;


/**
 * Type of callback that is called when a non-blocking findTime() request
 * completes.
 *
 * See findTime() and findTimeSync() for docs.
 */
typedef std::function<void(Status, lsn_t result)> find_time_callback_t;


class Client {
public:
  /**
   * This is the only way to create new Client instances.
   *
   * @param cluster_name   name of the LogDevice cluster to connect to
   * @param config_url     a URL that identifies at a LogDevice configuration
   *                       resource (such as a file) describing the LogDevice
   *                       cluster this client will talk to. The only supported
   *                       formats are currently
   *                       file:<path-to-configuration-file> and
   *                       configerator:<configerator-path>. Examples:
   *                         "file:logdevice.test.conf"
   *                         "configerator:logdevice/logdevice.test.conf"
   * @param credentials    credentials specification. This may include
   *                       credentials to present to the LogDevice cluster
   *                       along with authentication and encryption specifiers.
   *                       Format TBD. Currently ignored.
   * @param timeout        construction timeout. This value also serves as the
   *                       default timeout for methods on the created object
   * @param settings       client settings instance to take ownership of,
   *                       or nullptr for default settings
   *
   * @return on success, a fully constructed LogDevice client object for the
   *         specified LogDevice cluster. On failure nullptr is returned
   *         and logdevice::err is set to
   *           INVALID_PARAM    invalid config URL or cluster name
   *           TIMEDOUT         timed out while trying to get config
   *           FILE_OPEN        config file could not be opened
   *           FILE_READ        error reading config file
   *           INVALID_CONFIG   various errors in parsing the config
   *           SYSLIMIT         monitoring thread for the config could
   *                            not be started
   */
  static std::shared_ptr<Client>
  create(std::string cluster_name,
         std::string config_url,
         std::string credentials,
         std::chrono::milliseconds timeout,
         std::unique_ptr<ClientSettings> &&settings) noexcept;

  /**
   * create() actually returns pointers to objects of class ClientImpl
   * that inherits from Client. The destructor must be virtual in
   * order to work correctly.
   */
  virtual ~Client() {};


  /**
   * Appends a new record to the log. Blocks until operation completes.
   * The delivery of a signal does not interrupt the wait.
   *
   * @param logid     unique id of the log to which to append a new record
   * @param payload  record payload, see Record.h. Other threads of the caller
   *                 must not modify payload data until the call returns.
   *
   * @return on success the sequence number (LSN) of new record is returned.
   *         On failure LSN_INVALID is returned and logdevice::err is set to
   *         one of:
   *    TIMEDOUT       timeout expired before operation status was known. The
   *                   record may or may not have been appended. The timeout
   *                   used is from this Client object.
   *    NOSEQUENCER    There is currently no sequencer for this log. For
   *                   example, previous instance crashed and another one
   *                   has not yet been brought up.
   *    CONNFAILED     Failed to connect to sequencer. Possible reasons:
   *                    - invalid address in cluster config
   *                    - logdeviced running the sequencer is down or
   *                      unreachable
   *    TOOBIG         Payload is too big (see Payload::maxSize())
   *    PREEMPTED      the log is configured to have at most one writer at
   *                   a time, and another writer has bumped this one
   *    NOBUFS         request could not be enqueued because a buffer
   *                   space limit was reached
   *    SYSLIMIT       a system limit on resources, such as file descriptors,
   *                   ephemeral ports, or memory has been reached. Request
   *                   was not sent.
   *    FAILED         request did not reach LogDevice cluster, or the cluster
   *                   reported that it was unable to complete the request
   *                   because its nodes were misconfigured, overloaded,
   *                   or partitioned. In rare cases the record may still be
   *                   appended to a log and delivered to readers after log
   *                   recovery is executed.
   *    ACCESS         the service denied access to this client based on
   *                   credentials presented
   *    SHUTDOWN       the logdevice::Client instance was destroyed
   *    INTERNAL       an internal error has been detected, check logs
   *    INVALID_PARAM  logid is invalid
   */
  lsn_t appendSync(logid_t logid, const Payload& payload) noexcept;


  /**
   * Appends a new record to the log without blocking. The function returns
   * control to caller as soon as the append request is put on a delivery
   * queue in this process' address space. The LogDevice client library will
   * call a callback on an unspecified thread when the operation completes.
   *
   * NOTE: records sent by calling append() method of the same Client object
   *       on the same thread are guaranteed to be sequenced (receive their
   *       sequence numbers) in the order the append() calls were made.
   *       No guarantees are made for the sequencing order of records written
   *       via append() calls made on different threads.
   *
   * @param logid     unique id of the log to which to append a new record
   * @param payload   record payload. Same as appendSync() above. The data
   *                  and the payload object itself must not be modified until
   *                  cb() is called for this payload.
   * @param cb        the callback to call
   *
   * @return  0 is returned if the request was successfully enqueued for
   *          delivery. On failure -1 is returned and logdevice::err is set to
   *             TOOBIG  if payload is too big (see Payload::maxSize())
   *             NOBUFS  if request could not be enqueued because a buffer
   *                     space limit was reached
   *      INVALID_PARAM  logid is invalid
   */
  int append(logid_t logid,
             const Payload& payload,
             append_callback_t cb) noexcept;


  /**
   * Creates a Reader object that can be used to read from one or more logs.
   *
   * Approximate memory usage when reading is:
   *   max_logs * client_read_buffer_size * (24*F + C + avg_record_size) bytes
   *
   * The constant F is between 1 and 2 depending on the
   * client_read_flow_control_threshold setting.  The constant C is
   * ClientReadStream overhead, probably a few pointers.
   *
   * When reading many logs, or when memory is important, the client read
   * buffer size can be reduced (before creating the Reader) from the default
   * 4096:
   *
   *   int rv = client->settings().set("client-read-buffer-size", 128);
   *   assert(rv == 0);
   *
   * The client can also set its individual buffer size via the optional
   * buffer_size parameter
   *
   * @param max_logs maximum number of logs that can be read from this Reader
   *                 at the same time
   * @param buffer_size specify the read buffer size for this client, fallback
   *                 to the value in settings if it is -1 or omitted
   */
  std::unique_ptr<Reader> createReader(size_t max_logs,
                                       ssize_t buffer_size = -1) noexcept;


  /**
   * Creates an AsyncReader object that can be used to read from one or more
   * logs via callbacks.
   */
  std::unique_ptr<AsyncReader> createAsyncReader() noexcept;


  /**
   * Overrides the timeout value passed to Client::create() everywhere
   * that timeout is used.
   */
  void setTimeout(std::chrono::milliseconds timeout) noexcept;


  /**
   * Ask LogDevice cluster to trim the log up to and including the specified
   * LSN. After the operation successfully completes records with LSNs up to
   * 'lsn' are no longer accessible to LogDevice clients.
   *
   * This method is synchronous -- it blocks until all storage nodes
   * acknowledge the trim command, or the timeout occurs.
   *
   * @param logid ID of log to trim
   * @param lsn   Trim the log up to this LSN (inclusive)
   * @return      Returns 0 if the request was successfully acknowledged
   *              by all nodes. Otherwise, returns -1 with logdevice::err set to
   *              E::FAILED or E::PARTIAL (if some, but not all, nodes
   *              responded -- in that case, some storage nodes might not have
   *              trimmed their part of the log, so records with LSNs less than
   *              or equal to 'lsn' might still be delivered).
   */
  int trim(logid_t logid, lsn_t lsn) noexcept;


  /**
   * Looks for the sequence number that the log was at at the given time.  The
   * most common use case is to read all records since that time, by
   * subsequently calling startReading(result_lsn).
   *
   * More precisely, this attempts to find the first LSN at or after the given
   * time.  However, if we cannot get a conclusive answer (system issues
   * prevent us from getting answers from part of the cluster), this may
   * return a slightly earlier LSN (with an appropriate status as documented
   * below).  Note that even in that case startReading(result_lsn) will read
   * all records at the given timestamp or later, but it may also read some
   * earlier records.
   *
   * If the given timestamp is earlier than all records in the log, this returns
   * the LSN after the point to which the log was trimmed.
   *
   * If the given timestamp is later than all records in the log, this returns
   * the next sequence number to be issued.  Calling startReading(result_lsn)
   * will read newly written records.
   *
   * If the log is empty, this returns LSN_OLDEST.
   *
   * All of the above assumes that records in the log have increasing
   * timestamps.  If timestamps are not monotonic, the accuracy of this API
   * may be affected.  This may be the case if the sequencer's system clock is
   * changed, or if the sequencer moves and the clocks are not in sync.
   *
   * The delivery of a signal does not interrupt the wait.
   *
   * @param logid       ID of log to query
   * @param timestamp   select the oldest record in this log whose
   *                    timestamp is greater or equal to _timestamp_.
   * @param status_out  if this argument is nullptr, it is ignored. Otherwise,
   *                    *status_out will hold the outcome of the request as
   *                    described below.
   *
   * @return
   * Returns LSN_INVALID on complete failure or an LSN as described above.  If
   * status_out is not null, *status_out can be inspected to determine the
   * accuracy of the result:
   * - E::INVALID_PARAM: logid was invalid
   * - E::OK: Enough of the cluster responded to produce a conclusive answer.
   *   Assuming monotonic timestamps, the returned LSN is exactly the first
   *   record at or after the given time.
   * - E::PARTIAL: Only part of the cluster responded and we only got an
   *   approximate answer.  Assuming monotonic timestamps, the returned LSN is
   *   no later than any record at or after the given time.
   * - E::FAILED: No storage nodes responded, or another critical failure.
   * - E::SHUTDOWN: Client was destroyed while the request was processing.
   */
  lsn_t findTimeSync(logid_t logid,
                     std::chrono::milliseconds timestamp,
                     Status *status_out = nullptr) noexcept;


  /**
   * A non-blocking version of findTimeSync().
   *
   * @return If the request was successfully submitted for processing, returns
   * 0.  In that case, the supplied callback is guaranteed to be called at a
   * later time with the outcome of the request.  See findTimeSync() for
   * documentation for the result.  Otherwise, returns -1.
   */
  int findTime(logid_t logid,
               std::chrono::milliseconds timestamp,
               find_time_callback_t cb) noexcept;


  /**
   * Looks up the boundaries of a log range by its name as specified
   * in this Client's configuration.
   *
   * @return  if configuration has a JSON object in the "logs" section
   *          with "name" attribute @param name, returns a pair containing
   *          the lowest and  highest log ids in the range (this may be the
   *          same id for log ranges of size 1). Otherwise returns a pair
   *          where both ids are set to LOGID_INVALID.
   */
  std::pair<logid_t, logid_t> getLogRangeByName(const std::string& name)
                                                                     noexcept;

  /**
   * @return  on success returns the log id at offset @param offset in log
   *          range identified in the cluster config by @param range_name.
   *          On failure returns LOGID_INVALID and sets logdevice::err to:
   *
   *            NOTFOUND if no range with @param name is present in the config
   *            INVALID_PARAM  if offset is negative or >= the range size
   */
  logid_t getLogIdFromRange(const std::string& range_name,
                            off_t offset) noexcept;


  /**
   * Exposes a ClientSettings instance that can be used to change settings
   * for the Client.
   */
  ClientSettings& settings();


private:
  Client() {}             // can be constructed by the factory only
  Client(const Client&) = delete;              // non-copyable
  Client& operator= (const Client&) = delete;  // non-assignable

  friend class ClientImpl;
  ClientImpl *impl(); // downcasts (this)
};

}} // namespace
