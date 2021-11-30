# -*- coding: utf-8 -*-

"""
***************************************************************************
    Output.py
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

import sys

from qgis.core import (QgsExpressionContext,
                       QgsExpressionContextUtils,
                       QgsExpression,
                       QgsExpressionContextScope,
                       QgsProject,
                       QgsSettings,
                       QgsVectorFileWriter,
                       QgsProcessingUtils,
                       QgsProcessingParameterDefinition,
                       QgsProcessingOutputRasterLayer,
                       QgsProcessingOutputVectorLayer,
                       QgsProcessingOutputMapLayer,
                       QgsProcessingOutputHtml,
                       QgsProcessingOutputNumber,
                       QgsProcessingOutputString,
                       QgsProcessingOutputBoolean,
                       QgsProcessingOutputFolder,
                       QgsProcessingOutputMultipleLayers,
                       QgsProcessingOutputPointCloudLayer)


def getOutputFromString(s):
    try:
        if "|" in s and s.startswith("Output"):
            tokens = s.split("|")
            params = [t if str(t) != "None" else None for t in tokens[1:]]
            clazz = getattr(sys.modules[__name__], tokens[0])
            return clazz(*params)
        else:
            tokens = s.split("=")
            if tokens[1].lower()[: len('output')] != 'output':
                return None

            name = tokens[0]
            description = tokens[0]

            token = tokens[1].strip()[len('output') + 1:]
            out = None

            if token.lower().strip().startswith('outputraster'):
                out = QgsProcessingOutputRasterLayer(name, description)
            elif token.lower().strip() == 'outputvector':
                out = QgsProcessingOutputVectorLayer(name, description)
            elif token.lower().strip() == 'outputlayer':
                out = QgsProcessingOutputMapLayer(name, description)
            elif token.lower().strip() == 'outputmultilayers':
                out = QgsProcessingOutputMultipleLayers(name, description)
            #            elif token.lower().strip() == 'vector point':
            #                out = OutputVector(datatype=[dataobjects.TYPE_VECTOR_POINT])
            #            elif token.lower().strip() == 'vector line':
            #                out = OutputVector(datatype=[OutputVector.TYPE_VECTOR_LINE])
            #            elif token.lower().strip() == 'vector polygon':
            #                out = OutputVector(datatype=[OutputVector.TYPE_VECTOR_POLYGON])
            #            elif token.lower().strip().startswith('table'):
            #                out = OutputTable()
            elif token.lower().strip().startswith('outputhtml'):
                out = QgsProcessingOutputHtml(name, description)
            #            elif token.lower().strip().startswith('file'):
            #                out = OutputFile()
            #                ext = token.strip()[len('file') + 1:]
            #                if ext:
            #                    out.ext = ext
            elif token.lower().strip().startswith('outputfolder'):
                out = QgsProcessingOutputFolder(name, description)
            elif token.lower().strip().startswith('outputnumber'):
                out = QgsProcessingOutputNumber(name, description)
            elif token.lower().strip().startswith('outputstring'):
                out = QgsProcessingOutputString(name, description)
            elif token.lower().strip().startswith('outputboolean'):
                out = QgsProcessingOutputBoolean(name, description)
            elif token.lower().strip().startswith('outputPointCloud'):
                out = QgsProcessingOutputPointCloudLayer(name, description)
            #            elif token.lower().strip().startswith('extent'):
            #                out = OutputExtent()

            return out
    except:
        return None
