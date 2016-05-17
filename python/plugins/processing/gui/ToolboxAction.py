# -*- coding: utf-8 -*-

"""
***************************************************************************
    ToolboxAction.py
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
from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QCoreApplication


class ToolboxAction:

    def setData(self, toolbox):
        self.toolbox = toolbox

    def getIcon(self):
        return QIcon(os.path.dirname(__file__) + '/../images/alg.png')

    def tr(self, string, context=''):
        if context == '':
            context = self.__class__.__name__
        return QCoreApplication.translate(context, string)

    def trAction(self, string, context=''):
        if context == '':
            context = self.__class__.__name__
        return string, QCoreApplication.translate(context, string)
