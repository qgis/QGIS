import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Rectangle {
  id: root
  
  signal supportClicked
  signal websiteClicked
  
  implicitHeight: 60
  color: "transparent"

  RowLayout {
    anchors.fill: parent
    spacing: 8

    Text {
      text: qsTr("QGIS is a community project that relies on sustaining memberships and donations to fund many of our improvements and activities.")
      Layout.fillWidth: true
      font.pointSize: Application.font.pointSize * 0.8
      font.bold: true
      color: "#f9fafb"
      wrapMode: Text.WordWrap
      maximumLineCount: 2
      elide: Text.ElideRight
      lineHeight: 1.3
    }

    RoundButton {
      radius: 20
      Layout.preferredHeight: 50
      Layout.preferredWidth: implicitWidth * 1.2
      highlighted: true
      Material.accent: "#589632"
      text: qsTr("Support QGIS")
      icon.color: "#c53e38"
      icon.source: "../images/love.svg"
      icon.width: 20
      icon.height: 20
      font.bold: true
      background.layer.enabled: false
      onClicked: root.supportClicked()
    }

    RoundButton {
      radius: 20
      Layout.preferredHeight: 50
      Layout.preferredWidth: implicitWidth * 1.2
      highlighted: true
      Material.accent: "#8c8c8c"
      text: qsTr("Go to qgis.org")
      font.bold: true
      background.layer.enabled: false
      onClicked: root.websiteClicked()
    }
  }
}
