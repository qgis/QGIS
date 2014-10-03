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

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import \
        GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterTableField
from processing.core.outputs import OutputVector
from processing.tools import vector, dataobjects


class Dissolve(GeoAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    FIELD = 'FIELD'
    DISSOLVE_ALL = 'DISSOLVE_ALL'

    #==========================================================================
    #def getIcon(self):
    #   return QtGui.QIcon(os.path.dirname(__file__) + "/icons/dissolve.png")
    #==========================================================================

    def processAlgorithm(self, progress):
        useField = not self.getParameterValue(Dissolve.DISSOLVE_ALL)
        fieldname = self.getParameterValue(Dissolve.FIELD)
        vlayerA = dataobjects.getObjectFromUri(
                self.getParameterValue(Dissolve.INPUT))
        field = vlayerA.fieldNameIndex(fieldname)
        vproviderA = vlayerA.dataProvider()
        fields = vproviderA.fields()
        writer = self.getOutputFromName(
                Dissolve.OUTPUT).getVectorWriter(fields,
                                                 vproviderA.geometryType(),
                                                 vproviderA.crs())
        outFeat = QgsFeature()
        nElement = 0
        nFeat = vproviderA.featureCount()
        if not useField:
            first = True
            features = vector.features(vlayerA)
            for inFeat in features:
                nElement += 1
                progress.setPercentage(int(nElement / nFeat * 100))
                if first:
                    attrs = inFeat.attributes()
                    tmpInGeom = QgsGeometry(inFeat.geometry())
                    outFeat.setGeometry(tmpInGeom)
                    first = False
                else:
                    tmpInGeom = QgsGeometry(inFeat.geometry())
                    tmpOutGeom = QgsGeometry(outFeat.geometry())
                    try:
                        tmpOutGeom = QgsGeometry(tmpOutGeom.combine(tmpInGeom))
                        outFeat.setGeometry(tmpOutGeom)
                    except:
                        raise GeoAlgorithmExecutionException(
                                'Geometry exception while dissolving')
            outFeat.setAttributes(attrs)
            writer.addFeature(outFeat)
        else:
            unique = vector.getUniqueValues(vlayerA, int(field))
            nFeat = nFeat * len(unique)
            for item in unique:
                first = True
                add = True
                features = vector.features(vlayerA)
                for inFeat in features:
                    nElement += 1
                    progress.setPercentage(int(nElement / nFeat * 100))
                    atMap = inFeat.attributes()
                    tempItem = atMap[field]
                    if unicode(tempItem).strip() == unicode(item).strip():
                        if first:
                            QgsGeometry(inFeat.geometry())
                            tmpInGeom = QgsGeometry(inFeat.geometry())
                            outFeat.setGeometry(tmpInGeom)
                            first = False
                            attrs = inFeat.attributes()
                        else:
                            tmpInGeom = QgsGeometry(inFeat.geometry())
                            tmpOutGeom = QgsGeometry(outFeat.geometry())
                            try:
                                tmpOutGeom = QgsGeometry(
                                        tmpOutGeom.combine(tmpInGeom))
                                outFeat.setGeometry(tmpOutGeom)
                            except:
                                raise GeoAlgorithmExecutionException(
                                        'Geometry exception while dissolving')
                if add:
                    outFeat.setAttributes(attrs)
                    writer.addFeature(outFeat)
        del writer

    def defineCharacteristics(self):
        self.name = 'Dissolve'
        self.group = 'Vector geometry tools'
        self.addParameter(ParameterVector(Dissolve.INPUT, 'Input layer',
                          [ParameterVector.VECTOR_TYPE_POLYGON,
                          ParameterVector.VECTOR_TYPE_LINE]))
        self.addParameter(ParameterBoolean(Dissolve.DISSOLVE_ALL,
                          'Dissolve all (do not use field)', True))
        self.addParameter(ParameterTableField(Dissolve.FIELD, 'Unique ID field'
                          , Dissolve.INPUT, optional=True))
        self.addOutput(OutputVector(Dissolve.OUTPUT, 'Dissolved'))
