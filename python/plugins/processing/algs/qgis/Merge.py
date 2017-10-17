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
from qgis.core import (QgsFields,
                       QgsField,
                       QgsFeatureRequest,
                       QgsFeatureSink,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterMultipleLayers,
                       QgsProcessingParameterFeatureSink,
                       QgsMapLayer)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class Merge(QgisAlgorithm):
    LAYERS = 'LAYERS'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'merge_shapes.png'))

    def group(self):
        return self.tr('Vector general')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterMultipleLayers(self.LAYERS,
                                                               self.tr('Layers to merge'),
                                                               QgsProcessing.TypeVectorAnyGeometry))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Merged')))

    def name(self):
        return 'mergevectorlayers'

    def displayName(self):
        return self.tr('Merge vector layers')

    def processAlgorithm(self, parameters, context, feedback):
        input_layers = self.parameterAsLayerList(parameters, self.LAYERS, context)

        layers = []
        fields = QgsFields()
        totalFeatureCount = 0
        for layer in input_layers:
            if layer.type() != QgsMapLayer.VectorLayer:
                raise QgsProcessingException(
                    self.tr('All layers must be vector layers!'))

            if (len(layers) > 0):
                if (layer.wkbType() != layers[0].wkbType()):
                    raise QgsProcessingException(
                        self.tr('All layers must have same geometry type!'))

            layers.append(layer)
            totalFeatureCount += layer.featureCount()

            for sindex, sfield in enumerate(layer.fields()):
                found = None
                for dfield in fields:
                    if (dfield.name().upper() == sfield.name().upper()):
                        found = dfield
                        if (dfield.type() != sfield.type()):
                            raise QgsProcessingException(
                                self.tr('{} field in layer {} has different '
                                        'data type than in other layers.'.format(sfield.name(), layerSource)))

                if not found:
                    fields.append(sfield)

        add_layer_field = False
        if fields.lookupField('layer') < 0:
            fields.append(QgsField('layer', QVariant.String, '', 100))
            add_layer_field = True
        add_path_field = False
        if fields.lookupField('path') < 0:
            fields.append(QgsField('path', QVariant.String, '', 200))
            add_path_field = True

        total = 100.0 / totalFeatureCount if totalFeatureCount else 1
        dest_crs = layers[0].crs()
        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, layers[0].wkbType(), dest_crs)

        featureCount = 0
        for layer in layers:
            for feature in layer.getFeatures(QgsFeatureRequest().setDestinationCrs(dest_crs)):
                if feedback.isCanceled():
                    break

                sattributes = feature.attributes()
                dattributes = []
                for dindex, dfield in enumerate(fields):
                    if add_layer_field and dfield.name() == 'layer':
                        dattributes.append(layer.name())
                        continue
                    if add_path_field and dfield.name() == 'path':
                        dattributes.append(layer.publicSource())
                        continue

                    if (dfield.type() == QVariant.Int, QVariant.UInt, QVariant.LongLong, QVariant.ULongLong):
                        dattribute = 0
                    elif (dfield.type() == QVariant.Double):
                        dattribute = 0.0
                    else:
                        dattribute = ''

                    for sindex, sfield in enumerate(layer.fields()):
                        if (sfield.name().upper() == dfield.name().upper()):
                            if (sfield.type() != dfield.type()):
                                raise QgsProcessingException(
                                    self.tr('Attribute type mismatch'))
                            dattribute = sattributes[sindex]
                            break

                    dattributes.append(dattribute)

                feature.setAttributes(dattributes)
                sink.addFeature(feature, QgsFeatureSink.FastInsert)
                featureCount += 1
                feedback.setProgress(int(featureCount * total))

        return {self.OUTPUT: dest_id}
