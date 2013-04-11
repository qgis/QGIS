# -*- coding: utf-8 -*-

"""
***************************************************************************
    FieldPyculator.py
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

import sys

from PyQt4.QtCore import *

from qgis.core import *

from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.core.QGisLayers import QGisLayers
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException

from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterString import ParameterString
from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.parameters.ParameterSelection import ParameterSelection

from sextante.outputs.OutputVector import OutputVector


class FieldsPyculator(GeoAlgorithm):

    INPUT_LAYER = "INPUT_LAYER"
    FIELD_NAME = "FIELD_NAME"
    FIELD_TYPE = "FIELD_TYPE"
    FIELD_LENGTH = "FIELD_LENGTH"
    FIELD_PRECISION = "FIELD_PRECISION"
    GLOBAL = "GLOBAL"
    FORMULA = "FORMULA"
    OUTPUT_LAYER ="OUTPUT_LAYER"
    RESULT_VAR_NAME = "value"

    TYPE_NAMES = ["Integer", "Float", "String"]
    TYPES = [QVariant.Int, QVariant.Double, QVariant.String]


    #===========================================================================
    # def getIcon(self):
    #    return QtGui.QIcon(os.path.dirname(__file__) + "/../images/qgis.png")
    #===========================================================================

    def defineCharacteristics(self):
        self.name = "Advanced Python field calculator"
        self.group = "Vector table tools"
        self.addParameter(ParameterVector(self.INPUT_LAYER, "Input layer", ParameterVector.VECTOR_TYPE_ANY, False))
        self.addParameter(ParameterString(self.FIELD_NAME, "Result field name", "NewField"))
        self.addParameter(ParameterSelection(self.FIELD_TYPE, "Field type", self.TYPE_NAMES))
        self.addParameter(ParameterNumber(self.FIELD_LENGTH, "Field lenght", 1, 255, 10))
        self.addParameter(ParameterNumber(self.FIELD_PRECISION, "Field precision", 0, 10, 0))
        self.addParameter(ParameterString(self.GLOBAL, "Global expression", multiline = True))
        self.addParameter(ParameterString(self.FORMULA, "Formula", "value = ", multiline = True))
        self.addOutput(OutputVector(self.OUTPUT_LAYER, "Output layer"))

    def processAlgorithm(self, progress):
        fieldName = self.getParameterValue(self.FIELD_NAME)
        fieldType = self.getParameterValue(self.FIELD_TYPE)
        fieldLength = self.getParameterValue(self.FIELD_LENGTH)
        fieldPrecision = self.getParameterValue(self.FIELD_PRECISION)
        code = self.getParameterValue(self.FORMULA)
        globalExpression = self.getParameterValue(self.GLOBAL)
        output = self.getOutputFromName(self.OUTPUT_LAYER)

        layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.INPUT_LAYER))
        provider = layer.dataProvider()
        fields = provider.fields()
        fields.append(QgsField(fieldName, self.TYPES[fieldType], "", fieldLength, fieldPrecision))
        writer = output.getVectorWriter(fields, provider.geometryType(), layer.crs())
        outFeat = QgsFeature()
        new_ns = {}

        #run global code
        if globalExpression.strip() != "":
            try:
                bytecode = compile(globalExpression, '<string>', 'exec')
                exec bytecode in new_ns
            except:
                raise GeoAlgorithmExecutionException("FieldPyculator code execute error\n" +
                            "Global code block can't be executed!%s \n %s" %
                            (unicode(sys.exc_info()[0].__name__), unicode(sys.exc_info()[1])))

        #replace all fields tags
        fields = provider.fields()
        num = 0
        for field in fields:
            field_name = unicode(field.name())
            replval = '__attr[' + str(num) + ']'
            code = code.replace("<" + field_name + ">", replval)
            num += 1

        #replace all special vars
        code = code.replace('$id', '__id')
        code = code.replace('$geom', '__geom')
        need_id = code.find("__id") != -1
        need_geom = code.find("__geom") != -1
        need_attrs = code.find("__attr") != -1

        #compile
        try:
            bytecode = compile(code, '<string>', 'exec')
        except:
            raise GeoAlgorithmExecutionException("FieldPyculator code execute error\n"+
                                 "Field code block can't be executed! %s \n %s"
                                 (unicode(sys.exc_info()[0].__name__), unicode(sys.exc_info()[1])))

        #run
        features = QGisLayers.features(layer)
        nFeatures = len(features)
        nElement = 1
        for feat in features:
            progress.setPercentage(int((100 * nElement)/nFeatures))
            attrs = feat.attributes()
            feat_id = feat.id()

            #add needed vars
            if need_id:
                new_ns['__id'] = feat_id

            if need_geom:
                geom = feat.geometry()
                new_ns['__geom'] = geom

            if need_attrs:
                pyattrs = [self.Qvar2py(a) for a in attrs]
                new_ns['__attr'] = pyattrs

            #clear old result
            if new_ns.has_key(self.RESULT_VAR_NAME):
                del new_ns[self.RESULT_VAR_NAME]

            #exec
            #try:
            exec bytecode in new_ns
            #except:
            #    raise e
            #===============================================================
            # GeoAlgorithmExecutionException("FieldPyculator code execute error\n"+
            #            "Field code block can't be executed for feature %s\n%s\n%s" %
            #            (unicode(sys.exc_info()[0].__name__),
            #            unicode(sys.exc_info()[1]),
            #            unicode(feat_id)))
            #===============================================================

            #check result
            if not new_ns.has_key(self.RESULT_VAR_NAME):
                raise GeoAlgorithmExecutionException("FieldPyculator code execute error\n" +
                        "Field code block does not return '%s1' variable! Please declare this variable in your code!" %
                        self.RESULT_VAR_NAME)

            #write feature
            nElement += 1
            outFeat.setGeometry( feat.geometry() )
            attrs.append(QVariant(new_ns[self.RESULT_VAR_NAME]))
            outFeat.setAttributes(attrs)
            writer.addFeature(outFeat)

        del writer

    def Qvar2py(self,qv):
        if qv.type() == 2:
            return qv.toInt()[0]
        if qv.type() == 10:
            return unicode(qv.toString())
        if qv.type() == 6:
            return qv.toDouble()[0]
        return None

    def checkParameterValuesBeforeExecuting(self):
        ##TODO check that formula is correct and fields exist
        pass
