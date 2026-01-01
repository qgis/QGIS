import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Rectangle {
  id: root

  property string message: "An update to the QuickOSM plugin is available"
  property string buttonText: "Install updates"

  signal installClicked
  signal closeClicked

  RowLayout {
    anchors.fill: parent
    anchors.leftMargin: 20
    anchors.rightMargin: 16
    spacing: 12

    Rectangle {
      Layout.preferredWidth: 20
      Layout.preferredHeight: 20
      Layout.alignment: Qt.AlignVCenter
      radius: 10
      border.color: "#f0e64a"
      color: "transparent"

      Text {
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenterOffset: 1
        text: "!"
        font.pointSize: Application.font.pointSize * 0.6
        font.bold: true
        color: "#f0e64a"
      }
    }

    Text {
      Layout.fillWidth: true
      Layout.alignment: Qt.AlignVCenter
      text: root.message
      font.pointSize: Application.font.pointSize * 0.6
      color: "#ffffff"
      elide: Text.ElideRight
    }

    Rectangle {
      Layout.preferredWidth: installButtonText.implicitWidth + 32
      Layout.preferredHeight: 32
      Layout.alignment: Qt.AlignVCenter
      radius: 16
      color: "transparent"
      border.color: "#f0e64a"

      Text {
        id: installButtonText
        anchors.centerIn: parent
        text: root.buttonText
        font.pointSize: Application.font.pointSize * 0.6
        font.bold: true
        color: "#ffffff"
      }

      MouseArea {
        id: installMouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.installClicked()
      }
    }

    RoundButton {
      Layout.minimumWidth: 16
      Layout.minimumHeight: 16
      Layout.alignment: Qt.AlignVCenter

      flat: true
      icon.source: "../images/close.svg"
      icon.width: 16
      icon.height: 16
      icon.color: "#ffffff"
      onClicked: root.closeClicked()
    }
  }
}
