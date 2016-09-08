# -*- coding: utf-8 -*-

"""
***************************************************************************
    ParametersPanel.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
                           (C) 2013 by CS Systemes d'information (CS SI)
    Email                : volayaf at gmail dot com
                           otb at c-s dot fr (CS SI)
    Contributors         : Victor Olaya

***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import locale

from qgis.core import QgsMapLayerRegistry, QgsMapLayer

from qgis.PyQt import uic
from qgis.PyQt.QtCore import QCoreApplication, QVariant
from qgis.PyQt.QtWidgets import (QWidget, QLayout, QVBoxLayout, QHBoxLayout, QToolButton,                                  
                                 QLabel, QCheckBox, QComboBox, QLineEdit, QPlainTextEdit)
from qgis.PyQt.QtGui import QIcon

from processing.gui.OutputSelectionPanel import OutputSelectionPanel
from processing.gui.PointSelectionPanel import PointSelectionPanel
from processing.gui.GeometryPredicateSelectionPanel import \
    GeometryPredicateSelectionPanel
from processing.gui.ListMultiselectWidget import ListMultiSelectWidget
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTable
from processing.core.parameters import ParameterTableField
from processing.core.parameters import ParameterTableMultipleField
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterFixedTable
from processing.core.parameters import ParameterRange
from processing.core.parameters import ParameterMultipleInput
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterExtent
from processing.core.parameters import ParameterFile
from processing.core.parameters import ParameterCrs
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterPoint
from processing.core.parameters import ParameterGeometryPredicate

from processing.core.outputs import OutputRaster
from processing.core.outputs import OutputTable
from processing.core.outputs import OutputVector

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'widgetParametersPanel.ui'))


class ParametersPanel(BASE, WIDGET):

    NOT_SELECTED = QCoreApplication.translate('ParametersPanel', '[Not selected]')

    def __init__(self, parent, alg):
        super(ParametersPanel, self).__init__(None)
        self.setupUi(self)

        self.grpAdvanced.hide()

        self.layoutMain = self.scrollAreaWidgetContents.layout()
        self.layoutAdvanced = self.grpAdvanced.layout()

        self.parent = parent
        self.alg = alg
        self.valueItems = {}
        self.wrappers = {}
        self.labels = {}
        self.widgets = {}
        self.checkBoxes = {}
        self.dependentItems = {}
        self.iterateButtons = {}

        self.initWidgets()

    def layerRegistryChanged(self, layers):
        for wrapper in self.wrappers.values():
            wrapper.refresh()

    def initWidgets(self):
        # If there are advanced parameters — show corresponding groupbox
        for param in self.alg.parameters:
            if param.isAdvanced:
                self.grpAdvanced.show()
                break
        # Create widgets and put them in layouts
        for param in self.alg.parameters:
            if param.hidden:
                continue

            desc = param.description
            if isinstance(param, ParameterExtent):
                desc += self.tr(' (xmin, xmax, ymin, ymax)')
            if isinstance(param, ParameterPoint):
                desc += self.tr(' (x, y)')
            try:
                if param.optional:
                    desc += self.tr(' [optional]')
            except:
                pass

            wrapper = self.getWidgetWrapperFromParameter(param)
            self.wrappers[param.name] = wrapper
            self.valueItems[param.name] = wrapper.widget
            widget = wrapper.widget

            if isinstance(param, ParameterVector):
                layout = QHBoxLayout()
                layout.setSpacing(2)
                layout.setMargin(0)
                layout.addWidget(widget)
                button = QToolButton()
                icon = QIcon(os.path.join(pluginPath, 'images', 'iterate.png'))
                button.setIcon(icon)
                button.setToolTip(self.tr('Iterate over this layer'))
                button.setCheckable(True)
                layout.addWidget(button)
                self.iterateButtons[param.name] = button
                button.toggled.connect(self.buttonToggled)
                widget = QWidget()
                widget.setLayout(layout)

            print wrapper
            tooltips = self.alg.getParameterDescriptions()
            widget.setToolTip(tooltips.get(param.name, param.description))

            label = QLabel(desc)
            # label.setToolTip(tooltip)
            self.labels[param.name] = label
            if param.isAdvanced:
                self.layoutAdvanced.addWidget(label)
                self.layoutAdvanced.addWidget(widget)
            else:
                self.layoutMain.insertWidget(
                    self.layoutMain.count() - 2, label)
                self.layoutMain.insertWidget(
                    self.layoutMain.count() - 2, widget)

            self.widgets[param.name] = widget

        for output in self.alg.outputs:
            if output.hidden:
                continue

            label = QLabel(output.description)
            widget = OutputSelectionPanel(output, self.alg)
            self.layoutMain.insertWidget(self.layoutMain.count() - 1, label)
            self.layoutMain.insertWidget(self.layoutMain.count() - 1, widget)
            if isinstance(output, (OutputRaster, OutputVector, OutputTable)):
                check = QCheckBox()
                check.setText(self.tr('Open output file after running algorithm'))
                check.setChecked(True)
                self.layoutMain.insertWidget(self.layoutMain.count() - 1, check)
                self.checkBoxes[output.name] = check
            self.valueItems[output.name] = widget

        for wrapper in self.wrappers.values():
            wrapper.postInitialize(self.wrappers.values())

    def buttonToggled(self, value):
        if value:
            sender = self.sender()
            for button in self.iterateButtons.values():
                if button is not sender:
                    button.setChecked(False)

    def getWidgetWrapperFromParameter(self, param):
        wrapper = param.wrapper(self.parent)
        wrapper.widgetValueHasChanged.connect(self.widgetValueHasChanged)
        return wrapper
    
    def widgetValueHasChanged(self, wrapper):
        for wrapper in self.wrappers.values():
            wrapper.anotherParameterWidgetHasChanged(wrapper)
