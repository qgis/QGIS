# -*- coding: utf-8 -*-

"""
***************************************************************************
    OutputFactory.py
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

from processing.outputs.OutputHTML import OutputHTML
from processing.outputs.OutputRaster import OutputRaster
from processing.outputs.OutputTable import OutputTable
from processing.outputs.OutputVector import OutputVector
from processing.outputs.OutputNumber import OutputNumber
from processing.outputs.OutputFile import OutputFile
from processing.outputs.OutputString import OutputString

class OutputFactory():

    @staticmethod
    def getFromString(s):
        classes = [OutputRaster, OutputVector, OutputTable, OutputHTML, OutputNumber, OutputFile, OutputString]
        for clazz in classes:
            if s.startswith(clazz().outputTypeName()):
                tokens = s[len(clazz().outputTypeName())+1:].split("|")
                if len(tokens) == 2:
                    return clazz(tokens[0], tokens[1])
                else:
                    return clazz(tokens[0], tokens[1], tokens[2]==str(True))
