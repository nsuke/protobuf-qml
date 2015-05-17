import Protobuf 1.0 as Protobuf

Protobuf.GenericStreamProcessor {
  property alias size: impl.size
  property alias blockSize: impl.blockSize
  function clear() { impl.clear(); }

  channel: Protobuf.MemoryBufferImpl { id: impl }
}
