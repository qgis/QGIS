# -*- coding: utf-8 -*-

"""
***************************************************************************
    ogrinfo.py
    ---------------------
    Date                 : November 2012
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
__date__ = 'November 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputHTML

from processing.algs.gdal.GdalUtils import GdalUtils
from processing.algs.gdal.OgrAlgorithm import OgrAlgorithm


class OgrInfo(OgrAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name = 'Information'
        self.group = '[OGR] Miscellaneous'

        self.addParameter(ParameterVector(self.INPUT, self.tr('Input layer'),
                          [ParameterVector.VECTOR_TYPE_ANY], False))

        self.addOutput(OutputHTML(self.OUTPUT, self.tr('Layer information')))

    def getConsoleCommands(self):
        arguments = ["ogrinfo"]
        arguments.append('-al')
        arguments.append('-so')
        layer = self.getParameterValue(self.INPUT)
        conn = self.ogrConnectionString(layer)
        arguments.append(conn)
        return arguments

    def processAlgorithm(self, progress):
        GdalUtils.runGdal(self.getConsoleCommands(), progress)
        output = self.getOutputValue(self.OUTPUT)
        f = open(output, 'w')
        f.write('<pre>')
        for s in GdalUtils.getConsoleOutput()[1:]:
            f.write(unicode(s))
        f.write('</pre>')
        f.close()
