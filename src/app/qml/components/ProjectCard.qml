import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material.impl
import QtQuick.Effects

Item {
  id: root
  implicitWidth: 280
  implicitHeight: 100

  property int radius: 6
  property string title: ""
  property string subtitle: ""
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
      Layout.rightMargin: 20
      text: root.title
      font.pointSize: Application.font.pointSize
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
      font.pointSize: Application.font.pointSize * 0.9
      color: "#4a5568"
      wrapMode: Text.WordWrap
      elide: Text.ElideRight
      maximumLineCount: 2
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
