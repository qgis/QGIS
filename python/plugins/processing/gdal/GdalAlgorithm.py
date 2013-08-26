# -*- coding: utf-8 -*-

"""
***************************************************************************
    GdalAlgorithm.py
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

from processing.script.ScriptAlgorithm import ScriptAlgorithm
from PyQt4 import QtGui
import os

class GdalAlgorithm(ScriptAlgorithm):
    '''Just a ScriptAlgorithm that automatically takes its icon
    filename for the script filename'''

    def getIcon(self):
        filename = os.path.basename(self.descriptionFile[:-2] + "png")
        filepath = os.path.dirname(__file__) + "/icons/" + filename
        return QtGui.QIcon(filepath)