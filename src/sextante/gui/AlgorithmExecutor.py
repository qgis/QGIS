from PyQt4.QtGui import *
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from sextante.core.QGisLayers import QGisLayers
from sextante.core.SextanteUtils import SextanteUtils

class AlgorithmExecutor:

    @staticmethod
    def runalg(alg, progress):
        '''executes a given algorithm, showing its progress in the progress object passed along.
        Return true if everything went OK, false if the algorithm was canceled or there was
        any problem and could not be completed'''
        try:
            alg.execute(progress)
            return not alg.canceled
        except GeoAlgorithmExecutionException, e :
            QMessageBox.critical(None, "Error", e.msg)
            return False

    @staticmethod
    def runalgIterating(alg,paramToIter,progress):
        #generate all single-feature layers
        settings = QSettings()
        systemEncoding = settings.value( "/UI/encoding", "System" ).toString()
        layerfile = alg.getParameterFromName(paramToIter)
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
        #now run all the algorithms
        for out in alg.outputs:
            output[out.name] = out.value

        i = 1
        for f in filelist:
            alg.setOutputValue(paramToIter, f)
            for out in alg.outputs:
                filename = outputs[out.name]
                if filename:
                    filename = filename[:filename.rfind(".")] + "_" + str(i) +  filename[filename.rfind("."):]
                out.value = filename
            if AlgorithmExecutor.runalg(alg, SilentProgress()):
                progress.setValue(i/len(f))
                i+=1
            else:
                return False;

        return True


class SilentProgress():

    def setText(self, text):
        pass

    def setPercentage(self, i):
        pass
