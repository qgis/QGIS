from PyQt4.QtCore import *

from sextante.outputs.Output import Output
from sextante.core.SextanteVectorWriter import SextanteVectorWriter

class OutputVector(Output):

    encoding = None

    def getFileFilter(self,alg):
        exts = alg.provider.getSupportedOutputVectorLayerExtensions()
        for i in range(len(exts)):
            exts[i] = exts[i].upper() + " files(*." + exts[i].lower() + ")"
        return ";;".join(exts)

    def getDefaultFileExtension(self, alg):
        return alg.provider.getSupportedOutputVectorLayerExtensions()[0]

    def getVectorWriter(self, fields, geomType, crs, options=None):
        '''Returns a suitable writer to which features can be added as a
        result of the algorithm. Use this to transparently handle output
        values instead of creating your own method.

        Executing this method might modify the object, adding additional
        information to it, so the writer can be later accessed and processed
        within QGIS. It should be called just once, since a new call might
        result in previous data being replaced, thus rendering a previously
        obtained writer useless

        @param fields   a dict of int-QgsField
        @param geomType a suitable geometry type, as it would be passed
                        to a QgsVectorFileWriter constructor
        @param crs      the crs of the layer to create

        @return writer  instance of the vectoe writer class
        '''

        if self.encoding is None:
            settings = QSettings()
            self.encoding = settings.value("/SextanteQGIS/encoding", "System").toString()

        w = SextanteVectorWriter(self.value, self.encoding, fields, geomType, crs, options)
        self.memoryLayer = w.memLayer
        return w
