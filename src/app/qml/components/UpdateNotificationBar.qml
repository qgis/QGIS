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
    anchors.leftMargin: 28
    anchors.rightMargin: 28
    spacing: 20

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
        font.pointSize: tinyFontSize
        font.bold: true
        color: "#f0e64a"
      }
    }

    Text {
      Layout.fillWidth: true
      Layout.alignment: Qt.AlignVCenter
      text: root.message
      font.pointSize: tinyFontSize
      color: "#ffffff"
      elide: Text.ElideRight
    }
    
    RoundButton {
      id: installButtonText
      Layout.preferredWidth: implicitWidth * 1.2
      radius: implicitHeight / 2
      rightInset: 0
      leftInset: 0
      highlighted: true
      font.pointSize: tinyFontSize
      font.bold: true
      text: root.buttonText
      
      background: Rectangle {      
        implicitWidth: parent.Material.buttonHeight
        implicitHeight: parent.Material.buttonHeight
        radius: parent.radius
        color: "transparent"
        border.color: "#f0e64a"
      }
      
      onClicked: root.installClicked()
    }
  }
}
