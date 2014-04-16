# -*- coding: utf-8 -*-

"""
***************************************************************************
    HtmlReportPostProcessor.py
    ---------------------
    Date                 : December 2012
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
__date__ = 'December 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


def postProcessResults(alg):
    htmlFile = alg.getOutputFromName('html').value
    grassName = alg.grassName
    found = False
    f = open(htmlFile, 'w')
    f.write('<h2>' + grassName + '</h2>\n')
    for line in alg.consoleOutput:
        if found and not line.strip().endswith('exit'):
            f.write(line + '<br>\n')
        if grassName in line and not line.startswith('GRASS'):
            found = True
    f.close()
