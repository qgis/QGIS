from sextante.core.SextanteConfig import SextanteConfig
from sextante.core.SextanteUtils import SextanteUtils
from qgis.core import *
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.gdal.GdalUtils import GdalUtils

class LayerExporter():

    '''This class provides method to export layers so they can be used by third party applications.
    These method are used by the GeoAlgorithm class and allow the developer to use transparently
    any layer that is loaded into QGIS, without having to worry about its origin'''

    @staticmethod
    def exportVectorLayer(layer):
        '''Takes a QgsVectorLayer and returns the filename to refer to it, which allows external
        apps which support only file-based layers to use it. It performs the necessary export
        in case the input layer is not in a standard format suitable for most applications, it is
        a remote one or db-based (non-file based) one, or if there is a selection and it should be
        used, exporting just the selected features.
        Currently, the output is restricted to shapefiles, so anything that is not in a shapefile
        will get exported'''
        settings = QSettings()
        systemEncoding = settings.value( "/UI/encoding", "System" ).toString()
        output = SextanteUtils.getTempFilename("shp")
        provider = layer.dataProvider()
        allAttrs = provider.attributeIndexes()
        provider.select( allAttrs )
        useSelection = SextanteConfig.getSetting(SextanteConfig.USE_SELECTED)
        if useSelection and layer.selectedFeatureCount() != 0:
            writer = QgsVectorFileWriter( output, systemEncoding,provider.fields(), provider.geometryType(), provider.crs() )
            selection = layer.selectedFeatures()
            for feat in selection:
                writer.addFeature(feat)
            del writer
            return output
        else:
            if (not unicode(layer.source()).endswith("shp")):
                writer = QgsVectorFileWriter( output, systemEncoding,provider.fields(), provider.geometryType(), provider.crs() )
                feat = QgsFeature()
                while provider.nextFeature(feat):
                    writer.addFeature(feat)
                del writer
                return output
            else:
                return unicode(layer.source())



    @staticmethod
    def exportRasterLayer(layer):
        '''Takes a QgsRasterLayer and returns the filename to refer to it, which allows external
        apps which support only file-based layers to use it. It performs the necessary export
        in case the input layer is not in a standard format suitable for most applications, it is
        a remote one or db-based (non-file based) one
        Currently, the output is restricted to geotiff, but not all other formats are exported.
        Only those formats not supported by GDAL are exported, so it is assumed that the external
        app uses GDAL to read the layer'''
        exts = GdalUtils.getSupportedRasterExtensions()
        for ext in exts:
            if (unicode(layer.source()).endswith(ext)):
                return unicode(layer.source())

        #TODO:Do the conversion here
        return unicode(layer.source())


