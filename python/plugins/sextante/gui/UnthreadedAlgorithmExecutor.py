from PyQt4.QtGui import *
from PyQt4.QtCore import *
from qgis.core import *
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from sextante.core.QGisLayers import QGisLayers
from sextante.core.SextanteUtils import SextanteUtils
from sextante.gui.SextantePostprocessing import SextantePostprocessing
import traceback

class UnthreadedAlgorithmExecutor:

    @staticmethod
    def runalg(alg, progress):
        '''Executes a given algorithm, showing its progress in the progress object passed along.
        Return true if everything went OK, false if the algorithm could not be completed'''
        try:
            alg.execute(progress)
            return True
        except GeoAlgorithmExecutionException, e :
            QMessageBox.critical(None, "Error", e.msg)
            return False
        except Exception:
            QMessageBox.critical(None, "Error", traceback.format_exc())
            return False

    @staticmethod
    def runalgIterating(alg,paramToIter,progress):
        #generate all single-feature layers
        settings = QSettings()
        systemEncoding = settings.value( "/UI/encoding", "System" ).toString()
        layerfile = alg.getParameterValue(paramToIter)
        layer = QGisLayers.getObjectFromUri(layerfile, False)
        provider = layer.dataProvider()
        allAttrs = provider.attributeIndexes()
        provider.select( allAttrs )
        feat = QgsFeature()
        filelist = []
        outputs = {}
        while provider.nextFeature(feat):
            output = SextanteUtils.getTempFilename("shp")
            filelist.append(output)
            writer = QgsVectorFileWriter(output, systemEncoding,provider.fields(), provider.geometryType(), provider.crs() )
            writer.addFeature(feat)
            del writer

        #store output values to use them later as basenames for all outputs
        for out in alg.outputs:
            outputs[out.name] = out.value

        #now run all the algorithms
        i = 1
        for f in filelist:
            alg.setParameterValue(paramToIter, f)
            for out in alg.outputs:
                filename = outputs[out.name]
                if filename:
                    filename = filename[:filename.rfind(".")] + "_" + str(i) + filename[filename.rfind("."):]
                out.value = filename
            progress.setText("Executing iteration " + str(i) + "/" + str(len(filelist)) + "...")
            progress.setPercentage((i * 100) / len(filelist))
            if UnthreadedAlgorithmExecutor.runalg(alg, SilentProgress()):
                SextantePostprocessing.handleAlgorithmResults(alg, False)
                i+=1
            else:
                return False;

        return True


class SilentProgress():

    def setText(self, text):
        pass

    def setPercentage(self, i):
        pass

    def setInfo(self, _):
        pass

    def setCommand(self, _):
        pass

    def setDebugInfo(self, _):
        pass

    def setConsoleInfo(self, _):
        pass
