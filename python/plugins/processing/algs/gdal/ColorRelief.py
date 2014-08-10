# -*- coding: utf-8 -*-

"""
***************************************************************************
    ColorRelief.py
    ---------------------
    Date                 : October 2013
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
__date__ = 'October 2013'
__copyright__ = '(C) 2013, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


from PyQt4.QtGui import *

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterFile
from processing.core.parameters import ParameterSelection
from processing.core.outputs import OutputRaster
from processing.algs.gdal.GdalUtils import GdalUtils
from processing.tools.system import *


class ColorRelief(GdalAlgorithm):

    INPUT = 'INPUT'
    BAND = 'BAND'
    COMPUTE_EDGES = 'COMPUTE_EDGES'
    COLOR_TABLE = 'COLOR_TABLE'
    MATCH_MODE = 'MATCH_MODE'
    OUTPUT = 'OUTPUT'

    MATCHING_MODES = ['"0,0,0,0" RGBA', 'Exact color', 'Nearest color']

    #def getIcon(self):
    #    filepath = os.path.dirname(__file__) + '/icons/dem.png'
    #    return QIcon(filepath)

    def defineCharacteristics(self):
        self.name = 'Color relief'
        self.group = '[GDAL] Analysis'
        self.addParameter(ParameterRaster(self.INPUT, 'Input layer'))
        self.addParameter(ParameterNumber(self.BAND, 'Band number', 1, 99, 1))
        self.addParameter(ParameterBoolean(self.COMPUTE_EDGES, 'Compute edges',
                          False))
        self.addParameter(ParameterFile(self.COLOR_TABLE,
                          'Color configuration file', optional=False))
        self.addParameter(ParameterSelection(self.MATCH_MODE,
                          'Matching mode', self.MATCHING_MODES, 0))

        self.addOutput(OutputRaster(self.OUTPUT, 'Output file'))

    def processAlgorithm(self, progress):
        arguments = ['color-relief']
        arguments.append(unicode(self.getParameterValue(self.INPUT)))
        arguments.append(unicode(self.getParameterValue(self.COLOR_TABLE)))
        #filePath = unicode(self.getParameterValue(self.COLOR_TABLE))
        #if filePath is None or filePath == '':
        #    filePath = os.path.join(os.path.dirname(__file__), 'terrain.txt')
        #arguments.append(filePath)
        arguments.append(unicode(self.getOutputValue(self.OUTPUT)))

        arguments.append('-b')
        arguments.append(str(self.getParameterValue(self.BAND)))

        if self.getParameterValue(self.COMPUTE_EDGES):
            arguments.append('-compute_edges')

        mode = self.getParameterValue(self.MATCH_MODE)
        if mode == 1:
            arguments.append('-exact_color_entry')
        elif mode == 2:
            arguments.append('-nearest_color_entry')

        GdalUtils.runGdal(['gdaldem',
                          GdalUtils.escapeAndJoin(arguments)], progress)
