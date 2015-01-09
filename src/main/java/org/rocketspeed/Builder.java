package org.rocketspeed;

public final class Builder {

  private ConfigurationImpl config;
  private String clientID;
  private PublishCallbackImpl publishCallback;
  private SubscribeCallbackImpl subscribeCallback;
  private ReceiveCallbackImpl receiveCallback;
  private SubscriptionStorage storage;

  public Builder() {
    reset();
  }

  private void reset() {
    config = null;
    clientID = null;
    publishCallback = null;
    subscribeCallback = null;
    receiveCallback = null;
    storage = new SubscriptionStorage(StorageType.NONE, "");
  }

  private Builder configuration(Configuration config) {
    this.config = config.djinni();
    return this;
  }

  public Builder clientID(String clientID) {
    this.clientID = clientID;
    return this;
  }

  public Builder publishCallback(final PublishCallback callback) {
    this.publishCallback = new PublishCallbackImpl() {
      @Override
      public void Call(Status status, short namespaceId, String topicName, MsgIdImpl messageId,
                       long sequenceNumber, byte[] contents) {
        callback.call(status, namespaceId, topicName, new MsgId(messageId), sequenceNumber,
                      contents);
      }
    };
    return this;
  }

  public Builder subscribeCallback(final SubscribeCallback callback) {
    this.subscribeCallback = new SubscribeCallbackImpl() {
      @Override
      public void Call(Status status, long sequenceNumber, boolean subscribed) {
        callback.call(status, sequenceNumber, subscribed);
      }
    };
    return this;
  }

  public Builder receiveCallback(final ReceiveCallback callback) {
    this.receiveCallback = new ReceiveCallbackImpl() {
      @Override
      public void Call(short namespaceId, String topicName, long sequenceNumber, byte[] contents) {
        callback.call(new MessageReceived(namespaceId, topicName, sequenceNumber, contents));
      }
    };
    return this;
  }

  public Builder usingFileStorage(String filePath) {
    if (!StorageType.NONE.equals(storage.getType())) {
      throw new IllegalStateException();
    }
    storage = new SubscriptionStorage(StorageType.FILE, filePath);
    return this;
  }

  public Client build() {
    ClientImpl client = ClientImpl.Open(config, clientID, publishCallback, subscribeCallback,
                                        receiveCallback, storage);
    reset();
    return new Client(client);
  }
}