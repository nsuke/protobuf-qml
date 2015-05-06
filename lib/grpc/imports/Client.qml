import QtQuick 2.4

Item {
  signal data(int tag)
  signal error(int tag)

  function write(tag, msg) {
    msg.write(impl);
  }
}
