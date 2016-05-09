# -*- coding: utf-8 -*-

"""
***************************************************************************
    PredicatePanel.py
    ---------------------
    Date                 : January 2015
    Copyright            : (C) 2015 by Arnaud Morvan
    Email                : arnaud dot morvan at camptocamp dot com
    Contributors         : Arnaud Morvan
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Arnaud Morvan'
__date__ = 'January 2015'
__copyright__ = '(C) 2015, Arnaud Morvan'

# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os

from qgis.PyQt import uic
from qgis.PyQt.QtWidgets import QCheckBox
from qgis.core import QGis, QgsVectorLayer

from processing.core.parameters import ParameterGeometryPredicate

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'widgetGeometryPredicateSelector.ui'))


class GeometryPredicateSelectionPanel(BASE, WIDGET):

    unusablePredicates = {
        QGis.Point: {
            QGis.Point: ('touches', 'crosses'),
            QGis.Line: ('equals', 'contains', 'overlaps'),
            QGis.Polygon: ('equals', 'contains', 'overlaps')
        },
        QGis.Line: {
            QGis.Point: ('equals', 'within', 'overlaps'),
            QGis.Line: [],
            QGis.Polygon: ('equals', 'contains', 'overlaps')
        },
        QGis.Polygon: {
            QGis.Point: ('equals', 'within', 'overlaps'),
            QGis.Line: ('equals', 'within', 'overlaps'),
            QGis.Polygon: ('crosses')
        }
    }

    def __init__(self,
                 enabledPredicated=ParameterGeometryPredicate.predicates,
                 rows=4):
        super(GeometryPredicateSelectionPanel, self).__init__(None)
        self.setupUi(self)

        self.enabledPredicated = enabledPredicated
        self.leftLayer = None
        self.rightLayer = None
        self.setRows(rows)
        self.updatePredicates()

    def onLeftLayerChange(self):
        sender = self.sender()
        self.leftLayer = sender.itemData(sender.currentIndex())
        self.updatePredicates()

    def onRightLayerChange(self):
        sender = self.sender()
        self.rightLayer = sender.itemData(sender.currentIndex())
        self.updatePredicates()

    def updatePredicates(self):
        if (isinstance(self.leftLayer, QgsVectorLayer)
                and isinstance(self.rightLayer, QgsVectorLayer)):
            leftType = self.leftLayer.geometryType()
            rightType = self.rightLayer.geometryType()
            unusablePredicates = self.unusablePredicates[leftType][rightType]
        else:
            unusablePredicates = []
        for predicate in ParameterGeometryPredicate.predicates:
            widget = self.getWidget(predicate)
            widget.setEnabled(predicate in self.enabledPredicated
                              and predicate not in unusablePredicates)

    def setRows(self, rows):
        widgets = []
        for predicate in ParameterGeometryPredicate.predicates:
            widget = self.getWidget(predicate)
            self.gridLayout.removeWidget(widget)
            widgets.append(widget)
        for i in xrange(0, len(widgets)):
            widget = widgets[i]
            self.gridLayout.addWidget(widget, i % rows, i / rows)

    def getWidget(self, predicate):
        return self.findChild(QCheckBox, predicate + 'Box')

    def value(self):
        values = []
        for predicate in ParameterGeometryPredicate.predicates:
            widget = self.getWidget(predicate)
            if widget.isEnabled() and widget.isChecked():
                values.append(predicate)
        return values

    def setValue(self, values):
        if values:
            for predicate in ParameterGeometryPredicate.predicates:
                widget = self.getWidget(predicate)
                widget.setChecked(predicate in values)
        return True
