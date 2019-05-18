# -*- coding: utf-8 -*-

"""
***************************************************************************
    ProcessingResults.py
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

from qgis.PyQt.QtCore import QObject, pyqtSignal


class ProcessingResults(QObject):

    resultAdded = pyqtSignal()

    results = []

    def addResult(self, icon, name, timestamp, result):
        self.results.append(Result(icon, name, timestamp, result))
        self.resultAdded.emit()

    def getResults(self):
        return self.results


class Result:

    def __init__(self, icon, name, timestamp, filename):
        self.icon = icon
        self.name = name
        self.timestamp = timestamp
        self.filename = filename


resultsList = ProcessingResults()
