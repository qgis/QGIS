# -*- coding: utf-8 -*-

"""
***************************************************************************
    OtbUtils.py
    -----------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
                           (C) 2013 by CS Systemes d'information (CS SI)
    Email                : volayaf at gmail dot com
                           otb at c-s dot fr (CS SI)
    Contributors         : Victor Olaya
                           Julien Malik, Oscar Picas  (CS SI) - add functions to manage xml tree
                           Alexia Mondot (CS SI) - add a trick for OTBApplication SplitImages
                           Rashad Kanavath (CS SI) - re-integration of provider to QGIS
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""
from builtins import str

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
import re
import subprocess

from processing.core.ProcessingConfig import ProcessingConfig
from qgis.core import (Qgis, QgsApplication, QgsMessageLog)
from qgis.PyQt.QtCore import QCoreApplication
from processing.algs.otb.OtbSettings import OtbSettings


def cliPath():
    cli_ext = '.bat' if os.name == 'nt' else ''
    return os.path.normpath(os.path.join(QgsApplication.qgisSettingsDirPath(),
                                         'processing', 'qgis_otb_cli' + cli_ext))


def version():
    return ProcessingConfig.getSetting(OtbSettings.VERSION) or '0.0.0'


def loggerLevel():
    return ProcessingConfig.getSetting(OtbSettings.LOGGER_LEVEL) or 'INFO'


def maxRAMHint():
    return ProcessingConfig.getSetting(OtbSettings.MAX_RAM_HINT) or ''


def otbFolder():
    if ProcessingConfig.getSetting(OtbSettings.FOLDER):
        return os.path.normpath(os.sep.join(re.split(r'\\|/', ProcessingConfig.getSetting(OtbSettings.FOLDER))))
    else:
        return None


def appFolder():
    app_folder = ProcessingConfig.getSetting(OtbSettings.APP_FOLDER)
    if app_folder:
        return os.pathsep.join(app_folder.split(';'))
    else:
        return None


def srtmFolder():
    return ProcessingConfig.getSetting(OtbSettings.SRTM_FOLDER) or ''


def geoidFile():
    return ProcessingConfig.getSetting(OtbSettings.GEOID_FILE) or ''


def executeOtb(command, feedback, addToLog=True):
    loglines = []
    feedback.setProgress(0)
    with subprocess.Popen(
            [command],
            shell=True,
            stdout=subprocess.PIPE,
            stdin=subprocess.DEVNULL,
            stderr=subprocess.STDOUT,
            universal_newlines=True
    ) as proc:
        try:
            for line in iter(proc.stdout.readline, ''):
                line = line.strip()
                #'* ]' and '  ]' says its some progress update
                #print('line[-3:]',line[-3:])
                if line[-3:] == "* ]" or line[-3:] == "  ]":
                    part = line.split(':')[1]
                    percent = part.split('%')[0]
                    try:
                        if int(percent) >= 100:
                            loglines.append(line)
                        feedback.setProgress(int(percent))
                    except:
                        pass
                else:
                    loglines.append(line)
        except BaseException as e:
            loglines.append(str(e))
            pass

        for logline in loglines:
            if feedback is None:
                QgsMessageLog.logMessage(logline, 'Processing', Qgis.Info)
            else:
                feedback.pushConsoleInfo(logline)

        # for logline in loglines:
        #     if 'INFO' in logline or 'FATAL' in logline:
        #         if feedback is None:
        #             QgsMessageLog.logMessage(logline, 'Processing', Qgis.Info)
        #         else:
        #             feedback.pushConsoleInfo(logline)


def tr(string, context=''):
    if context == '':
        context = 'OtbUtils'
    return QCoreApplication.translate(context, string)
