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
from qgis.PyQt.QtCore import QCoreApplication

LOG_SEPARATOR = '|~|'


class ProcessingLog:

    DATE_FORMAT = "%Y-%m-%d %H:%M:%S"

    @staticmethod
    def logFilename():
        logFilename = userFolder() + os.sep + 'processing.log'
        if not os.path.isfile(logFilename):
            with codecs.open(logFilename, 'w', encoding='utf-8') as logfile:
                logfile.write('Started logging at ' +
                              datetime.datetime.now().strftime(ProcessingLog.DATE_FORMAT) + '\n')

        return logFilename

    @staticmethod
    def addToLog(msg):
        try:
            # It seems that this fails sometimes depending on the msg
            # added. To avoid it stopping the normal functioning of the
            # algorithm, we catch all errors, assuming that is better
            # to miss some log info than breaking the algorithm.
            line = 'ALGORITHM' + LOG_SEPARATOR + datetime.datetime.now().strftime(
                ProcessingLog.DATE_FORMAT) + LOG_SEPARATOR \
                + msg + '\n'
            with codecs.open(ProcessingLog.logFilename(), 'a',
                             encoding='utf-8') as logfile:
                logfile.write(line)
        except:
            pass

    @staticmethod
    def getLogEntries():
        entries = []

        with open(ProcessingLog.logFilename()) as f:
            lines = f.readlines()
        for line in lines:
            line = line.strip('\n').strip()
            tokens = line.split(LOG_SEPARATOR)
            if len(tokens) <= 1:
                # try old format log separator
                tokens = line.split('|')

            text = ''
            for i in range(2, len(tokens)):
                text += tokens[i] + LOG_SEPARATOR
            if line.startswith('ALGORITHM'):
                entries.append(LogEntry(tokens[1], tokens[2]))

        return entries

    @staticmethod
    def clearLog():
        os.unlink(ProcessingLog.logFilename())

    @staticmethod
    def saveLog(fileName):
        entries = ProcessingLog.getLogEntries()
        with codecs.open(fileName, 'w', encoding='utf-8') as f:
            for entry in entries:
                f.write('ALGORITHM{}{}{}{}\n'.format(LOG_SEPARATOR, entry.date, LOG_SEPARATOR, entry.text))

    @staticmethod
    def tr(string, context=''):
        if context == '':
            context = 'ProcessingLog'
        return QCoreApplication.translate(context, string)


class LogEntry:

    def __init__(self, date, text):
        self.date = date
        self.text = text
