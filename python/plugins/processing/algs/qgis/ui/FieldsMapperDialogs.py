# -*- coding: utf-8 -*-

"""
***************************************************************************
    FieldsMapper.py
    ---------------------
    Date                 : October 2014
    Copyright            : (C) 2014 by Arnaud Morvan
    Email                : arnaud dot morvan at camptocamp dot com
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
__date__ = 'October 2014'
__copyright__ = '(C) 2014, Arnaud Morvan'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


from PyQt4.QtGui import QComboBox, QSpacerItem

from processing.core.parameters import ParameterVector
from processing.tools import dataobjects
from processing.gui.ParametersPanel import ParametersPanel
from processing.gui.AlgorithmDialog import AlgorithmDialog, AlgorithmDialogBase
from processing.modeler.ModelerParametersDialog import ModelerParametersDialog

from processing.algs.qgis.fieldsmapping import ParameterFieldsMapping
from .FieldsMappingPanel import FieldsMappingPanel


class FieldsMapperParametersPanel(ParametersPanel):

    def __init__(self, parent, alg):
        ParametersPanel.__init__(self, parent, alg)

        item = self.layoutMain.itemAt(self.layoutMain.count() - 1)
        if isinstance(item, QSpacerItem):
            self.layoutMain.removeItem(item)
            item = None

    def getWidgetFromParameter(self, param):
        if isinstance(param, ParameterFieldsMapping):
            item = FieldsMappingPanel()
            if param.parent in self.dependentItems:
                items = self.dependentItems[param.parent]
            else:
                items = []
                self.dependentItems[param.parent] = items
            items.append(param.name)
            parent = self.alg.getParameterFromName(param.parent)
            if isinstance(parent, ParameterVector):
                layers = dataobjects.getVectorLayers(parent.shapetype)
            else:
                layers = dataobjects.getTables()
            if len(layers) > 0:
                item.setLayer(layers[0])
            return item
        return ParametersPanel.getWidgetFromParameter(self, param)

    def updateDependentFields(self):
        sender = self.sender()
        if not isinstance(sender, QComboBox):
            return
        if sender.name not in self.dependentItems:
            return
        layer = sender.itemData(sender.currentIndex())
        children = self.dependentItems[sender.name]
        for child in children:
            widget = self.valueItems[child]
            if isinstance(widget, FieldsMappingPanel):
                widget.setLayer(layer)

    def somethingDependsOnThisParameter(self, parent):
        for param in self.alg.parameters:
            if isinstance(param, ParameterFieldsMapping):
                if param.parent == parent.name:
                    return True
        return False


class FieldsMapperParametersDialog(AlgorithmDialog):
    def __init__(self, alg):
        AlgorithmDialogBase.__init__(self, alg)

        self.alg = alg

        self.mainWidget = FieldsMapperParametersPanel(self, alg)
        self.setMainWidget()

    def setParamValue(self, param, widget, alg=None):
        if isinstance(param, ParameterFieldsMapping):
            return param.setValue(widget.value())
        return AlgorithmDialog.setParamValue(self, param, widget, alg)


class FieldsMapperModelerParametersDialog(ModelerParametersDialog):

    def __init__(self, alg, model, algName=None):
        ModelerParametersDialog.__init__(self, alg, model, algName)

        paramsLayout = self.paramPanel.layout()
        item = paramsLayout.itemAt(paramsLayout.count() - 1)
        if isinstance(item, QSpacerItem):
            paramsLayout.removeItem(item)
            item = None

    def getWidgetFromParameter(self, param):
        if isinstance(param, ParameterFieldsMapping):
            return FieldsMappingPanel()
        return ModelerParametersDialog.getWidgetFromParameter(self, param)

    def setPreviousValues(self):
        ModelerParametersDialog.setPreviousValues(self)
        if self._algName is not None:
            alg = self.model.algs[self._algName]
            for param in alg.algorithm.parameters:
                if isinstance(param, ParameterFieldsMapping):
                    widget = self.valueItems[param.name]
                    value = alg.params[param.name]
                    if isinstance(value, unicode):
                        # convert to list because of ModelerAlgorithme.resolveValue behavior with lists
                        value = eval(value)
                    widget.setValue(value)

    def setParamValue(self, alg, param, widget):
        if isinstance(param, ParameterFieldsMapping):
            # convert to unicode because of ModelerAlgorithme.resolveValue behavior with lists
            alg.params[param.name] = unicode(widget.value())
            return True
        return ModelerParametersDialog.setParamValue(self, alg, param, widget)
