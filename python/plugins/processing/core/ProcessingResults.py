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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

class ProcessingResults():

    results = []

    @staticmethod
    def addResult(name, result):
        ProcessingResults.results.append(Result(name, result))

    @staticmethod
    def getResults():
        return ProcessingResults.results


class Result():

    def __init__(self, name, filename):
        self.name = name
        self.filename = filename





