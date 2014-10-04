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
from PyQt4 import QtGui
from PyQt4 import QtCore


class ToolboxAction:

    def __init__(self):
        # This should be true if the action should be shown even if
        # there are no algorithms in the provider (for instance,
        # when it is deactivated
        self.showAlways = False

    def setData(self, toolbox):
        self.toolbox = toolbox

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + '/../images/alg.png')

    def tr(self, string, context=''):
        if context == '':
            context = 'ToolboxAction'
        return QtCore.QCoreApplication.translate(context, string)
