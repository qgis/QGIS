# -*- coding: utf-8 -*-

"""
***************************************************************************
    ProcessingConfig.py
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

from processing.core.ProcessingUtils import ProcessingUtils
import os.path
from PyQt4 import QtGui

class ProcessingConfig():

    TABLE_LIKE_PARAM_PANEL = "TABLE_LIKE_PARAM_PANEL"
    OUTPUT_FOLDER = "OUTPUT_FOLDER"
    RASTER_STYLE = "RASTER_STYLE"
    VECTOR_POINT_STYLE = "VECTOR_POINT_STYLE"
    VECTOR_LINE_STYLE = "VECTOR_LINE_STYLE"
    VECTOR_POLYGON_STYLE = "VECTOR_POLYGON_STYLE"
    SHOW_RECENT_ALGORITHMS = "SHOW_RECENT_ALGORITHMS"
    USE_SELECTED = "USE_SELECTED"
    USE_FILENAME_AS_LAYER_NAME = "USE_FILENAME_AS_LAYER_NAME"
    KEEP_DIALOG_OPEN = "KEEP_DIALOG_OPEN"
    USE_THREADS = "USE_THREADS"
    SHOW_DEBUG_IN_DIALOG = "SHOW_DEBUG_IN_DIALOG"
    RECENT_ALGORITHMS = "RECENT_ALGORITHMS"
    PRE_EXECUTION_SCRIPT = "PRE_EXECUTION_SCRIPT"
    POST_EXECUTION_SCRIPT = "POST_EXECUTION_SCRIPT"
    SHOW_CRS_DEF = "SHOW_CRS_DEF"
    WARN_UNMATCHING_CRS = "WARN_UNMATCHING_CRS"

    settings = {}
    settingIcons= {}

    @staticmethod
    def initialize():
        icon =  QtGui.QIcon(os.path.dirname(__file__) + "/../images/alg.png")
        ProcessingConfig.settingIcons["General"] = icon
        ProcessingConfig.addSetting(Setting("General", ProcessingConfig.USE_THREADS, "Run algorithms in a new thread", True))
        ProcessingConfig.addSetting(Setting("General", ProcessingConfig.SHOW_DEBUG_IN_DIALOG, "Show extra info in Log panel (threaded execution only)", True))
        ProcessingConfig.addSetting(Setting("General", ProcessingConfig.KEEP_DIALOG_OPEN, "Keep dialog open after running an algorithm", False))
        ProcessingConfig.addSetting(Setting("General", ProcessingConfig.USE_SELECTED, "Use only selected features", True))
        ProcessingConfig.addSetting(Setting("General", ProcessingConfig.TABLE_LIKE_PARAM_PANEL, "Show table-like parameter panels", False))
        ProcessingConfig.addSetting(Setting("General", ProcessingConfig.USE_FILENAME_AS_LAYER_NAME, "Use filename as layer name", True))
        ProcessingConfig.addSetting(Setting("General", ProcessingConfig.SHOW_RECENT_ALGORITHMS, "Show recently executed algorithms", True))
        ProcessingConfig.addSetting(Setting("General", ProcessingConfig.OUTPUT_FOLDER, "Output folder", ProcessingUtils.tempFolder()))
        ProcessingConfig.addSetting(Setting("General", ProcessingConfig.SHOW_CRS_DEF, "Show layer CRS definition in selection boxes", True))
        ProcessingConfig.addSetting(Setting("General", ProcessingConfig.WARN_UNMATCHING_CRS, "Warn before executing if layer CRS's do not match", True))
        ProcessingConfig.addSetting(Setting("General", ProcessingConfig.RASTER_STYLE,"Style for raster layers",""))
        ProcessingConfig.addSetting(Setting("General", ProcessingConfig.VECTOR_POINT_STYLE,"Style for point layers",""))
        ProcessingConfig.addSetting(Setting("General", ProcessingConfig.VECTOR_LINE_STYLE,"Style for line layers",""))
        ProcessingConfig.addSetting(Setting("General", ProcessingConfig.VECTOR_POLYGON_STYLE,"Style for polygon layers",""))
        ProcessingConfig.addSetting(Setting("General", ProcessingConfig.VECTOR_POLYGON_STYLE,"Style for polygon layers",""))
        ProcessingConfig.addSetting(Setting("General", ProcessingConfig.PRE_EXECUTION_SCRIPT,"Pre-execution script",""))
        ProcessingConfig.addSetting(Setting("General", ProcessingConfig.POST_EXECUTION_SCRIPT,"Post-execution script",""))
        ProcessingConfig.addSetting(Setting("General", ProcessingConfig.RECENT_ALGORITHMS,"Recent algs","", hidden=True))


    @staticmethod
    def setGroupIcon(group, icon):
        ProcessingConfig.settingIcons[group] = icon

    @staticmethod
    def getGroupIcon(group):
        if group == "General":
            return QtGui.QIcon(os.path.dirname(__file__) + "/../images/alg.png")
        if group in ProcessingConfig.settingIcons:
            return ProcessingConfig.settingIcons[group]
        else:
            return QtGui.QIcon(os.path.dirname(__file__) + "/../images/alg.png")

    @staticmethod
    def addSetting(setting):
        ProcessingConfig.settings[setting.name] = setting

    @staticmethod
    def removeSetting(name):
        del ProcessingConfig.settings[name]

    @staticmethod
    def getSettings():
        settings={}
        for setting in ProcessingConfig.settings.values():
            if not setting.group in settings:
                group = []
                settings[setting.group] = group
            else:
                group = settings[setting.group]
            group.append(setting)
        return settings

    @staticmethod
    def configFile():
        return os.path.join(ProcessingUtils.userFolder(), "processing_qgis.conf")

    @staticmethod
    def loadSettings():
        if not os.path.isfile(ProcessingConfig.configFile()):
            return
        lines = open(ProcessingConfig.configFile())
        line = lines.readline().strip("\n")
        while line != "":
            tokens = line.split("=")
            if tokens[0] in ProcessingConfig.settings.keys():
                setting = ProcessingConfig.settings[tokens[0]]
                if isinstance(setting.value, bool):
                    setting.value = (tokens[1].strip() == str(True))
                else:
                    setting.value = tokens[1]
                ProcessingConfig.addSetting(setting)
            line = lines.readline().strip("\n")
        lines.close()

    @staticmethod
    def saveSettings():
        fout = open(ProcessingConfig.configFile(), "w")
        for setting in ProcessingConfig.settings.values():
            fout.write(str(setting) + "\n")
        fout.close()

    @staticmethod
    def getSetting(name):
        if name in ProcessingConfig.settings.keys():
            return ProcessingConfig.settings[name].value
        else:
            return None

    @staticmethod
    def setSettingValue(name, value):
        if name in ProcessingConfig.settings.keys():
            ProcessingConfig.settings[name].value = value
            ProcessingConfig.saveSettings()


class Setting():
    '''A simple config parameter that will appear on the config dialog'''
    def __init__(self, group, name, description, default, hidden = False):
        self.group=group
        self.name = name
        self.description = description
        self.default = default
        self.value = default
        self.hidden = hidden

    def __str__(self):
        return self.name + "=" + str(self.value)
