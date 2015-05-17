import Protobuf 1.0 as Protobuf

Protobuf.BufferService {
  property alias path: channel.path
  function exists() { channel.exists(); }
  function clear() { channel.clear(); }

  channel: Protobuf.FileChannel { id: channel }
}
