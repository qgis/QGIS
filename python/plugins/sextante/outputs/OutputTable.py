# -*- coding: utf-8 -*-

"""
***************************************************************************
    OutputTable.py
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
from sextante.core.SextanteTableWriter import SextanteTableWriter

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from sextante.outputs.Output import Output

from PyQt4.QtCore import *

class OutputTable(Output):

    def getFileFilter(self,alg):
        exts = alg.provider.getSupportedOutputTableExtensions()
        for i in range(len(exts)):
            exts[i] = exts[i].upper() + " files(*." + exts[i].lower() + ")"
        return ";;".join(exts)

    def getDefaultFileExtension(self, alg):
        return alg.provider.getSupportedOutputTableExtensions()[0]
    
    def getTableWriter(self, fields):
        '''Returns a suitable writer to which records can be added as a
        result of the algorithm. Use this to transparently handle output
        values instead of creating your own method.

        @param fields   a dict of int-QgsField
        
        @return writer  instance of the table writer class
        '''

        if self.encoding is None:
            settings = QSettings()
            self.encoding = settings.value("/SextanteQGIS/encoding", "System").toString()

        return SextanteTableWriter(self.value, self.encoding, fields)        