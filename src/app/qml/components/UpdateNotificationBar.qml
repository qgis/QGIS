import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Rectangle {
  id: root

  property string message: ""
  property string buttonText: qsTr("Install update")

  signal installClicked

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
        font.pointSize: Application.font.pointSize * 0.8
        font.bold: true
        color: "#f0e64a"
      }
    }

    Text {
      Layout.fillWidth: true
      Layout.alignment: Qt.AlignVCenter
      text: root.message
      font.pointSize: Application.font.pointSize * 0.8
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
        font.pointSize: Application.font.pointSize * 0.8
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
  }
}
