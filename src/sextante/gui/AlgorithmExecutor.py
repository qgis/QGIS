from PyQt4.QtGui import *
from PyQt4.QtCore import *
from qgis.core import *
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from sextante.core.QGisLayers import QGisLayers
from sextante.core.SextanteUtils import SextanteUtils
from sextante.gui.SextantePostprocessing import SextantePostprocessing
import traceback

class AlgorithmExecutor(QThread):
    percentageChanged = pyqtSignal(int)
    textChanged = pyqtSignal(QString)
    #~ cancelled = pyqtSignal()
    error = pyqtSignal()
    iterated = pyqtSignal(int)
    #started & finished inherited from QThread

    def __init__(self, alg, iterParam = None, parent = None):
        QThread.__init__(self, parent)
        self.algorithm = alg
        self.parameterToIterate = iterParam

        class Progress:
            def __init__(self, algex):
                self.algorithmExecutor = algex
            def setText(self, text):
                self.algorithmExecutor.textChanged.emit(text)
            def setPercentage(self, p):
                self.algorithmExecutor.percentageChanged.emit(p)
        self.progress = Progress(self)
        if self.parameterToIterate:
            self.run = self.runalgIterating

            #generate all single-feature layers
            settings = QSettings()
            systemEncoding = settings.value( "/UI/encoding", "System" ).toString()
            layerfile = alg.getParameterValue(self.parameterToIterate)
            layer = QGisLayers.getObjectFromUri(layerfile, False)
            provider = layer.dataProvider()
            allAttrs = provider.attributeIndexes()
            provider.select( allAttrs )
            feat = QgsFeature()
            self.filelist = []
            while provider.nextFeature(feat):
                output = SextanteUtils.getTempFilename("shp")
                self.filelist.append(output)
                writer = QgsVectorFileWriter(output, systemEncoding,provider.fields(), provider.geometryType(), provider.crs() )
                writer.addFeature(feat)
                del writer
        else:
            self.run = self.runalg

    def runalg(self):
        try:
            self.algorithm.execute(self.progress)
            #===================================================================
            # if self.algorithm.canceled:
            #    self.canceled.emit()
            #===================================================================
        except GeoAlgorithmExecutionException as e :
            self.error.emit(e.msg)
        except BaseException as e:
            self.error.emit(str(e))

    def runalgIterating(self):
        outputs = {}
        #store output values to use them later as basenames for all outputs
        for out in self.algorithm.outputs:
            outputs[out.name] = out.value
        i = 1
        for f in self.filelist:
            self.algorithm.setParameterValue(self.parameterToIterate, f)
            for out in self.algorithm.outputs:
                filename = outputs[out.name]
                if filename:
                    filename = filename[:filename.rfind(".")] + "_" + str(i) + filename[filename.rfind("."):]
                out.value = filename
            self.progress.setText("Executing iteration " + str(i) + "/" + str(len(self.filelist)) + "...")
            self.progress.setPercentage((i * 100) / len(self.filelist))
            self.runalg()
            #===================================================================
            # if self.algorithm.canceled:
            #    return
            #===================================================================
            self.iterated.emit(i)
            i += 1
