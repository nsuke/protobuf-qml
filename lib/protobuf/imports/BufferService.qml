import QtQuick 2.2
import Protobuf 1.0 as PB

Item {
  id: root
  property var channel
  property string serviceName: 'GenericBuffer'

  // readonly here does not work for Qt 5.2
  // readonly property var input: PB.BufferMethod {
  property var input: PB.BufferMethod {
    channel: root.channel
  }

  property var output: PB.BufferMethod {
    channel: root.channel
  }
}
