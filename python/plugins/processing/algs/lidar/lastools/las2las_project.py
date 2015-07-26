# -*- coding: utf-8 -*-

"""
***************************************************************************
    las2las_project.py
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

from processing.core.parameters import ParameterSelection

class las2las_project(LAStoolsAlgorithm):

    STATE_PLANES = ["---", "AK_10", "AK_2", "AK_3", "AK_4", "AK_5", "AK_6", "AK_7", "AK_8", "AK_9", "AL_E", "AL_W", "AR_N", "AR_S", "AZ_C", "AZ_E", "AZ_W", "CA_I", "CA_II", "CA_III", "CA_IV", "CA_V", "CA_VI", "CA_VII", "CO_C", "CO_N", "CO_S", "CT", "DE", "FL_E", "FL_N", "FL_W", "GA_E", "GA_W", "HI_1", "HI_2", "HI_3", "HI_4", "HI_5", "IA_N", "IA_S", "ID_C", "ID_E", "ID_W", "IL_E", "IL_W", "IN_E", "IN_W", "KS_N", "KS_S", "KY_N", "KY_S", "LA_N", "LA_S", "MA_I", "MA_M", "MD", "ME_E", "ME_W", "MI_C", "MI_N", "MI_S", "MN_C", "MN_N", "MN_S", "MO_C", "MO_E", "MO_W", "MS_E", "MS_W", "MT_C", "MT_N", "MT_S", "NC", "ND_N", "ND_S", "NE_N", "NE_S", "NH", "NJ", "NM_C", "NM_E", "NM_W", "NV_C", "NV_E", "NV_W", "NY_C", "NY_E", "NY_LI", "NY_W", "OH_N", "OH_S", "OK_N", "OK_S", "OR_N", "OR_S", "PA_N", "PA_S", "PR", "RI", "SC_N", "SC_S", "SD_N", "SD_S", "St.Croix", "TN", "TX_C", "TX_N", "TX_NC", "TX_S", "TX_SC", "UT_C", "UT_N", "UT_S", "VA_N", "VA_S", "VT", "WA_N", "WA_S", "WI_C", "WI_N", "WI_S", "WV_N", "WV_S", "WY_E", "WY_EC", "WY_W", "WY_WC"]

    UTM_ZONES = ["---", "1 (north)", "2 (north)", "3 (north)", "4 (north)", "5 (north)", "6 (north)", "7 (north)", "8 (north)", "9 (north)", "10 (north)", "11 (north)", "12 (north)", "13 (north)", "14 (north)", "15 (north)", "16 (north)", "17 (north)", "18 (north)", "19 (north)", "20 (north)", "21 (north)", "22 (north)", "23 (north)", "24 (north)", "25 (north)", "26 (north)", "27 (north)", "28 (north)", "29 (north)", "30 (north)", "31 (north)", "32 (north)", "33 (north)", "34 (north)", "35 (north)", "36 (north)", "37 (north)", "38 (north)", "39 (north)", "40 (north)", "41 (north)", "42 (north)", "43 (north)", "44 (north)", "45 (north)", "46 (north)", "47 (north)", "48 (north)", "49 (north)", "50 (north)", "51 (north)", "52 (north)", "53 (north)", "54 (north)", "55 (north)", "56 (north)", "57 (north)", "58 (north)", "59 (north)", "60 (north)", "1 (south)", "2 (south)", "3 (south)", "4 (south)", "5 (south)", "6 (south)", "7 (south)", "8 (south)", "9 (south)", "10 (south)", "11 (south)", "12 (south)", "13 (south)", "14 (south)", "15 (south)", "16 (south)", "17 (south)", "18 (south)", "19 (south)", "20 (south)", "21 (south)", "22 (south)", "23 (south)", "24 (south)", "25 (south)", "26 (south)", "27 (south)", "28 (south)", "29 (south)", "30 (south)", "31 (south)", "32 (south)", "33 (south)", "34 (south)", "35 (south)", "36 (south)", "37 (south)", "38 (south)", "39 (south)", "40 (south)", "41 (south)", "42 (south)", "43 (south)", "44 (south)", "45 (south)", "46 (south)", "47 (south)", "48 (south)", "49 (south)", "50 (south)", "51 (south)", "52 (south)", "53 (south)", "54 (south)", "55 (south)", "56 (south)", "57 (south)", "58 (south)", "59 (south)", "60 (south)"]

    PROJECTIONS = ["---", "utm", "sp83", "sp27", "longlat", "latlong", "ecef"]

    SOURCE_PROJECTION = "SOURCE_PROJECTION"
    SOURCE_UTM = "SOURCE_UTM"
    SOURCE_SP = "SOURCE_SP"

    TARGET_PROJECTION = "TARGET_PROJECTION"
    TARGET_UTM = "TARGET_UTM"
    TARGET_SP = "TARGET_SP"

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('las2las_project')
        self.group, self.i18n_group = self.trAlgorithm('LAStools')
        self.addParametersVerboseGUI()
        self.addParametersPointInputGUI()
        self.addParameter(ParameterSelection(las2las_project.SOURCE_PROJECTION,
            self.tr("source projection"), las2las_project.PROJECTIONS, 0))
        self.addParameter(ParameterSelection(las2las_project.SOURCE_UTM,
            self.tr("source utm zone"), las2las_project.UTM_ZONES, 0))
        self.addParameter(ParameterSelection(las2las_project.SOURCE_SP,
            self.tr("source state plane code"), las2las_project.STATE_PLANES, 0))
        self.addParameter(ParameterSelection(las2las_project.TARGET_PROJECTION,
            self.tr("target projection"), las2las_project.PROJECTIONS, 0))
        self.addParameter(ParameterSelection(las2las_project.TARGET_UTM,
            self.tr("target utm zone"), las2las_project.UTM_ZONES, 0))
        self.addParameter(ParameterSelection(las2las_project.TARGET_SP,
            self.tr("target state plane code"), las2las_project.STATE_PLANES, 0))
        self.addParametersPointOutputGUI()
        self.addParametersAdditionalGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "las2las")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputCommands(commands)
        source_projection = self.getParameterValue(las2las_project.SOURCE_PROJECTION)
        if source_projection != 0:
            if source_projection == 1:
                source_utm_zone = self.getParameterValue(las2las_project.SOURCE_UTM)
                if source_utm_zone != 0:
                    commands.append("-" + las2las_project.PROJECTIONS[source_projection])
                    if source_utm_zone > 60:
                        commands.append(str(source_utm_zone - 60) + "M")
                    else:
                        commands.append(str(source_utm_zone) + "N")
            elif source_projection < 4:
                source_sp_code = self.getParameterValue(las2las_project.SOURCE_SP)
                if source_sp_code != 0:
                    commands.append("-" + las2las_project.PROJECTIONS[source_projection])
                    commands.append(las2las_project.STATE_PLANES[source_sp_code])
            else:
                commands.append("-" + las2las_project.PROJECTIONS[source_projection])
        target_projection = self.getParameterValue(las2las_project.TARGET_PROJECTION)
        if target_projection != 0:
            if target_projection == 1:
                target_utm_zone = self.getParameterValue(las2las_project.TARGET_UTM)
                if target_utm_zone != 0:
                    commands.append("-target_" + las2las_project.PROJECTIONS[target_projection])
                    if target_utm_zone > 60:
                        commands.append(str(target_utm_zone - 60) + "M")
                    else:
                        commands.append(str(target_utm_zone) + "N")
            elif target_projection < 4:
                target_sp_code = self.getParameterValue(las2las_project.TARGET_SP)
                if target_sp_code != 0:
                    commands.append("-target_" + las2las_project.PROJECTIONS[target_projection])
                    commands.append(las2las_project.STATE_PLANES[target_sp_code])
            else:
                commands.append("-target_" + las2las_project.PROJECTIONS[target_projection])
        self.addParametersPointOutputCommands(commands)
        self.addParametersAdditionalCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
