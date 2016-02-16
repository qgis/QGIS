# -*- coding: utf-8 -*-

"""
***************************************************************************
    RenderingStyles.py
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
from processing.tools.system import userFolder


class RenderingStyles:

    styles = {}

    @staticmethod
    def addAlgStylesAndSave(algname, styles):
        RenderingStyles.styles[algname] = styles
        RenderingStyles.saveSettings()

    @staticmethod
    def configFile():
        return os.path.join(userFolder(), 'processing_qgis_styles.conf')

    @staticmethod
    def loadStyles():
        if not os.path.isfile(RenderingStyles.configFile()):
            return
        lines = open(RenderingStyles.configFile())
        line = lines.readline().strip('\n')
        while line != '':
            tokens = line.split('|')
            if tokens[0] in RenderingStyles.styles.keys():
                RenderingStyles.styles[tokens[0]][tokens[1]] = tokens[2]
            else:
                alg = {}
                alg[tokens[1]] = tokens[2]
                RenderingStyles.styles[tokens[0]] = alg
            line = lines.readline().strip('\n')
        lines.close()

    @staticmethod
    def saveSettings():
        fout = open(RenderingStyles.configFile(), 'w')
        for alg in RenderingStyles.styles.keys():
            for out in RenderingStyles.styles[alg].keys():
                fout.write(alg + '|' + out + '|'
                           + RenderingStyles.styles[alg][out] + '\n')
        fout.close()

    @staticmethod
    def getStyle(algname, outputname):
        if algname in RenderingStyles.styles:
            if outputname in RenderingStyles.styles[algname]:
                return RenderingStyles.styles[algname][outputname]
        return None
