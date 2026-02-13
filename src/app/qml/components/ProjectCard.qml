import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material.impl
import QtQuick.Effects

Rectangle {
  id: root
  implicitWidth: 280
  implicitHeight: 120
  radius: 6

  property string title: ""
  property string subtitle: ""
  property string crs: ""
  property string imageSource: ""
  property bool isPinned: false
  property bool isSelected: false
  property bool isPressed: false

  signal clicked(MouseEvent mouse)

  Rectangle {
    id: imageContainer
    anchors.fill: parent
    color: "#ffffff"
    radius: root.radius

    Image {
      id: sourceImage
      anchors.fill: parent
      source: root.imageSource
      fillMode: Image.PreserveAspectCrop
      cache: false
      opacity: 0.75
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
    
    Image {
      anchors.top: parent.top
      anchors.topMargin: 15
      anchors.right: parent.right
      anchors.rightMargin: 15
      source: "qrc:/images/themes/default/pin.svg"
      width: 24
      height: 24
      layer.enabled: true
      opacity: root.isPinned ? 1 : 0
      Behavior on opacity  {
        PropertyAnimation {
          duration: 500
          easing.type: Easing.OutQuart
        }
      }
    }
  }

  ColumnLayout {
    anchors.fill: parent
    anchors.margins: 12
    spacing: 4

    Text {
      Layout.fillWidth: true
      Layout.rightMargin: 20
      text: root.title
      font.pointSize: Application.font.pointSize
      font.bold: true
      color: "#2d3748"
      wrapMode: Text.Wrap
      elide: Text.ElideRight
    }
    
    Text {
      Layout.fillWidth: true
      Layout.rightMargin: 40
      visible: root.crs != ""
      text: root.crs
      font.pointSize: Application.font.pointSize * 0.9
      color: "#4a5568"
      wrapMode: Text.NoWrap
      elide: Text.ElideRight
    }

    Text {
      Layout.fillWidth: true
      Layout.rightMargin: 40
      Layout.fillHeight: true
      text: root.subtitle
      font.pointSize: Application.font.pointSize * 0.9
      color: "#4a5568"
      wrapMode: Text.Wrap
      elide: Text.ElideRight
    }
  }

  MouseArea {
    id: mouseArea
    anchors.fill: parent
    acceptedButtons: Qt.LeftButton | Qt.RightButton
    cursorShape: Qt.PointingHandCursor
    
    onClicked: (mouse) => {
                 root.clicked(mouse);
               }
  }
}
