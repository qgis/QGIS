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

import QtQuick 2.0
import QtQuick.Controls 2.2
import QtQuick 2.5
import QgsQuick 0.1 as QgsQuick

/**
 * Text Edit for QGIS Attribute Form
 * Requires various global properties set to function, see qgsquickfeatureform Loader section
 * Do not use directly from Application QML
 */
Item {
  signal valueChanged(var value, bool isNull)

  height: childrenRect.height

  TextField {
    id: textField
    height: textArea.height == 0 ? fontMetrics.height + 20 * QgsQuick.Utils.dp : 0
    topPadding: 10 * QgsQuick.Utils.dp
    bottomPadding: 10 * QgsQuick.Utils.dp
    visible: height !== 0
    anchors.left: parent.left
    anchors.right: parent.right
    font.pointSize: 28

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
      y: textField.height - height - textField.bottomPadding / 2
      implicitWidth: 120 * QgsQuick.Utils.dp
      height: textField.activeFocus ? 2 * QgsQuick.Utils.dp : 1 * QgsQuick.Utils.dp
      color: textField.activeFocus ? "#4CAF50" : "#C8E6C9"
    }

    onTextChanged: {
      valueChanged( text, text == '' )
    }
  }

  TextArea {
    id: textArea
    height: config['IsMultiline'] === true ? undefined : 0
    visible: height !== 0
    anchors.left: parent.left
    anchors.right: parent.right
    font.pointSize: 28
    wrapMode: "WordWrap"

    text: value || ''
    textFormat: config['UseHtml'] ? TextEdit.RichText : TextEdit.PlainText

    onEditingFinished: {
      valueChanged( text, text == '' )
    }
  }

  FontMetrics {
    id: fontMetrics
    font: textField.font
  }
}
