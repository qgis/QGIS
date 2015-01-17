# -*- coding: utf-8 -*-

"""
***************************************************************************
    information.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterBoolean
from processing.core.outputs import OutputHTML
from processing.algs.gdal.GdalUtils import GdalUtils


class information(GdalAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    NOGCP = 'NOGCP'
    NOMETADATA = 'NOMETADATA'

    def commandLineName(self):
        return "gdalorg:rasterinfo"

    def defineCharacteristics(self):
        self.name = 'Information'
        self.group = '[GDAL] Miscellaneous'
        self.addParameter(ParameterRaster(information.INPUT,
            self.tr('Input layer'), False))
        self.addParameter(ParameterBoolean(information.NOGCP,
            self.tr('Suppress GCP info'), False))
        self.addParameter(ParameterBoolean(information.NOMETADATA,
            self.tr('Suppress metadata info'), False))
        self.addOutput(OutputHTML(information.OUTPUT,
            self.tr('Layer information')))

    def processAlgorithm(self, progress):
        arguments = []
        if self.getParameterValue(information.NOGCP):
            arguments.append('-nogcp')
        if self.getParameterValue(information.NOMETADATA):
            arguments.append('-nomd')
        arguments.append(self.getParameterValue(information.INPUT))
        GdalUtils.runGdal(['gdalinfo', GdalUtils.escapeAndJoin(arguments)],
                          progress)
        output = self.getOutputValue(information.OUTPUT)
        f = open(output, 'w')
        f.write('<pre>')
        for s in GdalUtils.getConsoleOutput()[1:]:
            f.write(unicode(s))
        f.write('</pre>')
        f.close()
