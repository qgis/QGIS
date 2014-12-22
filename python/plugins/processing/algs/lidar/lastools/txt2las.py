# -*- coding: utf-8 -*-

"""
***************************************************************************
    txt2las.py
    ---------------------
    Date                 : September 2013
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

__author__ = 'Martin Isenburg'
__date__ = 'September 2013'
__copyright__ = '(C) 2013, Martin Isenburg'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from LAStoolsUtils import LAStoolsUtils
from LAStoolsAlgorithm import LAStoolsAlgorithm

from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterFile
from processing.core.parameters import ParameterSelection

class txt2las(LAStoolsAlgorithm):

    INPUT = "INPUT"
    PARSE = "PARSE"
    SKIP = "SKIP"
    SCALE_FACTOR_XY = "SCALE_FACTOR_XY"
    SCALE_FACTOR_Z = "SCALE_FACTOR_Z"

    STATE_PLANES = ["---", "AK_10", "AK_2", "AK_3", "AK_4", "AK_5", "AK_6", "AK_7", "AK_8", "AK_9", "AL_E", "AL_W", "AR_N", "AR_S", "AZ_C", "AZ_E", "AZ_W", "CA_I", "CA_II", "CA_III", "CA_IV", "CA_V", "CA_VI", "CA_VII", "CO_C", "CO_N", "CO_S", "CT", "DE", "FL_E", "FL_N", "FL_W", "GA_E", "GA_W", "HI_1", "HI_2", "HI_3", "HI_4", "HI_5", "IA_N", "IA_S", "ID_C", "ID_E", "ID_W", "IL_E", "IL_W", "IN_E", "IN_W", "KS_N", "KS_S", "KY_N", "KY_S", "LA_N", "LA_S", "MA_I", "MA_M", "MD", "ME_E", "ME_W", "MI_C", "MI_N", "MI_S", "MN_C", "MN_N", "MN_S", "MO_C", "MO_E", "MO_W", "MS_E", "MS_W", "MT_C", "MT_N", "MT_S", "NC", "ND_N", "ND_S", "NE_N", "NE_S", "NH", "NJ", "NM_C", "NM_E", "NM_W", "NV_C", "NV_E", "NV_W", "NY_C", "NY_E", "NY_LI", "NY_W", "OH_N", "OH_S", "OK_N", "OK_S", "OR_N", "OR_S", "PA_N", "PA_S", "PR", "RI", "SC_N", "SC_S", "SD_N", "SD_S", "St.Croix", "TN", "TX_C", "TX_N", "TX_NC", "TX_S", "TX_SC", "UT_C", "UT_N", "UT_S", "VA_N", "VA_S", "VT", "WA_N", "WA_S", "WI_C", "WI_N", "WI_S", "WV_N", "WV_S", "WY_E", "WY_EC", "WY_W", "WY_WC"]

    UTM_ZONES = ["---", "1 (north)", "2 (north)", "3 (north)", "4 (north)", "5 (north)", "6 (north)", "7 (north)", "8 (north)", "9 (north)", "10 (north)", "11 (north)", "12 (north)", "13 (north)", "14 (north)", "15 (north)", "16 (north)", "17 (north)", "18 (north)", "19 (north)", "20 (north)", "21 (north)", "22 (north)", "23 (north)", "24 (north)", "25 (north)", "26 (north)", "27 (north)", "28 (north)", "29 (north)", "30 (north)", "31 (north)", "32 (north)", "33 (north)", "34 (north)", "35 (north)", "36 (north)", "37 (north)", "38 (north)", "39 (north)", "40 (north)", "41 (north)", "42 (north)", "43 (north)", "44 (north)", "45 (north)", "46 (north)", "47 (north)", "48 (north)", "49 (north)", "50 (north)", "51 (north)", "52 (north)", "53 (north)", "54 (north)", "55 (north)", "56 (north)", "57 (north)", "58 (north)", "59 (north)", "60 (north)", "1 (south)", "2 (south)", "3 (south)", "4 (south)", "5 (south)", "6 (south)", "7 (south)", "8 (south)", "9 (south)", "10 (south)", "11 (south)", "12 (south)", "13 (south)", "14 (south)", "15 (south)", "16 (south)", "17 (south)", "18 (south)", "19 (south)", "20 (south)", "21 (south)", "22 (south)", "23 (south)", "24 (south)", "25 (south)", "26 (south)", "27 (south)", "28 (south)", "29 (south)", "30 (south)", "31 (south)", "32 (south)", "33 (south)", "34 (south)", "35 (south)", "36 (south)", "37 (south)", "38 (south)", "39 (south)", "40 (south)", "41 (south)", "42 (south)", "43 (south)", "44 (south)", "45 (south)", "46 (south)", "47 (south)", "48 (south)", "49 (south)", "50 (south)", "51 (south)", "52 (south)", "53 (south)", "54 (south)", "55 (south)", "56 (south)", "57 (south)", "58 (south)", "59 (south)", "60 (south)"]

    PROJECTIONS = ["---", "utm", "sp83", "sp27", "longlat", "latlong", "ecef"]

    PROJECTION = "PROJECTION"
    UTM = "UTM"
    SP = "SP"

    def defineCharacteristics(self):
        self.name = "txt2las"
        self.group = "LAStools"
        self.addParametersVerboseGUI()
        self.addParameter(ParameterFile(txt2las.INPUT, "Input ASCII file"))
        self.addParameter(ParameterString(txt2las.PARSE, "parse lines as", "xyz"))
        self.addParameter(ParameterNumber(txt2las.SKIP, "skip the first n lines", 0, None, 0))
        self.addParameter(ParameterNumber(txt2las.SCALE_FACTOR_XY, "resolution of x and y coordinate", 0, None, 0.01))
        self.addParameter(ParameterNumber(txt2las.SCALE_FACTOR_Z, "resolution of z coordinate", 0, None, 0.01))
        self.addParameter(ParameterSelection(txt2las.PROJECTION, "projection", txt2las.PROJECTIONS, 0))
        self.addParameter(ParameterSelection(txt2las.UTM, "utm zone", txt2las.UTM_ZONES, 0))
        self.addParameter(ParameterSelection(txt2las.SP, "state plane code", txt2las.STATE_PLANES, 0))
        self.addParametersPointOutputGUI()
        self.addParametersAdditionalGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "txt2las")]
        self.addParametersVerboseCommands(commands)
        commands.append("-i")
        commands.append(self.getParameterValue(txt2las.INPUT))
        parse_string = self.getParameterValue(txt2las.PARSE)
        if parse_string != "xyz":
            commands.append("-parse")
            commands.append(parse_string)
        skip = self.getParameterValue(txt2las.SKIP)
        if parse_string != 0:
            commands.append("-skip")
            commands.append(str(skip))
        scale_factor_xy = self.getParameterValue(txt2las.SCALE_FACTOR_XY)
        scale_factor_z = self.getParameterValue(txt2las.SCALE_FACTOR_Z)
        if scale_factor_xy != 0.01 or scale_factor_z != 0.01:
            commands.append("-set_scale")
            commands.append(str(scale_factor_xy) + " " + str(scale_factor_xy) + " " + str(scale_factor_z))
        projection = self.getParameterValue(txt2las.PROJECTION)
        if projection != 0:
            if projection == 1:
                utm_zone = self.getParameterValue(txt2las.UTM)
                if utm_zone != 0:
                    commands.append("-" + txt2las.PROJECTIONS[projection])
                    if utm_zone > 60:
                        commands.append(str(utm_zone - 60) + "M")
                    else:
                        commands.append(str(utm_zone) + "N")
            elif projection < 4:
                sp_code = self.getParameterValue(txt2las.SP)
                if sp_code != 0:
                    commands.append("-" + txt2las.PROJECTIONS[projection])
                    commands.append(txt2las.STATE_PLANES[sp_code])
            else:
                commands.append("-" + txt2las.PROJECTIONS[projection])
        self.addParametersPointOutputCommands(commands)
        self.addParametersAdditionalCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
