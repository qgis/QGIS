# -*- coding: utf-8 -*-

"""
***************************************************************************
    lasinfo.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
    ---------------------
    Date                 : September 2013 and May 2016
    Copyright            : (C) 2013 by Martin Isenburg
    Email                : martin near rapidlasso point com
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

import os
from .LAStoolsUtils import LAStoolsUtils
from .LAStoolsAlgorithm import LAStoolsAlgorithm

from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterBoolean
from processing.core.outputs import OutputFile
from processing.core.parameters import ParameterNumber


class lasinfo(LAStoolsAlgorithm):

    COMPUTE_DENSITY = "COMPUTE_DENSITY"
    REPAIR_BB = "REPAIR_BB"
    REPAIR_COUNTERS = "REPAIR_COUNTERS"
    HISTO1 = "HISTO1"
    HISTO2 = "HISTO2"
    HISTO3 = "HISTO3"
    HISTOGRAM = ["---", "x", "y", "z", "intensity", "classification", "scan_angle", "user_data", "point_source", "gps_time", "X", "Y", "Z"]
    HISTO1_BIN = "HISTO1_BIN"
    HISTO2_BIN = "HISTO2_BIN"
    HISTO3_BIN = "HISTO3_BIN"
    OUTPUT = "OUTPUT"

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('lasinfo')
        self.group, self.i18n_group = self.trAlgorithm('LAStools')
        self.addParametersVerboseGUI()
        self.addParametersPointInputGUI()
        self.addParameter(ParameterBoolean(lasinfo.COMPUTE_DENSITY,
                                           self.tr("compute density"), False))
        self.addParameter(ParameterBoolean(lasinfo.REPAIR_BB,
                                           self.tr("repair bounding box"), False))
        self.addParameter(ParameterBoolean(lasinfo.REPAIR_COUNTERS,
                                           self.tr("repair counters"), False))
        self.addParameter(ParameterSelection(lasinfo.HISTO1,
                                             self.tr("histogram"), lasinfo.HISTOGRAM, 0))
        self.addParameter(ParameterNumber(lasinfo.HISTO1_BIN,
                                          self.tr("bin size"), 0, None, 1.0))
        self.addParameter(ParameterSelection(lasinfo.HISTO2,
                                             self.tr("histogram"), lasinfo.HISTOGRAM, 0))
        self.addParameter(ParameterNumber(lasinfo.HISTO2_BIN,
                                          self.tr("bin size"), 0, None, 1.0))
        self.addParameter(ParameterSelection(lasinfo.HISTO3,
                                             self.tr("histogram"), lasinfo.HISTOGRAM, 0))
        self.addParameter(ParameterNumber(lasinfo.HISTO3_BIN,
                                          self.tr("bin size"), 0, None, 1.0))
        self.addOutput(OutputFile(lasinfo.OUTPUT,
                                  self.tr("Output ASCII file")))
        self.addParametersAdditionalGUI()

    def processAlgorithm(self, progress):
        if (LAStoolsUtils.hasWine()):
            commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasinfo.exe")]
        else:
            commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasinfo")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputCommands(commands)
        if self.getParameterValue(lasinfo.COMPUTE_DENSITY):
            commands.append("-cd")
        if self.getParameterValue(lasinfo.REPAIR_BB):
            commands.append("-repair_bb")
        if self.getParameterValue(lasinfo.REPAIR_COUNTERS):
            commands.append("-repair_counters")
        histo = self.getParameterValue(lasinfo.HISTO1)
        if histo != 0:
            commands.append("-histo")
            commands.append(lasinfo.HISTOGRAM[histo])
            commands.append(unicode(self.getParameterValue(lasinfo.HISTO1_BIN)))
        histo = self.getParameterValue(lasinfo.HISTO2)
        if histo != 0:
            commands.append("-histo")
            commands.append(lasinfo.HISTOGRAM[histo])
            commands.append(unicode(self.getParameterValue(lasinfo.HISTO2_BIN)))
        histo = self.getParameterValue(lasinfo.HISTO3)
        if histo != 0:
            commands.append("-histo")
            commands.append(lasinfo.HISTOGRAM[histo])
            commands.append(unicode(self.getParameterValue(lasinfo.HISTO3_BIN)))
        commands.append("-o")
        commands.append(self.getOutputValue(lasinfo.OUTPUT))
        self.addParametersAdditionalCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
