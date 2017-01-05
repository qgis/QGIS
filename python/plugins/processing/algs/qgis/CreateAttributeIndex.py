# -*- coding: utf-8 -*-

"""
***************************************************************************
    CreateAttributeIndex.py
    -----------------------
    Date                 : November 2016
    Copyright            : (C) 2016 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Nyall Dawson'
__date__ = 'November 2016'
__copyright__ = '(C) 2016, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import QgsVectorDataProvider, QgsFields

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterTable
from processing.core.parameters import ParameterTableField
from processing.core.outputs import OutputVector

from processing.tools import dataobjects


class CreateAttributeIndex(GeoAlgorithm):

    INPUT = 'INPUT'
    FIELD = 'FIELD'
    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Create attribute index')
        self.group, self.i18n_group = self.trAlgorithm('Vector general tools')

        self.addParameter(ParameterTable(self.INPUT,
                                         self.tr('Input Layer')))
        self.addParameter(ParameterTableField(self.FIELD,
                                              self.tr('Attribute to index'), self.INPUT))
        self.addOutput(OutputVector(self.OUTPUT,
                                    self.tr('Indexed layer'), True))

    def processAlgorithm(self, progress):
        file_name = self.getParameterValue(self.INPUT)
        layer = dataobjects.getObjectFromUri(file_name)
        field = self.getParameterValue(self.FIELD)
        provider = layer.dataProvider()

        field_index = layer.fields().lookupField(field)
        if field_index < 0 or layer.fields().fieldOrigin(field_index) != QgsFields.OriginProvider:
            progress.setInfo(self.tr('Can not create attribute index on "{}"').format(field))
        else:
            provider_index = layer.fields().fieldOriginIndex(field_index)
            if provider.capabilities() & QgsVectorDataProvider.CreateAttributeIndex:
                if not provider.createAttributeIndex(provider_index):
                    progress.setInfo(self.tr('Could not create attribute index'))
            else:
                progress.setInfo(self.tr("Layer's data provider does not support "
                                         "creating attribute indexes"))

        self.setOutputValue(self.OUTPUT, file_name)
