import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Rectangle {
  id: root
  
  signal issueTrackerClicked

  readonly property bool compactLayout: width < 640
  
  implicitHeight: compactLayout ? 88 : 60
  color: "transparent"

  GridLayout {
    anchors.fill: parent
    columns: root.compactLayout ? 1 : 2
    columnSpacing: 12
    rowSpacing: 8

    Text {
      text: qsTr("Strata combines QGIS-based GIS tools with native AI assistance. Report bugs and feature requests in the Strata issue tracker.")
      Layout.fillWidth: true
      Layout.alignment: Qt.AlignVCenter
      font.pointSize: Application.font.pointSize * 0.8
      font.bold: true
      color: "#f9fafb"
      wrapMode: Text.WordWrap
      maximumLineCount: 2
      elide: Text.ElideRight
      lineHeight: 1.3
    }

    Rectangle {
      id: issueTrackerField

      Layout.preferredWidth: root.compactLayout ? Math.min(root.width, 380) : Math.min(Math.max(root.width * 0.34, 260), 370)
      Layout.preferredHeight: 42
      Layout.alignment: root.compactLayout ? Qt.AlignLeft : Qt.AlignRight | Qt.AlignVCenter
      radius: 21
      color: issueTrackerMouseArea.containsMouse ? "#163a4d" : "#0a2b3c"
      border.width: 1
      border.color: issueTrackerMouseArea.containsMouse ? "#93b023" : "#566775"

      RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 14
        anchors.rightMargin: 14
        spacing: 8

        Text {
          text: qsTr("Report issue")
          font.pointSize: Application.font.pointSize * 0.8
          font.bold: true
          color: "#ffffff"
          elide: Text.ElideRight
        }

        Text {
          Layout.fillWidth: true
          text: qsTr("github.com/francemazzi/strata/issues")
          font.pointSize: Application.font.pointSize * 0.75
          color: "#cfe6ef"
          elide: Text.ElideMiddle
          horizontalAlignment: Text.AlignRight
        }
      }

      MouseArea {
        id: issueTrackerMouseArea
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        hoverEnabled: true
        onClicked: root.issueTrackerClicked()
      }
    }
  }
}
