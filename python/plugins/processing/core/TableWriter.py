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

import csv
import codecs
import cStringIO

class TableWriter:
    def __init__(self, fileName, encoding, fields):
        self.fileName = fileName
        if not self.fileName.lower().endswith("csv"):
            self.fileName += ".csv"

        self.encoding = encoding
        if self.encoding is None or encoding == "System":
            self.encoding = "utf-8"

        with open(self.fileName, "wb") as csvFile:
            self.writer = UnicodeWriter(csvFile, encoding=self.encoding)
            self.writer.writerow(fields)

    def addRecord(self, values):
        with open(self.fileName, "ab") as csvFile:
            self.writer = UnicodeWriter(csvFile, encoding=self.encoding)
            self.writer.writerow(values)

    def addRecords(self, records):
        with open(self.fileName, "ab") as csvFile:
            self.writer = UnicodeWriter(csvFile, encoding=self.encoding)
            self.writer.writerows(records)

class UnicodeWriter:
    def __init__(self, f, dialect=csv.excel, encoding="utf-8", **kwds):
        self.queue = cStringIO.StringIO()
        self.writer = csv.writer(self.queue, dialect=dialect, **kwds)
        self.stream = f
        self.encoder = codecs.getincrementalencoder(encoding)()

    def writerow(self, row):
        row = map(unicode, row)
        try:
            self.writer.writerow([s.encode("utf-8") for s in row])
        except:
            self.writer.writerow(row)
        data = self.queue.getvalue()
        data = data.decode("utf-8")
        data = self.encoder.encode(data)
        self.stream.write(data)
        self.queue.truncate(0)

    def writerows(self, rows):
        for row in rows:
            self.writerow(row)
