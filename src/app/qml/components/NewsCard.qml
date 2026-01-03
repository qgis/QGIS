import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Rectangle {
  id: root
  implicitWidth: 340
  implicitHeight: 160
  radius: 6

  property string title: ""
  property string description: ""
  property string imageSource: ""
  property string linkText: "Read more..."
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
    anchors.fill: parent
    anchors.margins: 16
    anchors.rightMargin: root.showCloseButton ? 36 : 16
    spacing: 8

    Text {
      Layout.fillWidth: true
      text: root.title
      font.pointSize: Application.font.pointSize
      font.bold: true
      color: "#1a365d"
      wrapMode: Text.WordWrap
      elide: Text.ElideRight
      maximumLineCount: 2
    }

    Text {
      Layout.fillWidth: true
      Layout.fillHeight: true
      text: root.description
      font.pointSize: Application.font.pointSize * 0.8
      color: "#4a5568"
      wrapMode: Text.WordWrap
      elide: Text.ElideRight
      maximumLineCount: 4
      lineHeight: 1.3
    }

    Text {
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
    icon.color: "black"
    icon.width: 20
    icon.height: 20
    onClicked: root.closeClicked()
  }
}
