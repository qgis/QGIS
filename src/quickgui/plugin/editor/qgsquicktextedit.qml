/***************************************************************************
 qgsquicktextedit.qml
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

import QtQuick 2.9
import QtQuick.Controls 2.2
import QgsQuick 0.1 as QgsQuick
import QtQuick.Layouts 1.3

/**
 * Text Edit for QGIS Attribute Form
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

  TextField {
    id: textField
    height: textArea.height == 0 ? customStyle.fields.height : 0
    topPadding: 10 * QgsQuick.Utils.dp
    bottomPadding: 10 * QgsQuick.Utils.dp
    visible: height !== 0
    anchors.left: parent.left
    anchors.right: parent.right
    font.pixelSize: customStyle.fields.fontPixelSize
    color: customStyle.fields.fontColor

    text: value || ''
    inputMethodHints: field.isNumeric || widget == 'Range' ? field.precision === 0 ? Qt.ImhDigitsOnly : Qt.ImhFormattedNumbersOnly : Qt.ImhNone

    // Make sure we do not input more characters than allowed for strings
    states: [
        State {
            name: "limitedTextLengthState"
            when: (!field.isNumeric) && (field.length > 0)
            PropertyChanges {
              target: textField
              maximumLength: field.length
            }
        }
    ]

    background: Rectangle {
        anchors.fill: parent
        border.color: textField.activeFocus ? customStyle.fields.activeColor : customStyle.fields.normalColor
        border.width: textField.activeFocus ? 2 : 1
        color: customStyle.fields.backgroundColor
        radius: customStyle.fields.cornerRadius
    }

    onTextChanged: {
      valueChanged( text, text == '' )
    }
  }

  TextArea {
    id: textArea
    height: config['IsMultiline'] === true ? undefined : 0
    topPadding: customStyle.fields.height * 0.25
    bottomPadding: customStyle.fields.height * 0.25
    visible: height !== 0
    anchors.left: parent.left
    anchors.right: parent.right
    font.pixelSize: customStyle.fields.fontPixelSize
    wrapMode: Text.Wrap
    color: customStyle.fields.fontColor
    text: value || ''
    textFormat: config['UseHtml'] ? TextEdit.RichText : TextEdit.PlainText

    background: Rectangle {
        color: customStyle.fields.backgroundColor
        radius: customStyle.fields.cornerRadius
    }

    onEditingFinished: {
      valueChanged( text, text == '' )
    }
  }

}
