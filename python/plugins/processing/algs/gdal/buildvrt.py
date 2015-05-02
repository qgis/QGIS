# -*- coding: utf-8 -*-

"""
***************************************************************************
    merge.py
    ---------------------
    Date                 : October 2014
    Copyright            : (C) 2014 by Radoslaw Guzinski
    Email                : rmgu at dhi-gras dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Radoslaw Guzinski'
__date__ = 'October 2014'
__copyright__ = '(C) 2014, Radoslaw Guzinski'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.core.outputs import OutputRaster
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterMultipleInput
from processing.core.parameters import ParameterSelection
from processing.algs.gdal.GdalUtils import GdalUtils
from processing.tools.system import tempFolder

import os

class buildvrt(GdalAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    RESOLUTION = 'RESOLUTION'
    SEPARATE = 'SEPARATE'
    PROJ_DIFFERENCE = 'PROJ_DIFFERENCE'

    RESOLUTION_OPTIONS = ['average', 'highest', 'lowest']

    def defineCharacteristics(self):
        self.name = 'Build Virtual Raster'
        self.group = '[GDAL] Miscellaneous'
        self.addParameter(ParameterMultipleInput(self.INPUT,
            self.tr('Input layers'), ParameterMultipleInput.TYPE_RASTER))
        self.addParameter(ParameterSelection(self.RESOLUTION,
            self.tr('Resolution'), self.RESOLUTION_OPTIONS, 0))
        self.addParameter(ParameterBoolean(self.SEPARATE,
            self.tr('Layer stack'), True))
        self.addParameter(ParameterBoolean(self.PROJ_DIFFERENCE,
            self.tr('Allow projection difference'), False))
        self.addOutput(OutputRaster(buildvrt.OUTPUT, self.tr('Virtual')))

    def processAlgorithm(self, progress):
        arguments = []
        arguments.append('-resolution')
        arguments.append(self.RESOLUTION_OPTIONS[self.getParameterValue(self.RESOLUTION)])
        if self.getParameterValue(buildvrt.SEPARATE):
            arguments.append('-separate')
        if self.getParameterValue(buildvrt.PROJ_DIFFERENCE):
            arguments.append('-allow_projection_difference')
        # Always write input files to a text file in case there are many of them and the
        # length of the command will be longer then allowed in command prompt
        listFile = os.path.join(tempFolder(), 'buildvrtInputFiles.txt')
        with open(listFile, 'w') as f:
            f.write(self.getParameterValue(buildvrt.INPUT).replace(';', '\n'))
        arguments.append('-input_file_list')
        arguments.append(listFile)
        out = self.getOutputValue(buildvrt.OUTPUT)
        # Ideally the file extensions should be limited to just .vrt but I'm not sure how
        # to do it simply so instead a check is performed.
        _, ext = os.path.splitext(out)
        if not ext.lower() == '.vrt':
            out = out.replace(ext, '.vrt')
            self.setOutputValue(self.OUTPUT, out)
        arguments.append(out)


        GdalUtils.runGdal(['gdalbuildvrt', GdalUtils.escapeAndJoin(arguments)], progress)
