/***************************************************************************
  FeaturePanel.qml
  --------------------------------------
  Date                 : Nov 2017
  Copyright            : (C) 2017 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

import QtQuick 2.7
import QtQuick.Controls 2.2
import QgisQuick 0.1 as QgsQuick

Drawer {

    property var mapSettings
    property var project

    property alias state: featureForm.state
    property alias layer: featureModel.layer
    property alias feature: featureModel.feature
    property alias currentFeatureModel: featureModel

    id: featurePanel
    visible: false
    modal: true
    interactive: true
    dragMargin: 0 // prevents opening the drawer by dragging.

    background: Rectangle {
        color: "white"
        opacity: 0.5
    }

    function show_panel(layer, feature, state) {
        if (QgsQuick.Utils.hasValidGeometry(layer, feature)) {
            // layer needs to be set before the feature otherwise the panel ends up empty on layer change
            featurePanel.layer = layer
            featurePanel.feature = feature
            featurePanel.state = state

            // visible needs to be after setting correct layer&feature,
            // so currentFeatureModel is already up to date (required for feature highlight)
            featurePanel.visible = true
        } else {
            QgsQuick.Utils.logMessage("The feature " + layer.name + " has a wrong geometry." , "YDNPA Surveys")
        }
    }

    QgsQuick.FeatureForm {
      id: featureForm

      // using anchors here is not working well as
      width: featurePanel.width
      height: featurePanel.height

      model: QgsQuick.AttributeFormModel {
        featureModel: QgsQuick.FeatureModel {
            id: featureModel
        }
      }

      project: featurePanel.project

      toolbarVisible: true

      onSaved: {
        featurePanel.visible = false
      }
      onCanceled: featurePanel.visible = false
    }

}
