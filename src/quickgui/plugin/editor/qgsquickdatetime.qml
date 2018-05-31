/***************************************************************************
 qgsquickdatetime.qml
  --------------------------------------
  Date                 : 2017
  Copyright            : (C) 2017 by Matthias Kuhn
  Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

import QtQuick 2.0
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4 as Controls1
import QgsQuick 0.1 as QgsQuick

/**
 * Calendar for QGIS Attribute Form
 * Requires various global properties set to function, see qgsquickfeatureform Loader section
 * Do not use directly from Application QML
 */
Item {
  signal valueChanged(var value, bool isNull)

  height: childrenRect.height
  anchors { right: parent.right; left: parent.left }

  ColumnLayout {
    id: main
    property var currentValue: value

    anchors { right: parent.right; left: parent.left }

    Item {
      anchors { right: parent.right; left: parent.left }
      Layout.minimumHeight: 48 * QgsQuick.Utils.dp

      Rectangle {
        anchors.fill: parent
        id: backgroundRect
        border.color: "#17a81a"
        border.width: 2
        color: "#dddddd"
        radius: 2
      }

      Label {
        id: label

        anchors.fill: parent
        verticalAlignment: Text.AlignVCenter

        MouseArea {
          anchors.fill: parent
          onClicked: {
            popup.open()
          }
        }

        Image {
          source: QgsQuick.Utils.getThemeIcon("ic_clear_black")
          anchors.left: parent.right
          visible: main.currentValue !== undefined && config['allow_null']

          MouseArea {
            anchors.fill: parent
            onClicked: {
              main.currentValue = undefined
            }
          }
        }
      }
    }

    Popup {
      id: popup
      modal: true
      focus: true
      closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
      parent: ApplicationWindow.overlay

      ColumnLayout {

        Controls1.Calendar {
          id: calendar
          selectedDate: main.currentValue
          weekNumbersVisible: true
          focus: false

          onSelectedDateChanged: {
            main.currentValue = selectedDate
          }
        }

        RowLayout {
          Button {
            text: qsTr( "Ok" )
            Layout.fillWidth: true

            onClicked: popup.close()
          }
        }
      }
    }

    onCurrentValueChanged: {
      valueChanged(currentValue, main.currentValue === undefined)
      if (main.currentValue === undefined)
      {
        label.text = qsTr('(no date)')
        label.color = 'gray'
      }
      else
      {
        label.color = 'black'
        label.text = new Date(value).toLocaleString(Qt.locale(), config['display_format'] )
      }
    }
  }
}
