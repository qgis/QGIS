# -*- coding: utf-8 -*-

"""
***************************************************************************
    wrappers_map_theme.py - Map theme widget wrappers
    ---------------------
    Date                 : August 2017
    Copyright            : (C) 2017 by OPENGIS.ch
    Email                : mario@opengis.ch
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""


from qgis.core import (QgsSettings,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterFile,
                       QgsProcessingParameterField,
                       QgsProcessingParameterExpression,
                       QgsProcessingOutputString,
                       QgsProcessingParameterString,
                       QgsMapThemeCollection,
                       QgsProject
                       )

from qgis.PyQt.QtWidgets import QComboBox
from qgis.PyQt.QtCore import pyqtSignal

from processing.gui.wrappers import (
    BasicWidgetWrapper
)
from processing.tools.postgis import GeoDB


class MapThemeWrapper(BasicWidgetWrapper):
    """
    WidgetWrapper for ParameterString that create and manage a combobox widget
    with existing postgis connections.
    """

    def createWidget(self):
        self._combo = QComboBox()
        for item in self.items():
            self._combo.addItem(item, item)
        self._combo.currentIndexChanged.connect(lambda:
                                                self.widgetValueHasChanged.emit(self))
        return self._combo

    def items(self):
        items = QgsProject.instance().mapThemeCollection().mapThemes()
        #if self.dialogType == DIALOG_MODELER:
        #    strings = self.dialog.getAvailableValuesOfType(
        #        [QgsProcessingParameterString, QgsProcessingParameterNumber,
        #  QgsProcessingParameterFile,
        #         QgsProcessingParameterField,
        # QgsProcessingParameterExpression], QgsProcessingOutputString)
        #    items = items + [(self.dialog.resolveValueDescription(s),
        # s) for s in strings]
        #
        return items

    def setValue(self, value):
        self.setComboValue(value, self._combo)

    def value(self):
        return self.comboValue(combobox=self._combo)
