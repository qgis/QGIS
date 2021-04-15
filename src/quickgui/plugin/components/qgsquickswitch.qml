import QtQuick 2.0
import QtQuick.Templates 2.0 as T
import QgsQuick 0.1 as QgsQuick

T.Switch {
    id: control

    property color bgndColorActive: "green"
    property color bgndColorInactive: "grey"
    property color handleColor: "white"
    property bool isReadOnly: false

    signal switchChecked( bool isChecked )

    implicitHeight: 60 * QgsQuick.Utils.dp
    implicitWidth: 2 * height

    indicator: Rectangle {
        x: {
          let actualPosition = control.visualPosition * (control.width - width)

          if ( control.checked ) // limit maximum position
            return Math.min( actualPosition, control.width * 0.55 )
          else // limit minimum position
            return Math.max( actualPosition, control.width * 0.13 )
        }
        y: (control.height - height) / 2
        height: parent.height * 0.66
        width: height

        radius: 20 * QgsQuick.Utils.dp
        color: control.handleColor

        Behavior on x {
            enabled: !control.pressed
            SmoothedAnimation { velocity: 200 }
        }
    }

    background: Rectangle {
        radius: 20 * QgsQuick.Utils.dp
        color: control.isReadOnly || !control.checked ? control.bgndColorInactive : control.bgndColorActive
    }

    onCheckedChanged: control.switchChecked( checked )
}
