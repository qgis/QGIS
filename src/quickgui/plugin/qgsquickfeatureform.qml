/***************************************************************************
 qgsquickfeatureform.qml
  --------------------------------------
  Date                 : Nov 2017
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

import QtQuick 2.6
import QtQuick.Controls 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import QtQml.Models 2.2
import QtQml 2.2

// We use calendar in datetime widget that is not
// yet implemented in Controls 2.2
import QtQuick.Controls 1.4 as Controls1

import QgisQuick 0.1 as QgsQuick

Item {
  signal saved
  signal canceled
  signal aboutToSave

  property QgsQuick.AttributeFormModel model
  property alias toolbarVisible: toolbar.visible
  property bool allowRememberAttribute: false // when adding new feature, add checkbox to be able to save the same value for the next feature as default
  property QgsQuick.Project project

  property var saveBtnIcon: QgsQuick.Utils.getThemeIcon( "ic_save_white" )
  property var deleteBtnIcon: QgsQuick.Utils.getThemeIcon( "ic_delete_forever_white" )
  property var closeBtnIcon: QgsQuick.Utils.getThemeIcon( "ic_clear_white" )

  property FeatureFormStyling style: FeatureFormStyling {}

  function reset() {
    master.reset()
  }

  id: form

  states: [
    State {
      name: "ReadOnly"
    },
    State {
      name: "Edit"
    },
    State {
      name: "Add"
    }
  ]

  /**
   * This is a relay to forward private signals to internal components.
   */
  QtObject {
    id: master

    /**
     * This signal is emitted whenever the state of Flickables and TabBars should
     * be restored.
     */
    signal reset
  }

  Item {
    id: container

    clip: true

    anchors {
      top: toolbar.bottom
      bottom: parent.bottom
      left: parent.left
      right: parent.right
    }

    Flickable {
      id: flickable
      anchors {
        left: parent.left
        right: parent.right
      }
      height: tabRow.height

      flickableDirection: Flickable.HorizontalFlick
      contentWidth: tabRow.width

      // Tabs
      TabBar {
        id: tabRow
        visible: model.hasTabs
        height: form.style.tabs.height

        Connections {
          target: master
          onReset: tabRow.currentIndex = 0
        }

        Connections {
          target: swipeView
          onCurrentIndexChanged: tabRow.currentIndex = swipeView.currentIndex
        }

        Repeater {
          model: form.model

          TabButton {
            id: tabButton
            text: Name
            leftPadding: 8 * QgsQuick.Utils.dp
            rightPadding: 8 * QgsQuick.Utils.dp

            width: contentItem.width + leftPadding + rightPadding
            height: form.style.tabs.height

            contentItem: Text {
              // Make sure the width is derived from the text so we can get wider
              // than the parent item and the Flickable is useful
              width: paintedWidth
              text: tabButton.text
              color: !tabButton.enabled ? form.style.tabs.disabledColor : tabButton.down ||
                                          tabButton.checked ? form.style.tabs.activeColor : form.style.tabs.normalColor
              font.weight: tabButton.checked ? Font.DemiBold : Font.Normal

              horizontalAlignment: Text.AlignHCenter
              verticalAlignment: Text.AlignVCenter
            }
          }
        }
      }
    }

    SwipeView {
      id: swipeView
      currentIndex: tabRow.currentIndex
      anchors {
        top: flickable.bottom
        left: parent.left
        right: parent.right
        bottom: parent.bottom
      }

      Repeater {
        // One page per tab in tabbed forms, 1 page in auto forms
        model: form.model.hasTabs ? form.model : 1

        Item {
          id: formPage
          property int currentIndex: index

          /**
           * The main form content area
           */
          Rectangle {
            width: parent.width
            height: parent.height
            color: form.style.backgroundColor
            opacity: form.style.backgroundOpacity
          }

          ListView {
            id: content
            anchors.fill: parent
            clip: true
            section.property: "Group"
            section.labelPositioning: ViewSection.CurrentLabelAtStart | ViewSection.InlineLabels
            section.delegate: Component {
              // section header: group box name
              Rectangle {
                width: parent.width
                height: section === "" ? 0 : form.style.group.height
                color: form.style.group.backgroundColor

                Text {
                  anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                  font.bold: true
                  text: section
                }
              }
            }

            Connections {
              target: master
              onReset: content.contentY = 0
            }

            model: QgsQuick.SubModel {
              id: contentModel
              model: form.model
              rootIndex: form.model.hasTabs ? form.model.index(currentIndex, 0) : undefined
            }

            delegate: fieldItem
          }
        }
      }
    }
  }

  /**
   * A field editor
   */
  Component {
    id: fieldItem

    Item {
      id: fieldContainer
      visible: Type === 'field'
      height: childrenRect.height

      anchors {
        left: parent.left
        right: parent.right
        leftMargin: 12 * QgsQuick.Utils.dp
      }

      Label {
        id: fieldLabel

        text: Name || ''
        font.bold: true
        color: ConstraintValid ? form.style.constraint.validColor : form.style.constraint.invalidColor
      }

      Label {
        id: constraintDescriptionLabel
        anchors {
          left: parent.left
          right: parent.right
          top: fieldLabel.bottom
        }

        text: ConstraintDescription
        height: ConstraintValid ? 0 : undefined
        visible: !ConstraintValid

        color: form.style.constraint.descriptionColor
      }

      Item {
        id: placeholder
        height: childrenRect.height
        anchors { left: parent.left; right: rememberCheckbox.left; top: constraintDescriptionLabel.bottom }

        Loader {
          id: attributeEditorLoader

          height: childrenRect.height
          anchors { left: parent.left; right: parent.right }

          enabled: form.state !== "ReadOnly" && !!AttributeEditable

          property var value: AttributeValue
          property var config: EditorWidgetConfig
          property var widget: EditorWidget
          property var field: Field
          property var constraintValid: ConstraintValid
          property var homePath: form.project ? form.project.homePath : ""

          active: widget !== 'Hidden'
          source: 'qgsquick' + widget.toLowerCase() + '.qml'

          onStatusChanged: {
            if ( attributeEditorLoader.status === Loader.Error )
            {
              console.warn( "Editor widget type '" + EditorWidget + "' is not supported" );
              source = 'qgsquicktextedit.qml';
            }
          }
        }

        Connections {
          target: form
          onAboutToSave: {
            try {
              attributeEditorLoader.item.pushChanges()
            }
            catch ( err )
            {}
          }
        }

        Connections {
          target: attributeEditorLoader.item
          onValueChanged: {
            AttributeValue = isNull ? undefined : value
          }
        }
      }

      CheckBox {
        id: rememberCheckbox
        checked: RememberValue ? true : false

        visible: form.allowRememberAttribute && form.state === "Add" && EditorWidget !== "Hidden"
        width: visible ? undefined : 0

        anchors { right: parent.right; top: fieldLabel.bottom }

        onCheckedChanged: {
          RememberValue = checked
        }
      }
    }
  }

  function save() {
    parent.focus = true
    aboutToSave()

    if ( form.state === "Add" ) {
      model.create()
      state = "Edit"
    }
    else
    {
      model.save()
    }

    saved()
  }

  Connections {
    target: Qt.inputMethod
    onVisibleChanged: {
      Qt.inputMethod.commit()
    }
  }

  /** The title toolbar **/
  Item {
    id: toolbar
    height: visible ? 48 * QgsQuick.Utils.dp : 0
    visible: form.state === 'Add'
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
    }

    RowLayout {
      anchors.fill: parent
      Layout.margins: 0

      ToolButton {
        id: saveButton
        Layout.preferredWidth: form.style.toolbutton.size
        Layout.preferredHeight: form.style.toolbutton.size

        visible: form.state !== "ReadOnly"

        contentItem: Image {
            source: form.saveBtnIcon
            sourceSize: Qt.size(width, height)
        }

        background: Rectangle {
          color: model.constraintsValid ? form.style.toolbutton.backgroundColor : form.style.toolbutton.backgroundColorInvalid
        }

        enabled: model.constraintsValid

        onClicked: {
          save()
        }
      }

      ToolButton {
        id: deleteButton

        Layout.preferredWidth: form.style.toolbutton.size
        Layout.preferredHeight: form.style.toolbutton.size

        visible: form.state === "Edit"

        contentItem: Image {
            source: form.deleteBtnIcon
            sourceSize: Qt.size(width, height)
        }

        background: Rectangle {
          color: form.style.toolbutton.backgroundColor
        }

        onClicked: deleteDialog.visible = true
      }

      Label {
        id: titleLabel

        text:
        {
          var currentLayer = model.featureModel.layer
          var layerName = 'N/A'
          if (currentLayer !== null)
            layerName = currentLayer.name

          if ( form.state === 'Add' )
            qsTr( 'Add feature on <i>%1</i>' ).arg(layerName )
          else if ( form.state === 'Edit' )
            qsTr( 'Edit feature on <i>%1</i>' ).arg(layerName)
          else
            qsTr( 'View feature on <i>%1</i>' ).arg(layerName)
        }
        font.bold: true
        font.pointSize: 16
        elide: Label.ElideRight
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
        Layout.fillWidth: true
        color: "white"
      }

      ToolButton {
        id: closeButton
        anchors.right: parent.right

        Layout.preferredWidth: form.style.toolbutton.size
        Layout.preferredHeight: form.style.toolbutton.size

        contentItem: Image {
            source: form.closeBtnIcon
            sourceSize: Qt.size(width, height)
        }

        background: Rectangle {
          color: form.style.toolbutton.backgroundColor
        }

        onClicked: {
          Qt.inputMethod.hide()

          canceled()
        }
      }
    }
  }

  MessageDialog {
    id: deleteDialog

    visible: false

    title: qsTr( "Delete feature" )
    text: qsTr( "Really delete this feature?" )
    icon: StandardIcon.Warning
    standardButtons: StandardButton.Ok | StandardButton.Cancel
    onAccepted: {
      model.featureModel.deleteFeature()
      visible = false

      canceled()
    }
    onRejected: {
      visible = false
    }
  }
}
