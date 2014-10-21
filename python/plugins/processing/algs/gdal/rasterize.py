# -*- coding: utf-8 -*-

"""
***************************************************************************
    rasterize.py
    ---------------------
    Date                 : September 2013
    Copyright            : (C) 2013 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'September 2013'
__copyright__ = '(C) 2013, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.tools.system import *
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTableField
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterBoolean
from processing.core.outputs import OutputRaster
from processing.algs.gdal.GdalUtils import GdalUtils


class rasterize(GdalAlgorithm):

    INPUT = 'INPUT'
    FIELD = 'FIELD'
    DIMENSIONS = 'DIMENSIONS'
    WIDTH = 'WIDTH'
    HEIGHT = 'HEIGHT'
    WRITEOVER = 'WRITEOVER'     
    RTYPE = 'RTYPE'
    OUTPUT = 'OUTPUT'
    
    TYPE = ['Byte','Int16','UInt16','UInt32','Int32','Float32','Float64','CInt16','CInt32','CFloat32','CFloat64']

    def commandLineName(self):
        return "gdalogr:rasterize"

    def defineCharacteristics(self):
        self.name = 'Rasterize (vector to raster)'
        self.group = '[GDAL] Conversion'
        self.addParameter(ParameterVector(self.INPUT, 'Input layer'))
        self.addParameter(ParameterTableField(self.FIELD, 'Attribute field',
                          self.INPUT))
        self.addParameter(ParameterBoolean(self.WRITEOVER,
                          'Write values inside an existing raster layer(*)', False))
        self.addParameter(ParameterSelection(self.DIMENSIONS,
                          'Set output raster size (ignored if above option is checked)', ['Output size in pixels',
                          'Output resolution in map units per pixel'], 1))
        self.addParameter(ParameterNumber(self.WIDTH, 'Horizontal', 0.0,
                          99999999.999999, 100.0))
        self.addParameter(ParameterNumber(self.HEIGHT, 'Vertical', 0.0,
                          99999999.999999, 100.0))
	self.addParameter(ParameterSelection(self.RTYPE, 'Raster type',
                          self.TYPE, 0))

        self.addOutput(OutputRaster(self.OUTPUT, 'Output layer: mandatory to choose an existing raster layer if the (*) option is selected'))

    def processAlgorithm(self, progress):
        writeOver = self.getParameterValue(self.WRITEOVER)
        
        arguments = []
        arguments.append('-a')
        arguments.append(str(self.getParameterValue(self.FIELD)))

        if not writeOver:
             arguments.append('-ot')
             arguments.append(self.TYPE[self.getParameterValue(self.RTYPE)])  
	     dimType = self.getParameterValue(self.DIMENSIONS)
	     if dimType == 0:
		# size in pixels
		arguments.append('-ts')
	     else:
		    # resolution in map units per pixel
		arguments.append('-tr')
		arguments.append(str(self.getParameterValue(self.WIDTH)))
		arguments.append(str(self.getParameterValue(self.HEIGHT)))

        arguments.append('-l')
        arguments.append(
                os.path.basename(os.path.splitext(
                        unicode(self.getParameterValue(self.INPUT)))[0]))
        arguments.append(unicode(self.getParameterValue(self.INPUT)))

        arguments.append(unicode(self.getOutputValue(self.OUTPUT)))

        GdalUtils.runGdal(['gdal_rasterize',
                          GdalUtils.escapeAndJoin(arguments)], progress)
