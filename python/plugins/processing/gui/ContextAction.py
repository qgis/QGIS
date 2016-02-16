# -*- coding: utf-8 -*-

"""
***************************************************************************
    ContextAction.py
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


from PyQt4 import QtCore


class ContextAction:

    def setData(self, alg, toolbox):
        self.alg = alg
        self.toolbox = toolbox

    def updateToolbox(self):
        '''
        Updates the list of algorithms and then the toolbox.
        It only update the item corresponding to the provider of the algorithm.
        To be called after the action is executed, if needed
        '''
        self.toolbox.updateProvider(self.alg.provider.getName())

    def tr(self, string, context=''):
        if context == '':
            context = 'ContextAction'
        return QtCore.QCoreApplication.translate(context, string)
