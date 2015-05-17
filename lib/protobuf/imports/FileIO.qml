import Protobuf 1.0 as Protobuf

Protobuf.GenericStreamProcessor {
  property alias path: impl.path
  function exists() { impl.exists(); }
  function clear() { impl.clear(); }

  channel: Protobuf.FileIOImpl { id: impl }
}
