// Copyright (c) 2014, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.
#pragma once

#include <inttypes.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <gflags/gflags.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/util.h>
#include "include/Env.h"
#include "src/util/logging.h"
#include "src/util/log_buffer.h"
#include "src/controltower/options.h"

namespace rocketspeed {

class ControlTower {
 private:
  //
  // The environment for this control tower
  Env* env_;

  // The options used by the Control Tower
  ControlTowerOptions options_;

  // The configuration of this rocketspeed instance
  Configuration conf_;

  // The event loop base.
  struct event_base *base_;

  // private Constructor
  ControlTower(const ControlTowerOptions& options,
               const Configuration& conf);

  // Sanitize input options if necessary
  ControlTowerOptions SanitizeOptions(const ControlTowerOptions& src);

  // callbacks needed by libevent
  static void readcb(struct bufferevent *bev, void *ctx);
  static void errorcb(struct bufferevent *bev, short error, void *ctx);
  static void do_accept(evutil_socket_t listener, short event, void *arg);

 public:
  // A new instance of a Control Tower
  static Status CreateNewInstance(const ControlTowerOptions& options,
                                  const Configuration& conf,
                                  ControlTower** ct);

  // Start this instance of the Control Tower
  void Run(void);

  // Returns the sanitized options used by the control tower
  ControlTowerOptions& GetOptions() {return options_;}
};

}  // namespace rocketspeed