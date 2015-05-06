import QtQuick 2.4

Item {
  signal dataEnd(int tag)
  signal data(int tag)

  function write(tag, msg) {
  }

  function end(tag) {
  }
}
