import QtQuick
import QtQuick.Controls.impl
import QtQuick.Templates as T

T.ScrollBar {
  id: control

  property color color: "#a7a7a7"
  property real handleSize: 8

  width: horizontal ? parent.width : handleSize
  height: horizontal ? handleSize  : parent.height
  visible: control.policy !== T.ScrollBar.AlwaysOff
  anchors.right: parent.right

  background: Rectangle {
    color: "transparent"
  }

  contentItem: Item {
    property bool collapsed: control.policy === T.ScrollBar.AlwaysOn || (control.active && control.size < 1.0)

    implicitWidth: control.handleSize
    implicitHeight: Math.max(10, control.handleSize)

    Rectangle {
      id: handle
      width: vertical ? control.handleSize / 2 : parent.width
      height: horizontal ? control.handleSize / 2 : Math.max(10, parent.height)
      color: control.color
      anchors {
        right: vertical ? parent.right : undefined
        bottom: horizontal ? parent.bottom : undefined
      }
      radius: width / 2
      visible: control.size < 1.0
    }

    states: [
      State {
        name: "fullyVisible"
        when: contentItem.collapsed
        PropertyChanges {
          target: handle
          width: vertical ? control.handleSize : parent.width
          height: horizontal ? control.handleSize : Math.max(10, parent.height)
        }
      },
      State {
        name: "semiVisible"
        when: !contentItem.collapsed
        PropertyChanges {
          target: handle
          width: vertical ? control.handleSize / 2 : parent.width
          height: horizontal ? control.handleSize / 2 : Math.max(10, parent.height)
        }
      }
    ]

    transitions: [
      Transition {
        to: "semiVisible"
        NumberAnimation {
          target: handle
          properties: vertical ? "width" : "height"
          duration: 170
          easing.type: Easing.OutCubic
        }
      },
      Transition {
        to: "fullyVisible"
        NumberAnimation {
          target: handle
          properties: vertical ? "width" : "height"
          duration: 170
          easing.type: Easing.OutCubic
        }
      }
    ]
  }
}
