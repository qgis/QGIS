# -*- coding: utf-8 -*-

"""
***************************************************************************
    Dissolve.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
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
from collections import defaultdict

from qgis.PyQt.QtGui import QIcon

from qgis.core import QgsFeature, QgsGeometry

from processing.core.ProcessingLog import ProcessingLog
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterTableMultipleField
from processing.core.outputs import OutputVector
from processing.tools import vector, dataobjects

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class Dissolve(GeoAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    FIELD = 'FIELD'
    DISSOLVE_ALL = 'DISSOLVE_ALL'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'dissolve.png'))

    def defineCharacteristics(self):
        self.name = 'Dissolve'
        self.group = 'Vector geometry tools'
        self.addParameter(ParameterVector(Dissolve.INPUT,
                                          self.tr('Input layer'),
                                          [dataobjects.TYPE_VECTOR_POLYGON, dataobjects.TYPE_VECTOR_LINE]))
        self.addParameter(ParameterBoolean(Dissolve.DISSOLVE_ALL,
                                           self.tr('Dissolve all (do not use fields)'), True))
        self.addParameter(ParameterTableMultipleField(Dissolve.FIELD,
                                                      self.tr('Unique ID fields'), Dissolve.INPUT, optional=True))
        self.addOutput(OutputVector(Dissolve.OUTPUT, self.tr('Dissolved')))

    def processAlgorithm(self, progress):
        useField = not self.getParameterValue(Dissolve.DISSOLVE_ALL)
        field_names = self.getParameterValue(Dissolve.FIELD)
        vlayerA = dataobjects.getObjectFromUri(
            self.getParameterValue(Dissolve.INPUT))

        writer = self.getOutputFromName(
            Dissolve.OUTPUT).getVectorWriter(
                vlayerA.fields().toList(),
                vlayerA.wkbType(),
                vlayerA.crs())

        outFeat = QgsFeature()
        features = vector.features(vlayerA)
        total = 100.0 / len(features)

        if not useField:
            first = True
            for current, inFeat in enumerate(features):
                progress.setPercentage(int(current * total))
                if first:
                    attrs = inFeat.attributes()
                    tmpInGeom = inFeat.geometry()
                    if tmpInGeom.isEmpty() or tmpInGeom.isGeosEmpty():
                        continue
                    errors = tmpInGeom.validateGeometry()
                    if len(errors) != 0:
                        for error in errors:
                            ProcessingLog.addToLog(ProcessingLog.LOG_ERROR,
                                                   self.tr('ValidateGeometry()'
                                                           'error: One or more '
                                                           'input features have '
                                                           'invalid geometry: ')
                                                   + error.what())
                        continue
                    outFeat.setGeometry(tmpInGeom)
                    first = False
                else:
                    tmpInGeom = inFeat.geometry()
                    if tmpInGeom.isEmpty() or tmpInGeom.isGeosEmpty():
                        continue
                    tmpOutGeom = outFeat.geometry()
                    errors = tmpInGeom.validateGeometry()
                    if len(errors) != 0:
                        for error in errors:
                            ProcessingLog.addToLog(ProcessingLog.LOG_ERROR,
                                                   self.tr('ValidateGeometry()'
                                                           'error:One or more input'
                                                           'features have invalid '
                                                           'geometry: ')
                                                   + error.what())
                        continue
                    try:
                        tmpOutGeom = QgsGeometry(tmpOutGeom.combine(tmpInGeom))
                        outFeat.setGeometry(tmpOutGeom)
                    except:
                        raise GeoAlgorithmExecutionException(
                            self.tr('Geometry exception while dissolving'))
            outFeat.setAttributes(attrs)
            writer.addFeature(outFeat)
        else:
            field_indexes = [vlayerA.fields().lookupField(f) for f in field_names.split(';')]

            attribute_dict = {}
            geometry_dict = defaultdict(lambda: [])

            for inFeat in features:
                attrs = inFeat.attributes()

                index_attrs = tuple([attrs[i] for i in field_indexes])

                tmpInGeom = QgsGeometry(inFeat.geometry())
                if tmpInGeom.isGeosEmpty():
                    continue
                errors = tmpInGeom.validateGeometry()
                if len(errors) != 0:
                    for error in errors:
                        ProcessingLog.addToLog(ProcessingLog.LOG_ERROR,
                                               self.tr('ValidateGeometry() '
                                                       'error: One or more input'
                                                       'features have invalid '
                                                       'geometry: ')
                                               + error.what())

                if not index_attrs in attribute_dict:
                    # keep attributes of first feature
                    attribute_dict[index_attrs] = attrs

                geometry_dict[index_attrs].append(tmpInGeom)

            nFeat = len(attribute_dict)

            nElement = 0
            for key, value in list(geometry_dict.items()):
                outFeat = QgsFeature()
                nElement += 1
                progress.setPercentage(int(nElement * 100 / nFeat))
                try:
                    tmpOutGeom = QgsGeometry.unaryUnion(value)
                except:
                    raise GeoAlgorithmExecutionException(
                        self.tr('Geometry exception while dissolving'))
                outFeat.setGeometry(tmpOutGeom)
                outFeat.setAttributes(attribute_dict[key])
                writer.addFeature(outFeat)

        del writer
