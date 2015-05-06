import QtQuick 2.4

Item {
  signal dataEnd(int tag)
  signal data(int tag)
  signal error(int tag)

  function write(tag, msg) {
  }
}
