/***************************************************************************
 qgsquickmessagelog.qml
  --------------------------------------
  Date                 : January 2018
  Copyright            : (C) 2018 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
import QtQuick.Controls 2.0
import QtQuick 2.5
import QgsQuick 0.1 as QgsQuick


/**
 * \brief A component that shows all log messages. The component shows
 * the items from QgsQuickMessageLogModel.
 */
Item {
  /**
   * Input for messages. Must be connected before usage from QgsQuick.MessageLog.
   *
   * See also QgsQuickMessageLogModel
   */
  property alias model: table.model
  /**
   * Background color
   */
  property color bgColor: "white"
  /**
   * Separator color
   */
  property color separatorColor: "gray"

  /**
   * Separator width
   */
  property int separatorSize: 1 * QgsQuick.Utils.dp
  /**
   * True if there are unread messages in the list. All messages
   * are marked read when the widget receives visibility.
   */
  property bool unreadMessages: false

  id: item

  /**
   * List containing message logs
   */
  ListView {
    id: table
    anchors.fill: parent

    delegate: Column {
      Text {
        text: MessageTag
        font.bold: true
      }

      Text {
        text: Message
        width: table.width
        wrapMode: Text.WordWrap
      }

      /**
       * Message separator.
       */
      Rectangle {
        color: item.separatorColor
        height: item.separatorSize
        width: table.width
      }
    }
  }

  /**
   * Handles adding new messages to the list.
   */
  Connections {
    target: model

    onRowsInserted: {
      if (!visible)
        unreadMessages = true
    }
  }

  onVisibleChanged: {
    if (visible)
      unreadMessages = false
  }
}
