from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.outputs.OutputVector import OutputVector
from sextante.parameters.ParameterVector import ParameterVector
from qgis.core import *
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.parameters.ParameterString import ParameterString
from sextante.core.QGisLayers import QGisLayers
import os
from PyQt4 import QtGui
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from sextante.parameters.ParameterBoolean import ParameterBoolean
import sys


class FieldsPyculator(GeoAlgorithm):

    INPUT_LAYER = "INPUT_LAYER"
    USE_SELECTED = "USE_SELECTED"
    FIELD_NAME = "FIELD_NAME"
    GLOBAL = "GLOBAL"
    FORMULA = "FORMULA"
    OUTPUT_LAYER ="OUTPUT_LAYER"
    RESULT_VAR_NAME = "value"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/toolbox.png")

    def defineCharacteristics(self):
        self.name = "Field Pyculator"
        self.group = "Algorithms for vector layers"
        self.addParameter(ParameterVector(self.INPUT_LAYER, "Input layer", ParameterVector.VECTOR_TYPE_ANY, False))
        self.addParameter(ParameterBoolean(self.USE_SELECTED, "Use only selected features", False))
        self.addParameter(ParameterString(self.FIELD_NAME, "Result field name", "NewField"))
        self.addParameter(ParameterString(self.GLOBAL, "Global expression", multiline = True))
        self.addParameter(ParameterString(self.FORMULA, "Formula", "value = ", multiline = True))
        self.addOutput(OutputVector(self.OUTPUT_LAYER, "Output layer"))


    def processAlgorithm(self, progress):
        fieldname = self.getParameterValue(self.FIELD_NAME)
        code = self.getParameterValue(self.FORMULA)
        globalExpression = self.getParameterValue(self.GLOBAL)
        useSelected = self.getParameterValue(self.USE_SELECTED)
        settings = QSettings()
        systemEncoding = settings.value( "/UI/encoding", "System" ).toString()
        output = self.getOutputFromName(self.OUTPUT_LAYER)
        layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.INPUT_LAYER))
        vprovider = layer.dataProvider()
        allAttrs = vprovider.attributeIndexes()
        vprovider.select( allAttrs )
        fields = vprovider.fields()
        fields[len(fields)] = QgsField(fieldname, QVariant.Double)
        writer = output.getVectorWriter(fields, vprovider.geometryType(), vprovider.crs() )
        outFeat = QgsFeature()
        nFeatures = vprovider.featureCount()
        nElement = 0
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
        field_map = vprovider.fields()
        for num, field in field_map.iteritems():
            field_name = unicode(field.name())
            replval = '__attr[' + str(num) + ']'
            code = code.replace("<"+field_name+">",replval)

        #replace all special vars
        code = code.replace('$id','__id')
        code = code.replace('$geom','__geom')
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
        if not useSelected:
            feat = QgsFeature()
            if need_attrs:
                attr_ind = vprovider.attributeIndexes()
            else:
                attr_ind = []
            vprovider.select(attr_ind, QgsRectangle(), True)

            while vprovider.nextFeature(feat):
                progress.setPercentage(int((100 * nElement)/nFeatures))
                attrMap = feat.attributeMap()
                feat_id = feat.id()

                #add needed vars
                if need_id:
                    new_ns['__id'] = feat_id

                if need_geom:
                    geom = feat.geometry()
                    new_ns['__geom'] = geom

                if need_attrs:
                    attr = []
                    for num,a in attrMap.iteritems():
                        attr.append(self.Qvar2py(a))
                    new_ns['__attr'] = attr

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
                outFeat.setAttributeMap( attrMap )
                outFeat.addAttribute(len(vprovider.fields()), QVariant(new_ns[self.RESULT_VAR_NAME]))
                writer.addFeature(outFeat)

        else:
            features = layer.selectedFeatures()
            nFeatures = len(features)
            for feat in features:
                progress.setPercentage(int((100 * nElement)/nFeatures))
                attrMap = feat.attributeMap()
                feat_id = feat.id()

                #add needed vars
                if need_id:
                    new_ns['__id'] = feat_id

                if need_geom:
                    geom = feat.geometry()
                    new_ns['__geom'] = geom

                if need_attrs:
                    attrMap = feat.attributeMap()
                    attr = []
                    for num,a in attrMap.iteritems():
                        attr.append(self.Qvar2py(a))
                    new_ns['__attr'] = attr

                #clear old result
                if new_ns.has_key(self.RESULT_VAR_NAME):
                    del new_ns[self.RESULT_VAR_NAME]

                #exec
                exec bytecode in new_ns

                #check result
                if not new_ns.has_key(self.RESULT_VAR_NAME):
                    raise GeoAlgorithmExecutionException("FieldPyculator code execute error\n" +
                            "Field code block does not return '%s1' variable! Please declare this variable in your code!" %
                            self.RESULT_VAR_NAME)

                #write feature
                nElement += 1
                outFeat.setGeometry( feat.geometry() )
                outFeat.setAttributeMap( attrMap )
                outFeat.addAttribute(len(vprovider.fields()), QVariant(new_ns[self.RESULT_VAR_NAME]))
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


