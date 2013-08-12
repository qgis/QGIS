# -*- coding: utf-8 -*-

"""
***************************************************************************
    TableWriter.py
    ---------------------
    Date                 : September 2012
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
__date__ = 'September 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.core import *

from PyQt4.QtCore import *

class TableWriter:

    def __init__(self, fileName, encoding, fields):
        self.fileName = fileName
        self.writer = None

        if encoding is None:
            settings = QSettings()
            encoding = settings.value("/ProcessingQGIS/encoding", "System")

        if not fileName.endswith("csv"):
            fileName += ".csv"
        file = open(fileName, "w")
        file.write(";".join(unicode(field.name()) for field in fields))
        file.write("\n")
        file.close()

    def addRecord(self, values):
        file = open(self.fileName, "a")
        file.write(";".join([unicode(value) for value in values]))
        file.write("\n")
        file.close()
