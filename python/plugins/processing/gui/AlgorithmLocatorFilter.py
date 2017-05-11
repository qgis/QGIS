# -*- coding: utf-8 -*-

"""
***************************************************************************
    AlgorithmLocatorFilter.py
    -------------------------
    Date                 : May 2017
    Copyright            : (C) 2017 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Nyall Dawson'
__date__ = 'May 2017'
__copyright__ = '(C) 2017, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


from qgis.core import (QgsApplication,
                       QgsProcessingAlgorithm)
from qgis.gui import (QgsLocatorFilter,
                      QgsLocatorResult)
from processing.gui.MessageDialog import MessageDialog
from processing.gui.AlgorithmDialog import AlgorithmDialog
from qgis.utils import iface

class AlgorithmLocatorFilter(QgsLocatorFilter):

    def __init__(self, parent=None):
        super(AlgorithmLocatorFilter, self).__init__(parent)

    def fetchResults(self,string,context,feedback):
        for a in QgsApplication.processingRegistry().algorithms():
            if feedback.isCanceled():
                return
            if a.flags() & QgsProcessingAlgorithm.FlagHideFromToolbox:
                continue

            if string.lower() in a.displayName().lower() or [t for t in a.tags() if string.lower() in t.lower()]:
                result = QgsLocatorResult()
                result.filter = self
                result.displayString = a.displayName()
                result.icon = a.icon()
                result.userData = a.id()
                self.resultFetched.emit(result)

    def triggerResult(self, result):
        a = QgsApplication.processingRegistry().algorithmById(result.userData)
        if a:
            alg = a.getCopy()
            message = alg.checkBeforeOpeningParametersDialog()
            if message:
                dlg = MessageDialog()
                dlg.setTitle(self.tr('Missing dependency'))
                dlg.setMessage(message)
                dlg.exec_()
                return
            dlg = alg.getCustomParametersDialog()
            if not dlg:
                dlg = AlgorithmDialog(alg)
            canvas = iface.mapCanvas()
            prevMapTool = canvas.mapTool()
            dlg.show()
            dlg.exec_()
            if canvas.mapTool() != prevMapTool:
                try:
                    canvas.mapTool().reset()
                except:
                    pass
                canvas.setMapTool(prevMapTool)
