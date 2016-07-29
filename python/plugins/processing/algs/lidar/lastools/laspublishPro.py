# -*- coding: utf-8 -*-

"""
***************************************************************************
    laspublishPro.py
    ---------------------
    Date                 : May 2016
    Copyright            : (C) 2016 by Martin Isenburg
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
__date__ = 'May 2016'
__copyright__ = '(C) 2016, Martin Isenburg'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from LAStoolsUtils import LAStoolsUtils
from LAStoolsAlgorithm import LAStoolsAlgorithm

from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterFile


class laspublishPro(LAStoolsAlgorithm):

    MODE = "MODE"
    MODES = ["3D only", "3D + download map", "download map only"]
    DIR = "DIR"
    SHOW_SKYBOX = "SHOW_SKYBOX"
    USE_EDL = "USE_EDL"
    MATERIAL = "MATERIAL"
    MATERIALS = ["elevation", "intensity", "return_number", "point_source", "rgb"]
    COPY_OR_MOVE = "COPY_OR_MOVE"
    COPY_OR_MOVE_OPTIONS = ["copy into portal dir", "move into portal dir", "neither"]
    PORTAL_DIRECTORY = "PORTAL_DIRECTORY"
    PORTAL_HTML_PAGE = "PORTAL_HTML_PAGE"
    OVERWRITE_EXISTING = "OVERWRITE_EXISTING"
    PORTAL_TITLE = "PORTAL_TITLE"
    PORTAL_DESCRIPTION = "PORTAL_DESCRIPTION"

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('laspublishPro')
        self.group, self.i18n_group = self.trAlgorithm('LAStools Production')
        self.addParametersVerboseGUI()
        self.addParametersPointInputFolderGUI()
        self.addParameter(ParameterSelection(laspublishPro.MODE,
                                             self.tr("type of portal"), laspublishPro.MODES, 1))
        self.addParameter(ParameterBoolean(laspublishPro.USE_EDL,
                                           self.tr("use Eye Dome Lighting (EDL)"), True))
        self.addParameter(ParameterBoolean(laspublishPro.SHOW_SKYBOX,
                                           self.tr("show Skybox"), True))
        self.addParameter(ParameterSelection(laspublishPro.MATERIAL,
                                             self.tr("default material colors on start-up"), laspublishPro.MATERIALS, 0))
        self.addParameter(ParameterFile(laspublishPro.PORTAL_DIRECTORY,
                                        self.tr("portal output directory"), True, False))
        self.addParameter(ParameterSelection(laspublishPro.COPY_OR_MOVE,
                                             self.tr("copy or move source LiDAR files into portal (only for download portals)"), laspublishPro.COPY_OR_MOVE_OPTIONS, 2))
        self.addParameter(ParameterBoolean(laspublishPro.OVERWRITE_EXISTING,
                                           self.tr("overwrite existing files"), True))
        self.addParameter(ParameterString(laspublishPro.PORTAL_HTML_PAGE,
                                          self.tr("portal HTML page"), "portal.html", False))
        self.addParameter(ParameterString(laspublishPro.PORTAL_TITLE,
                                          self.tr("portal title"), "My LiDAR Portal"))
        self.addParameter(ParameterString(laspublishPro.PORTAL_DESCRIPTION,
                                          self.tr("portal description")))
        self.addParametersAdditionalGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "laspublish")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputFolderCommands(commands)
        mode = self.getParameterValue(laspublishPro.MODE)
        if (mode == 0):
            commands.append("-only_3D")
        elif (mode == 2):
            commands.append("-only_2D")
        material = self.getParameterValue(laspublishPro.MATERIAL)
        commands.append("-" + laspublishPro.MATERIALS[material])
        if not self.getParameterValue(laspublishPro.USE_EDL):
            commands.append("-no_edl")
        if not self.getParameterValue(laspublishPro.SHOW_SKYBOX):
            commands.append("-no_skybox")
        portal_directory = self.getParameterValue(laspublishPro.PORTAL_DIRECTORY)
        if portal_directory != "":
            commands.append("-odir")
            commands.append('"' + portal_directory + '"')
        copy_or_move = self.getParameterValue(laspublishPro.COPY_OR_MOVE)
        if (copy_or_move == 0):
            commands.append("-copy_source_files")
        elif (copy_or_move == 1):
            commands.append("-move_source_files")
            commands.append("-really_move")
        if self.getParameterValue(laspublishPro.OVERWRITE_EXISTING):
            commands.append("-overwrite")
        portal_html_page = self.getParameterValue(laspublishPro.PORTAL_HTML_PAGE)
        if portal_html_page != "":
            commands.append("-o")
            commands.append('"' + portal_html_page + '"')
        title = self.getParameterValue(laspublishPro.PORTAL_TITLE)
        if title != "":
            commands.append("-title")
            commands.append('"' + title + '"')
        description = self.getParameterValue(laspublishPro.PORTAL_DESCRIPTION)
        if description != "":
            commands.append("-description")
            commands.append('"' + description + '"')
        commands.append("-olaz")
        self.addParametersAdditionalCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
