# -*- coding: utf-8 -*-

"""
***************************************************************************
    MessageBarProgress.py
    ---------------------
    Date                 : April 2013
    Copyright            : (C) 2013 by Victor Olaya
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
__date__ = 'April 2013'
__copyright__ = '(C) 2013, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.PyQt.QtCore import Qt, QCoreApplication
from qgis.PyQt.QtWidgets import QProgressBar
from qgis.utils import iface
from qgis.core import QgsProcessingFeedback, Qgis
from processing.gui.MessageDialog import MessageDialog


class MessageBarProgress(QgsProcessingFeedback):

    def __init__(self, algname=None):
        QgsProcessingFeedback.__init__(self)

        self.msg = []
        self.progressMessageBar = \
            iface.messageBar().createMessage(self.tr('Executing algorithm <i>{0}</i>'.format(algname if algname else '')))
        self.progress = QProgressBar()
        self.progressChanged.connect(self.progress.setValue)
        self.progress.setMaximum(100)
        self.progress.setAlignment(Qt.AlignLeft | Qt.AlignVCenter)
        self.progressMessageBar.layout().addWidget(self.progress)
        self.message_bar_item = iface.messageBar().pushWidget(self.progressMessageBar,
                                                              Qgis.Info)

    def reportError(self, msg, fatalError=False):
        self.msg.append(msg)

    def close(self):
        if self.msg:
            dlg = MessageDialog()
            dlg.setTitle(QCoreApplication.translate('MessageBarProgress', 'Problem executing algorithm'))
            dlg.setMessage("<br>".join(self.msg))
            dlg.exec_()
        iface.messageBar().popWidget(self.message_bar_item)

    def tr(self, string, context=''):
        if context == '':
            context = 'MessageBarProgress'
        return QCoreApplication.translate(context, string)
