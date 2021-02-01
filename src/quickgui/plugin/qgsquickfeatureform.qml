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

// We use calendar in datetime widget that is not yet implemented in Controls 2.2
import QtQuick.Controls 1.4 as Controls1

import QgsQuick 0.1 as QgsQuick

Item {
  /**
   * When feature in the form is saved.
   */
  signal saved
  /**
   * When the form is about to be closed by closeButton or deleting a feature.
   */
  signal canceled

   /**
    * A handler for extra events in externalSourceWidget.
    */
  property var externalResourceHandler: QtObject {

        /**
         * Called when clicked on the camera icon to capture an image.
         * \param itemWidget editorWidget for modified field to send valueChanged signal.
         */
        property var capturePhoto: function captureImage(itemWidget) {
        }

        /**
         * Called when clicked on the gallery icon to choose a file in a gallery.
         * \param itemWidget editorWidget for modified field to send valueChanged signal.
         */
        property var chooseImage: function chooseImage(itemWidget) {
        }

        /**
          * Called when clicked on the photo image. Suppose to be used to bring a bigger preview.
          * \param imagePath Absolute path to the image.
          */
        property var previewImage: function previewImage(imagePath) {
        }

        /**
          * Called when clicked on the trash icon. Suppose to delete the value and optionally also the image.
          * \param itemWidget editorWidget for modified field to send valueChanged signal.
          * \param imagePath Absolute path to the image.
          */
        property var removeImage: function removeImage(itemWidget, imagePath) {
        }

        /**
          * Called when clicked on the OK icon after taking a photo with the Photo panel.
          * \param itemWidget editorWidget for modified field to send valueChanged signal.
          * \param prefixToRelativePath Together with the value creates absolute path
          * \param value Relative path of taken photo.
          */
        property var confirmImage: function confirmImage(itemWidget, prefixToRelativePath, value) {
          itemWidget.image.source = prefixToRelativePath + "/" + value
          itemWidget.valueChanged(value, value === "" || value === null)
        }
    }

  /**
   * Support for custom callback on events happening in widgets
   */
  property var customWidgetCallback: QtObject {

    /**
     * Called when user clicks on valuerelation widget and combobox shall open
     * \param widget valuerelation widget for specific field to send valueChanged signal.
     * \param valueRelationModel model of type FeaturesListModel bears features of related layer.
     */
    property var valueRelationOpened: function valueRelationOpened( widget, valueRelationModel ) {}

    /**
     * Called when field for value relation is created, by default it returns value "combobox".
     * Return value of this function sets corresponding widget type. Currently accepted values are:
     *    - combobox -> QML combobox component.
     *    - textfield -> custom text widget that shows only title of selected feature in value relation
     *                   and calls function "valueRelationOpened" when it is clicked.
     * \param widget valuerelation widget for specific field to send valueChanged signal.
     * \param valueRelationModel model of type FeaturesListModel bears features of related layer.
     */
    property var getTypeOfWidget: function getTypeOfWidget( widget, valueRelationModel ) {
      return "combobox"
    }
  }

  /**
   * AttributeFormModel binded on a feature supporting auto-generated editor layouts and "tab" layout.
   */
  property QgsQuick.AttributeFormModel model

  /**
   * Visibility of toolbar.
   */
  property alias toolbarVisible: toolbar.visible

  /**
   * When adding a new feature, add checkbox to be able to save the same value for the next feature as default.
   */
  property bool allowRememberAttribute: false

  /**
   * Active project.
   */
  property QgsQuick.Project project

  /**
   * The function used for a component loader to find qml edit widget components used in form.
   */
  property var loadWidgetFn: QgsQuick.Utils.getEditorComponentSource

  /**
   * Icon path for save button.
   */
  property string saveButtonIcon: QgsQuick.Utils.getThemeIcon( "ic_save_white" )
  /**
   * Icon path for delete button.
   */
  property string deleteButtonIcon: QgsQuick.Utils.getThemeIcon( "ic_delete_forever_white" )
  /**
   * Icon path for close button
   */
  property string closeButtonIcon: QgsQuick.Utils.getThemeIcon( "ic_clear_white" )

  /**
   * Predefined form styling
   */
  property FeatureFormStyling style: FeatureFormStyling {}

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

  function reset() {
    master.reset()
  }

  function save() {
    if ( !model.constraintsHardValid )
    {
      console.log( qsTr( 'Constraints not valid') )
      return
    }
    else if ( !model.constraintsSoftValid )
    {
      console.log( qsTr( 'Note: soft constraints were not met') )
    }

    parent.focus = true
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

  /**
    * Forward change about remembering values to model
    */
  onAllowRememberAttributeChanged: form.model.rememberValuesAllowed = allowRememberAttribute

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

  Rectangle {
    id: container

    clip: true
    color: form.style.tabs.backgroundColor

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
      height: form.model.hasTabs ? tabRow.height : 0

      flickableDirection: Flickable.HorizontalFlick
      contentWidth: tabRow.width

      // Tabs
      TabBar {
        id: tabRow
        visible: model.hasTabs
        height: form.style.tabs.height
        spacing: form.style.tabs.spacing

        background: Rectangle {
          anchors.fill: parent
          color: form.style.tabs.backgroundColor
        }

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
            anchors.bottom: parent.bottom

            width: leftPadding + rightPadding
            height: form.style.tabs.buttonHeight

            contentItem: Text {
              // Make sure the width is derived from the text so we can get wider
              // than the parent item and the Flickable is useful
              Component.onCompleted: tabButton.width = tabButton.width + paintedWidth

              width: paintedWidth
              text: tabButton.text
              color: !tabButton.enabled ? form.style.tabs.disabledColor : tabButton.down ||
                                          tabButton.checked ? form.style.tabs.activeColor : form.style.tabs.normalColor
              font.weight: tabButton.checked ? Font.DemiBold : Font.Normal

              horizontalAlignment: Text.AlignHCenter
              verticalAlignment: Text.AlignVCenter
            }

            background: Rectangle {
              color: !tabButton.enabled ? form.style.tabs.disabledBackgroundColor : tabButton.down ||
                                                 tabButton.checked ? form.style.tabs.activeBackgroundColor : form.style.tabs.normalBackgroundColor
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
        /**
         * One page per tab in tabbed forms, 1 page in auto forms
         */
        model: form.model.hasTabs ? form.model : 1

        Item {
          id: formPage
          property int currentIndex: index

          /**
           * The main form content area
           */
          Rectangle {
            anchors.fill: parent
            color: form.style.backgroundColor
            opacity: form.style.backgroundOpacity
          }

          ListView {
            id: content
            anchors.fill: parent
            clip: true
            spacing: form.style.group.spacing
            section.property: "Group"
            section.labelPositioning: ViewSection.CurrentLabelAtStart | ViewSection.InlineLabels
            section.delegate: Component {

            // section header: group box name
            Rectangle {
                width: parent.width
                height: section === "" ? 0 : form.style.group.height
                color: form.style.group.marginColor

                Rectangle {
                  anchors.fill: parent
                  anchors {
                    leftMargin: form.style.group.leftMargin
                    rightMargin: form.style.group.rightMargin
                    topMargin: form.style.group.topMargin
                    bottomMargin: form.style.group.bottomMargin
                  }
                  color: form.style.group.backgroundColor

                  Text {
                    anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                    font.bold: true
                    font.pixelSize: form.style.group.fontPixelSize
                    text: section
                    color: form.style.group.fontColor
                  }
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
              rootIndex: form.model.hasTabs ? form.model.index(currentIndex, 0) : QgsQuick.Utils.invalidIndex()
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

        text: Name ? qsTr(Name) : ''
        font.bold: true
        color: ConstraintSoftValid && ConstraintHardValid ? form.style.constraint.validColor : form.style.constraint.invalidColor
      }

      Label {
        id: constraintDescriptionLabel
        anchors {
          left: parent.left
          right: parent.right
          top: fieldLabel.bottom
        }

        text: ConstraintDescription ? qsTr(ConstraintDescription) : ''
        visible: !ConstraintHardValid || !ConstraintSoftValid
        height: visible ? undefined : 0
        wrapMode: Text.WordWrap
        color: form.style.constraint.descriptionColor
      }

      Item {
        id: placeholder
        height: childrenRect.height
        anchors { left: parent.left; right: rememberCheckboxContainer.left; top: constraintDescriptionLabel.bottom }

        Loader {
          id: attributeEditorLoader

          height: childrenRect.height
          anchors { left: parent.left; right: parent.right }

          signal dataHasChanged() // to propagate signal to valuerelation model from model

          property var value: AttributeValue
          property var config: EditorWidgetConfig
          property var widget: EditorWidget
          property var field: Field
          property var constraintHardValid: ConstraintHardValid
          property var constraintSoftValid: ConstraintSoftValid
          property bool constraintsHardValid: form.model.constraintsHardValid
          property bool constraintsSoftValid: form.model.constraintsSoftValid
          property var homePath: form.project ? form.project.homePath : ""
          property var customStyle: form.style
          property var externalResourceHandler: form.externalResourceHandler
          property bool readOnly: form.state == "ReadOnly" || !AttributeEditable
          property var featurePair: form.model.attributeModel.featureLayerPair
          property var activeProject: form.project
          property var customWidget: form.customWidgetCallback

          active: widget !== 'Hidden'

          source: {
            if ( widget !== undefined )
               return form.loadWidgetFn(widget.toLowerCase())
            else return ''
          }
        }

        Connections {
          target: attributeEditorLoader.item
          onValueChanged: {
            AttributeValue = isNull ? undefined : value
          }
        }

        Connections {
          target: form.model
          onDataChanged: {
            if ( attributeEditorLoader.item && attributeEditorLoader.item.dataUpdated )
            {
              attributeEditorLoader.item.dataUpdated( form.model.attributeModel.featureLayerPair.feature )
            }
          }
        }

        Connections {
          target: form
          ignoreUnknownSignals: true
          onSaved: {
            if (attributeEditorLoader.item && typeof attributeEditorLoader.item.callbackOnSave === "function") {
              attributeEditorLoader.item.callbackOnSave()
            }
          }
        }

        Connections {
          target: form
          ignoreUnknownSignals: true
          onCanceled: {
            if (attributeEditorLoader.item && typeof attributeEditorLoader.item.callbackOnCancel === "function") {
              attributeEditorLoader.item.callbackOnCancel()
            }
          }
        }
      }

      Item {
        id: rememberCheckboxContainer
        visible: form.allowRememberAttribute && form.state === "Add" && EditorWidget !== "Hidden"

        implicitWidth: visible ? 40 * QgsQuick.Utils.dp : 0
        implicitHeight: placeholder.height

        anchors {
          top: constraintDescriptionLabel.bottom
          right: parent.right
        }

        QgsQuick.CheckboxComponent {
          id: rememberCheckbox
          visible: rememberCheckboxContainer.visible
          baseColor: form.style.checkboxComponent.baseColor

          implicitWidth: 40 * QgsQuick.Utils.dp
          implicitHeight: width
          x: -5 // hack to get over placeholder spacing
          y: rememberCheckboxContainer.height/2 - rememberCheckbox.height/2

          onCheckboxClicked: RememberValue = buttonState
          checked: RememberValue ? true : false
        }

        MouseArea {
          anchors.fill: parent
          onClicked: rememberCheckbox.checkboxClicked( !rememberCheckbox.checkState )
        }
      }
    }
  }

  Connections {
    target: Qt.inputMethod
    onVisibleChanged: {
      Qt.inputMethod.commit()
    }
  }

  /** The form toolbar **/
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
          source: form.saveButtonIcon
          sourceSize: Qt.size(width, height)
        }

        background: Rectangle {
          color: model.constraintsSoftValid && model.constraintsHardValid ? form.style.toolbutton.backgroundColor : form.style.toolbutton.backgroundColorInvalid
        }

        enabled: model.constraintsHardValid

        onClicked: {
          form.save()
        }
      }

      ToolButton {
        id: deleteButton

        Layout.preferredWidth: form.style.toolbutton.size
        Layout.preferredHeight: form.style.toolbutton.size

        visible: form.state === "Edit"

        contentItem: Image {
          source: form.deleteButtonIcon
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
          var currentLayer = model.attributeModel.featureLayerPair.layer
          var layerName = 'N/A'
          if (!!currentLayer)
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
        Layout.alignment: Qt.AlignRight

        Layout.preferredWidth: form.style.toolbutton.size
        Layout.preferredHeight: form.style.toolbutton.size

        contentItem: Image {
          source: form.closeButtonIcon
          sourceSize: Qt.size(width, height)
        }

        background: Rectangle {
          color: form.style.toolbutton.backgroundColor
        }

        onClicked: {
          Qt.inputMethod.hide()
          form.canceled()
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
      model.attributeModel.deleteFeature()
      visible = false

      form.canceled()
    }
    onRejected: {
      visible = false
    }
  }
}

