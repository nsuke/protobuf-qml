import QtQuick 2.6
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import 'my_message.pb.js' as PB

Window {
  title: 'XMLHttpRequest Sample'
  visible: true
  height: 270
  width: 480
  minimumHeight: 180
  minimumWidth: 320

  Rectangle {
    id: root
    anchors.fill: parent
    color: '#eee'

    RowLayout {
      id: sendArea
      anchors {
        top: parent.top
        left: parent.left
        right: parent.right
        topMargin: 30
      }
      height: 50

      Item {
        Layout.preferredWidth: 120
        Layout.fillHeight: true
        Text {
          color: '#222'
          font.bold: true
          anchors.fill: parent
          text: 'Sending Text'
          verticalAlignment: Text.AlignVCenter
          horizontalAlignment: Text.AlignHCenter
        }
      }

      Item {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Rectangle {
          clip: true
          anchors {
            fill: parent
            margins: 10
          }
          radius: 4
          border {
            width: 1
            color: '#aaa'
          }
          TextInput {
            color: '#222'
            id: sendingText
            anchors {
              fill: parent
              margins: 5
            }
            text: 'Hello.'
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
          }
        }
      }

      Item {
        Layout.preferredWidth: 120
        Layout.fillHeight: true
        Layout.columnSpan: 2
        Rectangle {
          anchors {
            fill: parent
            margins: 10
          }
          color: mouseArea.containsPress ? '#a0a0a0' : (mouseArea.containsMouse ? '#a8a8a8' : '#afafaf')
          radius: 5

          Text {
            color: '#222'
            font.bold: true
            anchors.centerIn: parent
            text: 'Send Text'
          }

          MouseArea {
            id: mouseArea
            hoverEnabled: true
            anchors.fill: parent
            onClicked: {
              var xhr = new XMLHttpRequest();
              xhr.open('POST', 'http://localhost:34255/');
              xhr.onreadystatechange = function() {
                if (xhr.readyState === 4 && xhr.status === 200) {
                  var msg2 = PB.MyMessage.parse(xhr.response);
                  console.log('Server says: ' + msg2.myText);
                  receivedText.text = msg2.myText;
                }
              };
              xhr.responseType = 'arraybuffer';

              var msg = new PB.MyMessage({
                myText: sendingText.text,
              });

              var buf = msg.serialize();

              console.log('Sending: ' + msg.myText);
              xhr.send(new DataView(buf));
            }
          }
        }
      }
    }

    RowLayout {
      anchors {
        top: sendArea.bottom
        left: parent.left
        right: parent.right
      }
      height: 50


      Item {
        Layout.fillHeight: true
        Layout.preferredWidth: 120
        Text {
          color: '#222'
          font.bold: true
          anchors.fill: parent
          text: 'Received Text'
          verticalAlignment: Text.AlignVCenter
          horizontalAlignment: Text.AlignHCenter
        }
      }
      Item {
        Layout.fillWidth: true
        Layout.preferredHeight: 50
        Rectangle {
          anchors {
            fill: parent
            margins: 10
          }
          radius: 4
          border {
            width: 1
            color: '#aaa'
          }
          Text {
            color: '#222'
            id: receivedText
            anchors {
              fill: parent
              margins: 5
            }
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
          }
        }
      }
    }
  }
}
