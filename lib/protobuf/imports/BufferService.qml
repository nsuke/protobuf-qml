import QtQuick 2.2
import Protobuf 1.0 as PB

Item {
  id: root
  property var channel
  property string serviceName: 'GenericBuffer'

  // readonly for those properties does not seem to work for Qt 5.2
  property var input: PB.Method {
    channel: root.channel
  }

  property var output: PB.Method {
    channel: root.channel
  }
}
