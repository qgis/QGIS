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

import QtQuick 2.11
import QtQuick.Controls 2.4
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

    id: fieldItem
    enabled: !readOnly
    height: childrenRect.height
    anchors {
      left: parent.left
      right: parent.right
      rightMargin: 10 * QgsQuick.Utils.dp
    }


    ColumnLayout {
        id: main
        property var isDateTimeType: field.type === Qt.DateTime || field.type === Qt.Date || field.type === Qt.Time
        property var currentValue: isDateTimeType? value : Qt.formatDateTime(value, config['field_format'])

        anchors { right: parent.right; left: parent.left }

        Item {
            Layout.fillWidth: true
            Layout.minimumHeight: customStyle.height

            TextField {
                id: label

                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: customStyle.fontPixelSize
                padding: 0
                background: Rectangle {
                    radius: customStyle.cornerRadius

                    border.color: label.activeFocus ? customStyle.activeColor : customStyle.normalColor
                    border.width: label.activeFocus ? 2 : 1
                    color: customStyle.backgroundColor
                }

                inputMethodHints: Qt.ImhDigitsOnly

                // this is a bit difficult to auto generate input mask out of date/time format using regex
                // mainly because number of characters is a variable (e.g. "d": the day as number without a leading zero)
                inputMask:      if (config['display_format'] === "yyyy-MM-dd" ) { "9999-99-99;_" }
                                else if (config['display_format'] === "yyyy.MM.dd" ) { "9999.99.09;_" }
                                else if (config['display_format'] === "yyyy-MM-dd HH:mm:ss" ) { "9999-99-09 99:99:99;_" }
                                else if (config['display_format'] === "HH:mm:ss" ) { "99:99:99;_" }
                                else if (config['display_format'] === "HH:mm" ) { "99:99;_" }
                                else { "" }

                text: {
                    if ( main.currentValue === undefined )
                      {
                          qsTr('(no date)')
                      }
                      else
                      {
                          if ( main.isDateTimeType )
                          {
                              Qt.formatDateTime(main.currentValue, config['display_format'])
                          }
                          else
                          {
                              var date = Date.fromLocaleString(Qt.locale(), main.currentValue, config['field_format'])
                              Qt.formatDateTime(date, config['display_format'])
                          }
                      }
            }

                color: main.currentValue === undefined ? 'transparent' : customStyle.fontColor

                MouseArea {
                    enabled: config['calendar_popup']
                    anchors.fill: parent
                    onClicked: {
                        popup.open()
                    }
                }

                onTextEdited: {
                    var date = Date.fromLocaleString(Qt.locale(), label.text, config['display_format'])
                    if ( date.toLocaleString() !== "" )
                    {
                        if ( main.isDateTimeType )
                        {
                            main.currentValue = date
                        }
                        else
                        {
                            main.currentValue = Qt.formatDateTime(date, config['field_format'])
                        }
                        valueChanged(main.currentValue, main.currentValue === undefined)
                    }
                    else
                    {
                        valueChanged(undefined, true)
                    }
                }

                onActiveFocusChanged: {
                    if (activeFocus) {
                        var mytext = label.text
                        var cur = label.cursorPosition
                        while ( cur > 0 )
                        {
                            if (!mytext.charAt(cur-1).match("[0-9]") )
                                break
                            cur--
                        }
                        label.cursorPosition = cur
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
            x: (window.width - width) / 2
            y: (window.height - height) / 2

            ColumnLayout {

                Controls1.Calendar {
                    id: calendar
                    selectedDate: main.currentValue || new Date()
                    weekNumbersVisible: true
                    focus: false

                    onSelectedDateChanged: {
                        if ( main.isDateTimeType )
                        {
                            main.currentValue = selectedDate
                        }
                        else
                        {
                            main.currentValue = Qt.formatDateTime(selectedDate, config['field_format'])
                        }
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
            valueChanged(main.currentValue, main.currentValue === undefined)
            if (main.currentValue === undefined)
            {
                label.text = qsTr('(no date)')
                label.color = customStyle.fontColor
            }
            else
            {
                label.color = customStyle.fontColor
                label.text = new Date(value).toLocaleString(Qt.locale(), config['display_format'] )
            }
        }
    }
}
