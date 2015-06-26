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

import os

from PyQt4.QtCore import QPyNullVariant, QCoreApplication, QSettings
from PyQt4.QtGui import QIcon
from processing.tools.system import tempFolder


class ProcessingConfig:

    OUTPUT_FOLDER = 'OUTPUT_FOLDER'
    RASTER_STYLE = 'RASTER_STYLE'
    VECTOR_POINT_STYLE = 'VECTOR_POINT_STYLE'
    VECTOR_LINE_STYLE = 'VECTOR_LINE_STYLE'
    VECTOR_POLYGON_STYLE = 'VECTOR_POLYGON_STYLE'
    SHOW_RECENT_ALGORITHMS = 'SHOW_RECENT_ALGORITHMS'
    USE_SELECTED = 'USE_SELECTED'
    USE_FILENAME_AS_LAYER_NAME = 'USE_FILENAME_AS_LAYER_NAME'
    KEEP_DIALOG_OPEN = 'KEEP_DIALOG_OPEN'
    SHOW_DEBUG_IN_DIALOG = 'SHOW_DEBUG_IN_DIALOG'
    RECENT_ALGORITHMS = 'RECENT_ALGORITHMS'
    PRE_EXECUTION_SCRIPT = 'PRE_EXECUTION_SCRIPT'
    POST_EXECUTION_SCRIPT = 'POST_EXECUTION_SCRIPT'
    SHOW_CRS_DEF = 'SHOW_CRS_DEF'
    WARN_UNMATCHING_CRS = 'WARN_UNMATCHING_CRS'

    settings = {}
    settingIcons = {}

    @staticmethod
    def initialize():
        icon = QIcon(os.path.dirname(__file__) + '/../images/alg.png')
        ProcessingConfig.settingIcons['General'] = icon
        ProcessingConfig.addSetting( Setting(
            ProcessingConfig.tr('General'),
            ProcessingConfig.SHOW_DEBUG_IN_DIALOG,
            ProcessingConfig.tr('Show extra info in Log panel'), True))
        ProcessingConfig.addSetting(Setting(
            ProcessingConfig.tr('General'),
            ProcessingConfig.KEEP_DIALOG_OPEN,
            ProcessingConfig.tr('Keep dialog open after running an algorithm'), False))
        ProcessingConfig.addSetting(Setting(
            ProcessingConfig.tr('General'),
            ProcessingConfig.USE_SELECTED,
            ProcessingConfig.tr('Use only selected features'), True))
        ProcessingConfig.addSetting(Setting(
            ProcessingConfig.tr('General'),
            ProcessingConfig.USE_FILENAME_AS_LAYER_NAME,
            ProcessingConfig.tr('Use filename as layer name'), False))
        ProcessingConfig.addSetting(Setting(
            ProcessingConfig.tr('General'),
            ProcessingConfig.SHOW_RECENT_ALGORITHMS,
            ProcessingConfig.tr('Show recently executed algorithms'), True))
        ProcessingConfig.addSetting(Setting(
            ProcessingConfig.tr('General'),
            ProcessingConfig.OUTPUT_FOLDER,
            ProcessingConfig.tr('Output folder'), tempFolder(),
            valuetype=Setting.FOLDER))
        ProcessingConfig.addSetting(Setting(
            ProcessingConfig.tr('General'),
            ProcessingConfig.SHOW_CRS_DEF,
            ProcessingConfig.tr('Show layer CRS definition in selection boxes'), True))
        ProcessingConfig.addSetting(Setting(
            ProcessingConfig.tr('General'),
            ProcessingConfig.WARN_UNMATCHING_CRS,
            ProcessingConfig.tr("Warn before executing if layer CRS's do not match"), True))
        ProcessingConfig.addSetting(Setting(
            ProcessingConfig.tr('General'),
            ProcessingConfig.RASTER_STYLE,
            ProcessingConfig.tr('Style for raster layers'), ''))
        ProcessingConfig.addSetting(Setting(
            ProcessingConfig.tr('General'),
            ProcessingConfig.VECTOR_POINT_STYLE,
            ProcessingConfig.tr('Style for point layers'), ''))
        ProcessingConfig.addSetting(Setting(
            ProcessingConfig.tr('General'),
            ProcessingConfig.VECTOR_LINE_STYLE,
            ProcessingConfig.tr('Style for line layers'), ''))
        ProcessingConfig.addSetting(Setting(
            ProcessingConfig.tr('General'),
            ProcessingConfig.VECTOR_POLYGON_STYLE,
            ProcessingConfig.tr('Style for polygon layers'), ''))
        ProcessingConfig.addSetting(Setting(
            ProcessingConfig.tr('General'),
            ProcessingConfig.VECTOR_POLYGON_STYLE,
            ProcessingConfig.tr('Style for polygon layers'), ''))
        ProcessingConfig.addSetting(Setting(
            ProcessingConfig.tr('General'),
            ProcessingConfig.PRE_EXECUTION_SCRIPT,
            ProcessingConfig.tr('Pre-execution script'), ''))
        ProcessingConfig.addSetting(Setting(
            ProcessingConfig.tr('General'),
            ProcessingConfig.POST_EXECUTION_SCRIPT,
            ProcessingConfig.tr('Post-execution script'), ''))
        ProcessingConfig.addSetting(Setting(
            ProcessingConfig.tr('General'),
            ProcessingConfig.RECENT_ALGORITHMS,
            ProcessingConfig.tr('Recent algs'), '', hidden=True))

    @staticmethod
    def setGroupIcon(group, icon):
        ProcessingConfig.settingIcons[group] = icon

    @staticmethod
    def getGroupIcon(group):
        if group == ProcessingConfig.tr('General'):
            return QIcon(os.path.dirname(__file__) + '/../images/alg.png')
        if group in ProcessingConfig.settingIcons:
            return ProcessingConfig.settingIcons[group]
        else:
            return QIcon( os.path.dirname(__file__) + '/../images/alg.png')

    @staticmethod
    def addSetting(setting):
        ProcessingConfig.settings[setting.name] = setting

    @staticmethod
    def removeSetting(name):
        del ProcessingConfig.settings[name]

    @staticmethod
    def getSettings():
        '''Return settings as a dict with group names as keys and lists of settings as values'''
        settings = {}
        for setting in ProcessingConfig.settings.values():
            if setting.group not in settings:
                group = []
                settings[setting.group] = group
            else:
                group = settings[setting.group]
            group.append(setting)
        return settings


    @staticmethod
    def readSettings():
        for setting in ProcessingConfig.settings.values():
            setting.read()


    @staticmethod
    def getSetting(name):
        if name in ProcessingConfig.settings.keys():
            v = ProcessingConfig.settings[name].value
            if isinstance(v, QPyNullVariant):
                v = None
            return v
        else:
            return None

    @staticmethod
    def setSettingValue(name, value):
        if name in ProcessingConfig.settings.keys():
            ProcessingConfig.settings[name].value = value
            ProcessingConfig.settings[name].save()

    @staticmethod
    def tr(string, context=''):
        if context == '':
            context = 'ProcessingConfig'
        return QCoreApplication.translate(context, string)


class Setting:
    """A simple config parameter that will appear on the config dialog.
    """
    STRING = 0
    FILE = 1
    FOLDER = 2

    def __init__(self, group, name, description, default, hidden=False, valuetype=None):
        self.group = group
        self.name = name
        self.qname = "Processing/Configuration/" + self.name
        self.description = description
        self.default = default
        self.value = default
        self.hidden = hidden
        self.valuetype = valuetype

    def read(self):
        qsettings = QSettings()
        value = qsettings.value(self.qname, None)
        if value is not None:
            if isinstance(self.value, bool):
                value = str(value).lower() == str(True).lower()
            self.value = value

    def save(self):
        QSettings().setValue(self.qname, self.value)

    def __str__(self):
        return self.name + '=' + str(self.value)
