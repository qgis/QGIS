# -*- coding: utf-8 -*-

"""
***************************************************************************
    Gridify.py
    ---------------------
    Date                 : May 2010
    Copyright            : (C) 2010 by Michael Minn
    Email                : pyqgis at michaelminn dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Michael Minn'
__date__ = 'May 2010'
__copyright__ = '(C) 2010, Michael Minn'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QVariant
from qgis.core import QgsFields, QgsVectorLayer

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterMultipleInput
from processing.core.outputs import OutputVector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class Merge(GeoAlgorithm):
    LAYERS = 'LAYERS'
    OUTPUT = 'OUTPUT'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'merge_shapes.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Merge vector layers')
        self.group, self.i18n_group = self.trAlgorithm('Vector general tools')

        self.addParameter(ParameterMultipleInput(self.LAYERS,
                                                 self.tr('Layers to merge'), datatype=ParameterMultipleInput.TYPE_VECTOR_ANY))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Merged')))

    def processAlgorithm(self, progress):
        inLayers = self.getParameterValue(self.LAYERS)
        paths = inLayers.split(';')

        layers = []
        fields = QgsFields()
        totalFeatureCount = 0
        for x in xrange(len(paths)):
            layer = QgsVectorLayer(paths[x], unicode(x), 'ogr')

            if (len(layers) > 0):
                if (layer.dataProvider().geometryType() != layers[0].dataProvider().geometryType()):
                    raise GeoAlgorithmExecutionException(
                        self.tr('All layers must have same geometry type!'))

            layers.append(layer)
            totalFeatureCount += layer.featureCount()

            for sindex, sfield in enumerate(layer.dataProvider().fields()):
                found = None
                for dfield in fields:
                    if (dfield.name().upper() == sfield.name().upper()):
                        found = dfield
                        if (dfield.type() != sfield.type()):
                            raise GeoAlgorithmExecutionException(
                                self.tr('{} field in layer {} has different '
                                        'data type than in other layers.'))

                if not found:
                    fields.append(sfield)

        total = 100.0 / totalFeatureCount
        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            fields.toList(), layers[0].dataProvider().geometryType(),
            layers[0].crs())

        featureCount = 0
        for layer in layers:
            for feature in layer.dataProvider().getFeatures():
                sattributes = feature.attributes()
                dattributes = []
                for dindex, dfield in enumerate(fields):
                    if (dfield.type() == QVariant.Int, QVariant.UInt, QVariant.LongLong, QVariant.ULongLong):
                        dattribute = 0
                    elif (dfield.type() == QVariant.Double):
                        dattribute = 0.0
                    else:
                        dattribute = ''

                    for sindex, sfield in enumerate(layer.dataProvider().fields()):
                        if (sfield.name().upper() == dfield.name().upper()):
                            if (sfield.type() != dfield.type()):
                                raise GeoAlgorithmExecutionException(
                                    self.tr('Attribute type mismatch'))
                            dattribute = sattributes[sindex]
                            break

                    dattributes.append(dattribute)

                feature.setAttributes(dattributes)
                writer.addFeature(feature)
                featureCount += 1
                progress.setPercentage(int(featureCount * total))

        del writer
