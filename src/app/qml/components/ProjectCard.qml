import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material.impl
import QtQuick.Effects

Item {
  id: root
  implicitWidth: 280
  implicitHeight: 100

  property int radius: 0
  property string title: ""
  property string subtitle: ""
  property string imageSource: ""
  property bool isSelected: false
  property bool isPressed: false

  signal clicked

  Item {
    id: imageContainer
    anchors.fill: parent

    Image {
      id: sourceImage
      anchors.fill: parent
      source: root.imageSource
      fillMode: Image.PreserveAspectCrop
      visible: false
      cache: false
      layer.enabled: true
    }

    Item {
      id: mask
      anchors.fill: parent
      layer.enabled: true
      visible: false

      Rectangle {
        anchors.fill: parent
        radius: root.radius
      }
    }

    MultiEffect {
      anchors.fill: sourceImage
      source: sourceImage
      maskEnabled: true
      maskSource: mask
    }
    
    Rectangle {
      anchors.fill: parent
      radius: root.radius
      gradient: Gradient {
        orientation: Gradient.Horizontal
        GradientStop { position: 0.0; color: "#ffffff" }
        GradientStop { position: 1.0; color: "transparent" }
      }
    }
    
    Ripple {
      clip: true
      width: imageContainer.width
      height: imageContainer.height
      pressed: mouseArea.pressed
      active: mouseArea.pressed
      color: "#3393b023"
    }
  }

  ColumnLayout {
    anchors.fill: parent
    anchors.margins: 12
    spacing: 4

    Text {
      Layout.fillWidth: true
      text: root.title
      font.pointSize: 12
      font.bold: true
      color: "#2d3748"
      wrapMode: Text.WordWrap
      elide: Text.ElideRight
      maximumLineCount: 1
    }

    Text {
      Layout.preferredWidth: parent.width / 3 * 2
      Layout.fillHeight: true
      text: root.subtitle
      font.pointSize: 11
      color: "#4a5568"
      wrapMode: Text.WordWrap
      elide: Text.ElideRight
      maximumLineCount: 2
    }
  }

  MouseArea {
    id: mouseArea
    anchors.fill: parent
    cursorShape: Qt.PointingHandCursor
    onClicked: root.clicked()
  }
}
