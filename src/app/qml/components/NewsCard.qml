import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Rectangle {
  id: root
  implicitWidth: 340
  implicitHeight: newsLayout.childrenRect.height + 32
  radius: 6

  property string title: ""
  property string description: ""
  property string imageSource: ""
  property string linkText: qsTr("Read more...")
  property bool showCloseButton: false

  signal readMoreClicked
  signal closeClicked

  Image {
    id: backgroundImage
    anchors.fill: parent
    source: root.imageSource
    fillMode: Image.PreserveAspectCrop
    opacity: 0.15
    visible: root.imageSource !== ""
  }

  ColumnLayout {
    id: newsLayout
    width: root.width - (root.showCloseButton ? 52 : 32)
    anchors.top: parent.top
    anchors.topMargin: 16
    anchors.left: parent.left
    anchors.leftMargin: 16
    spacing: 8

    Text {
      Layout.fillWidth: true
      text: root.title
      font.pointSize: Application.font.pointSize
      font.bold: true
      color: "#1a365d"
      wrapMode: Text.WordWrap
    }

    Text {
      Layout.fillWidth: true
      text: root.description
      font.pointSize: Application.font.pointSize * 0.8
      color: "#4a5568"
      wrapMode: Text.WordWrap
      lineHeight: 1.3
      linkColor: "#589632"
      
      onLinkActivated: link => {
        Qt.openUrlExternally(link);
      }
    }

    Text {
      Layout.fillWidth: true
      text: root.linkText
      font.pointSize: Application.font.pointSize * 0.8
      font.underline: mouseArea.containsMouse
      color: "#2b6cb0"

      MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.readMoreClicked()
      }
    }
  }

  RoundButton {
    anchors{
      top: parent.top
      right: parent.right
      margins: -8
    }

    flat: true
    icon.source: "../images/close.svg"
    icon.color: "#1a365d"
    icon.width: 20
    icon.height: 20
    onClicked: root.closeClicked()
  }
}
