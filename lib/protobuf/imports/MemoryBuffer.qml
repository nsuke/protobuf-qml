import Protobuf 1.0 as Protobuf

Protobuf.BufferService {
  property alias size: channel.size
  property alias blockSize: channel.blockSize
  function clear() { channel.clear(); }

  channel: Protobuf.MemoryBufferChannel { id: channel }
}
