# -*- coding: utf-8 -*-

"""
***************************************************************************
    TINSurfaceCreate.py
    ---------------------
    Date                 : June 2014
    Copyright            : (C) 2014 by Agresta S. Coop
    Email                : iescamochero at agresta dot org
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Agresta S. Coop - www.agresta.org'
__date__ = 'June 2014'
__copyright__ = '(C) 2014, Agresta S. Coop'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
import subprocess
from processing.core.parameters import ParameterFile
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterString
from processing.core.outputs import OutputFile
from .FusionAlgorithm import FusionAlgorithm
from .FusionUtils import FusionUtils


class TinSurfaceCreate(FusionAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    CELLSIZE = 'CELLSIZE'
    XYUNITS = 'XYUNITS'
    ZUNITS = 'ZUNITS'
    UNITS = ['Meter', 'Feet']
    CLASS = 'CLASS'
    RETURN = 'RETURN'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Tin Surface Create')
        self.group, self.i18n_group = self.trAlgorithm('Surface')
        self.addParameter(ParameterFile(
            self.INPUT, self.tr('Input LAS layer')))
        self.addParameter(ParameterNumber(self.CELLSIZE,
                                          self.tr('Cellsize'), 0, None, 10.0))
        self.addParameter(ParameterSelection(self.XYUNITS,
                                             self.tr('XY Units'), self.UNITS))
        self.addParameter(ParameterSelection(self.ZUNITS,
                                             self.tr('Z Units'), self.UNITS))
        self.addOutput(OutputFile(self.OUTPUT,
                                  self.tr('.dtm output surface'), 'dtm'))
        class_var = ParameterString(self.CLASS,
                                    self.tr('Class'), '', False, True)
        class_var.isAdvanced = True
        self.addParameter(class_var)
        return_sel = ParameterString(self.RETURN,
                                     self.tr('Select specific return'), '', False, True)
        return_sel.isAdvanced = True
        self.addParameter(return_sel)

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), 'TINSurfaceCreate.exe')]
        commands.append('/verbose')
        class_var = self.getParameterValue(self.CLASS)
        if unicode(class_var).strip():
            commands.append('/class:' + unicode(class_var))
            return_sel = self.getParameterValue(self.RETURN)
        if unicode(return_sel).strip():
            commands.append('/return:' + unicode(return_sel))
        outFile = self.getOutputValue(self.OUTPUT)
        commands.append(outFile)
        commands.append(unicode(self.getParameterValue(self.CELLSIZE)))
        commands.append(self.UNITS[self.getParameterValue(self.XYUNITS)][0])
        commands.append(self.UNITS[self.getParameterValue(self.ZUNITS)][0])
        commands.append('0')
        commands.append('0')
        commands.append('0')
        commands.append('0')
        files = self.getParameterValue(self.INPUT).split(';')
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            commands.extend(files)
        FusionUtils.runFusion(commands, progress)
        commands = [os.path.join(FusionUtils.FusionPath(), 'DTM2ASCII.exe')]
        commands.append('/raster')
        commands.append(outFile)
        commands.append(self.getOutputValue(self.OUTPUT))
        p = subprocess.Popen(commands, shell=True)
        p.wait()
