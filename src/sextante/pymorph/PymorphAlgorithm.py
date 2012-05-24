from sextante.script.ScriptAlgorithm import ScriptAlgorithm
from sextante.parameters.ParameterRaster import ParameterRaster
from sextante.outputs.OutputRaster import OutputRaster
from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.pymorph.AlgNames import AlgNames
import os

class PymorphAlgorithm(ScriptAlgorithm):

    LOAD_LAYER_SCRIPT = "from sextante.gdal.GdalUtils import GdalUtils\n" \
                        +"from sextante.pymorph.mmorph import datatype\n" \
                        + "import numpy\n" \
                        +"gdal_datatypes={'binary':'Byte','uint8':'Byte','uint16':'UInt16','int32':'Int32'}\n" \
                        + "try:\n" \
                        + "\tfrom osgeo import gdal\n" \
                        + "except ImportError:\n" \
                        + "\timport gdal\n" \
                        + "gdal.AllRegister()\n" \
                        + "img = gdal.Open(input_filename)\n"\
                        + "input_array = img.ReadAsArray()\n" \
                        + "if isinstance(input_array[0][0], (numpy.float32, numpy.float64)):\n" \
                        + "\tinput_array = input_array.astype(numpy.int32)\n"

    SAVE_LAYER_SCRIPT = "\ndrv = gdal.GetDriverByName(GdalUtils.getFormatShortNameFromFilename(output_filename))\n" \
                        + "out = drv.Create(output_filename, img.RasterXSize, img.RasterYSize, 1, gdal.GetDataTypeByName('UInt16'))\n"\
                        + "out.SetGeoTransform( img.GetGeoTransform())\n"\
                        + "out.SetProjection( img.GetProjectionRef())\n"\
                        + "out.GetRasterBand(1).WriteArray(output_array)"
                        #gdal_datatypes[datatype(output_array)]

    def getCopy(self):
        newone = PymorphAlgorithm(self.descriptionFile)
        newone.provider = self.provider
        return newone

    def getIcon(self):
        return GeoAlgorithm.getIcon(self)

    def defineCharacteristicsFromFile(self):
        ScriptAlgorithm.defineCharacteristicsFromFile(self)
        self.parameters.insert(0, ParameterRaster("input_filename", "Input image", False))
        self.outputs.append(OutputRaster("output_filename", "Output image"))
        self.script = self.LOAD_LAYER_SCRIPT + self.script + self.SAVE_LAYER_SCRIPT
        self.cmdname = self.name
        self.name = AlgNames.getName(self.name)

    def helpFile(self):
        command = os.path.basename(self.descriptionFile)
        command = command[:command.rfind(".")].replace("_", "")
        filename = os.path.dirname(__file__) + "/html/morph/morph/mm" + command + ".html"
        return filename

    def commandLineName(self):
        return "pymorph:" + ":" + self.cmdname
