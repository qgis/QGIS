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

import re
import os
import codecs
import datetime
from processing.tools.system import userFolder
from processing.core.ProcessingConfig import ProcessingConfig
from qgis.core import *

class ProcessingLog:

    LOG_ERROR = 'ERROR'
    LOG_INFO = 'INFO'
    LOG_WARNING = 'WARNING'
    LOG_ALGORITHM = 'ALGORITHM'
    DATE_FORMAT = u'%a %b %d %Y %H:%M:%S'.encode('utf-8')
    recentAlgs = []

    @staticmethod
    def startLogging():
        if os.path.isfile(ProcessingLog.logFilename()):
            logfile = codecs.open(ProcessingLog.logFilename(), 'a',
                                  encoding='utf-8')
        else:
            logfile = codecs.open(ProcessingLog.logFilename(), 'w',
                                  encoding='utf-8')
        logfile.write('Started logging at ' +
                      datetime.datetime.now().strftime(
                          ProcessingLog.DATE_FORMAT).decode('utf-8') + '\n')
        logfile.close()

    @staticmethod
    def logFilename():
        batchfile = userFolder() + os.sep + 'processing.log'
        return batchfile

    @staticmethod
    def addToLog(msgtype, msg):
        try:
            # It seems that this fails sometimes depending on the msg
            # added. To avoid it stopping the normal functioning of the
            # algorithm, we catch all errors, assuming that is better
            # to miss some log info that breaking the algorithm.
            if msgtype == ProcessingLog.LOG_ALGORITHM:
                line = msgtype + '|' + datetime.datetime.now().strftime(
                        ProcessingLog.DATE_FORMAT).decode('utf-8') + '|' \
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
                            ProcessingLog.LOG_WARNING: QgsMessageLog.WARNING,}
                QgsMessageLog.logMessage(msg, "Processing", msgtypes[msgtype])
        except:
            pass

    @staticmethod
    def getLogEntries():
        entries = {}
        errors = []
        algorithms = []
        warnings = []
        info = []
        lines = tail(ProcessingLog.logFilename())
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
        ProcessingLog.startLogging()

    @staticmethod
    def saveLog(fileName):
        entries = ProcessingLog.getLogEntries()
        with codecs.open(fileName, 'w', encoding='utf-8') as f:
            for k, v in entries.iteritems():
                for entry in v:
                    f.write('%s|%s|%s\n' % (k, entry.date, entry.text))


class LogEntry:

    def __init__(self, date, text):
        self.date = date
        self.text = text

"""
***************************************************************************
    This code has been take from pytailer
    http://code.google.com/p/pytailer/

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use, copy,
    modify, merge, publish, distribute, sublicense, and/or sell copies
    of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
    BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
    ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
***************************************************************************
"""


class Tailer(object):
    """Implements tailing and heading functionality like GNU tail and
    head commands.
    """
    line_terminators = ('\r\n', '\n', '\r')

    def __init__(self, filename, read_size=1024, end=False):
        self.read_size = read_size
        self.file = codecs.open(filename, encoding='utf-8')
        self.start_pos = self.file.tell()
        if end:
            self.seek_end()

    def splitlines(self, data):
        return re.split('|'.join(self.line_terminators), data)

    def seek_end(self):
        self.seek(0, 2)

    def seek(self, pos, whence=0):
        self.file.seek(pos, whence)

    def read(self, read_size=None):
        if read_size:
            read_str = self.file.read(read_size)
        else:
            read_str = self.file.read()

        return (len(read_str), read_str)

    def seek_line(self):
        """Searches backwards from the current file position for a
        line terminator and seeks to the charachter after it.
        """
        pos = end_pos = self.file.tell()

        read_size = self.read_size
        if pos > read_size:
            pos -= read_size
        else:
            pos = 0
            read_size = end_pos

        self.seek(pos)

        (bytes_read, read_str) = self.read(read_size)

        if bytes_read and read_str[-1] in self.line_terminators:
            # The last charachter is a line terminator, don't count
            # this one.
            bytes_read -= 1

            if read_str[-2:] == '\r\n' and '\r\n' in self.line_terminators:
                # found CRLF
                bytes_read -= 1

        while bytes_read > 0:
            # Scan backward, counting the newlines in this bufferfull
            i = bytes_read - 1
            while i >= 0:
                if read_str[i] in self.line_terminators:
                    self.seek(pos + i + 1)
                    return self.file.tell()
                i -= 1

            if pos == 0 or pos - self.read_size < 0:
                # Not enought lines in the buffer, send the whole file
                self.seek(0)
                return None

            pos -= self.read_size
            self.seek(pos)

            (bytes_read, read_str) = self.read(self.read_size)

        return None

    def tail(self, lines=10):
        """Return the last lines of the file.
        """
        self.seek_end()
        end_pos = self.file.tell()

        for i in xrange(lines):
            if not self.seek_line():
                break

        data = self.file.read(end_pos - self.file.tell() - 1)
        if data:
            return self.splitlines(data)
        else:
            return []

    def __iter__(self):
        return self.follow()

    def close(self):
        self.file.close()


def tail(file, lines=200):
    return Tailer(file).tail(lines)
