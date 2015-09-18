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

from PyQt4.QtCore import QVariant
from qgis.core import QgsFeature, QgsGeometry, QgsField, QgsFields
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTableField
from processing.core.outputs import OutputVector
from processing.tools import vector, dataobjects


class Dissolve(GeoAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    FIELD = 'FIELD'

    #==========================================================================
    #def getIcon(self):
    #   return QtGui.QIcon(os.path.dirname(__file__) + "/icons/dissolve.png")
    #==========================================================================

    def processAlgorithm(self, progress):
        vlayerA = dataobjects.getObjectFromUri(
            self.getParameterValue(Dissolve.INPUT))
        vproviderA = vlayerA.dataProvider()
        fields = QgsFields()
        fieldname = self.getParameterValue(Dissolve.FIELD)
        try:
            fieldIdx = vlayerA.fieldNameIndex(fieldname)
            field = vlayerA.fields().field(fieldname)
            fields.append(field)
            useField = True
        except:
            useField = False

        countField = QgsField("count", QVariant.Int, '', 10, 0)
        fields.append(countField)
        writer = self.getOutputFromName(
            Dissolve.OUTPUT).getVectorWriter(fields,
                                             vproviderA.geometryType(),
                                             vproviderA.crs())
        outFeat = QgsFeature()
        outFeat.initAttributes(fields.count())
        nElement = 0
        nFeat = vlayerA.selectedFeatureCount()

        if nFeat == 0:
            nFeat = vlayerA.featureCount()

        if not useField:
            first = True
            features = vector.features(vlayerA)
            for inFeat in features:
                nElement += 1
                progress.setPercentage(int(nElement * 100 / nFeat))
                if first:
                    tmpOutGeom = QgsGeometry(inFeat.geometry())
                    first = False
                else:
                    tmpInGeom = QgsGeometry(inFeat.geometry())
                    try:
                        tmpOutGeom = QgsGeometry(tmpOutGeom.combine(tmpInGeom))
                    except:
                        raise GeoAlgorithmExecutionException(
                            self.tr('Geometry exception while dissolving'))
            outFeat.setGeometry(tmpOutGeom)
            outFeat[0] = nElement
            writer.addFeature(outFeat)
        else:
            unique = vector.getUniqueValues(vlayerA, int(fieldIdx))
            nFeat = len(unique)
            myDict = {}
            for item in unique:
                myDict[unicode(item).strip()] = []

            features = vector.features(vlayerA)
            for inFeat in features:
                attrs = inFeat.attributes()
                tempItem = attrs[fieldIdx]
                tmpInGeom = QgsGeometry(inFeat.geometry())

                if len(myDict[unicode(tempItem).strip()]) == 0:
                    myDict[unicode(tempItem).strip()].append(tempItem)

                myDict[unicode(tempItem).strip()].append(tmpInGeom)

            for key, value in myDict.items():
                nElement += 1
                progress.setPercentage(int(nElement * 100 / nFeat))
                for i in range(len(value)):
                    if i == 0:
                        tempItem = value[i]
                        continue
                    else:
                        tmpInGeom = value[i]

                        if i == 1:
                            tmpOutGeom = tmpInGeom
                        else:
                            try:
                                tmpOutGeom = QgsGeometry(
                                    tmpOutGeom.combine(tmpInGeom))
                            except:
                                raise GeoAlgorithmExecutionException(
                                    self.tr('Geometry exception while dissolving'))
                outFeat.setGeometry(tmpOutGeom)
                outFeat[0] = tempItem
                outFeat[1] = i
                writer.addFeature(outFeat)
        del writer

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Dissolve')
        self.group, self.i18n_group = self.trAlgorithm('Vector geometry tools')
        self.addParameter(ParameterVector(Dissolve.INPUT,
                                          self.tr('Input layer'),
                                          [ParameterVector.VECTOR_TYPE_POLYGON, ParameterVector.VECTOR_TYPE_LINE]))
        self.addParameter(ParameterTableField(Dissolve.FIELD,
                                              self.tr('Dissolve field (if not set all features are dissolved)'),
                                              Dissolve.INPUT, optional=True))
        self.addOutput(OutputVector(Dissolve.OUTPUT, self.tr('Dissolved')))
