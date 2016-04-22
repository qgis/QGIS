# -*- coding: utf-8 -*-

"""
***************************************************************************
    ProcessingLog.py
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
import codecs
import datetime
from processing.tools.system import userFolder
from processing.core.ProcessingConfig import ProcessingConfig
from qgis.core import QgsMessageLog
from qgis.PyQt.QtCore import QCoreApplication


class ProcessingLog:

    LOG_ERROR = 'ERROR'
    LOG_INFO = 'INFO'
    LOG_WARNING = 'WARNING'
    LOG_ALGORITHM = 'ALGORITHM'
    DATE_FORMAT = "%Y-%m-%d %H:%M:%S"
    recentAlgs = []

    @staticmethod
    def logFilename():
        logFilename = userFolder() + os.sep + 'processing.log'
        if not os.path.isfile(logFilename):
            logfile = codecs.open(logFilename, 'w', encoding='utf-8')
            logfile.write('Started logging at ' +
                          datetime.datetime.now().strftime(ProcessingLog.DATE_FORMAT) + '\n')
            logfile.close()

        return logFilename

    @staticmethod
    def addToLog(msgtype, msg):
        try:
            # It seems that this fails sometimes depending on the msg
            # added. To avoid it stopping the normal functioning of the
            # algorithm, we catch all errors, assuming that is better
            # to miss some log info than breaking the algorithm.
            if msgtype == ProcessingLog.LOG_ALGORITHM:
                line = msgtype + '|' + datetime.datetime.now().strftime(
                    ProcessingLog.DATE_FORMAT) + '|' \
                    + msg + '\n'
                logfile = codecs.open(ProcessingLog.logFilename(), 'a',
                                      encoding='utf-8')
                logfile.write(line)
                logfile.close()
                algname = msg[len('Processing.runalg("'):]
                algname = algname[:algname.index('"')]
                if algname not in ProcessingLog.recentAlgs:
                    ProcessingLog.recentAlgs.append(algname)
                    recentAlgsString = ';'.join(ProcessingLog.recentAlgs[-6:])
                    ProcessingConfig.setSettingValue(
                        ProcessingConfig.RECENT_ALGORITHMS,
                        recentAlgsString)
            else:
                if isinstance(msg, list):
                    msg = '\n'.join([m for m in msg])
                msgtypes = {ProcessingLog.LOG_ERROR: QgsMessageLog.CRITICAL,
                            ProcessingLog.LOG_INFO: QgsMessageLog.INFO,
                            ProcessingLog.LOG_WARNING: QgsMessageLog.WARNING, }
                QgsMessageLog.logMessage(msg, ProcessingLog.tr("Processing"), msgtypes[msgtype])
        except:
            pass

    @staticmethod
    def getLogEntries():
        entries = {}
        errors = []
        algorithms = []
        warnings = []
        info = []

        with open(ProcessingLog.logFilename()) as f:
            lines = f.readlines()
        for line in lines:
            line = line.strip('\n').strip()
            tokens = line.split('|')
            text = ''
            for i in range(2, len(tokens)):
                text += tokens[i] + '|'
            if line.startswith(ProcessingLog.LOG_ERROR):
                errors.append(LogEntry(tokens[1], text))
            elif line.startswith(ProcessingLog.LOG_ALGORITHM):
                algorithms.append(LogEntry(tokens[1], tokens[2]))
            elif line.startswith(ProcessingLog.LOG_WARNING):
                warnings.append(LogEntry(tokens[1], text))
            elif line.startswith(ProcessingLog.LOG_INFO):
                info.append(LogEntry(tokens[1], text))

        entries[ProcessingLog.LOG_ALGORITHM] = algorithms
        return entries

    @staticmethod
    def getRecentAlgorithms():
        recentAlgsSetting = ProcessingConfig.getSetting(
            ProcessingConfig.RECENT_ALGORITHMS)
        try:
            ProcessingLog.recentAlgs = recentAlgsSetting.split(';')
        except:
            pass
        return ProcessingLog.recentAlgs

    @staticmethod
    def clearLog():
        os.unlink(ProcessingLog.logFilename())

    @staticmethod
    def saveLog(fileName):
        entries = ProcessingLog.getLogEntries()
        with codecs.open(fileName, 'w', encoding='utf-8') as f:
            for k, v in entries.iteritems():
                for entry in v:
                    f.write('%s|%s|%s\n' % (k, entry.date, entry.text))

    @staticmethod
    def tr(string, context=''):
        if context == '':
            context = 'ProcessingLog'
        return QCoreApplication.translate(context, string)


class LogEntry:

    def __init__(self, date, text):
        self.date = date
        self.text = text
