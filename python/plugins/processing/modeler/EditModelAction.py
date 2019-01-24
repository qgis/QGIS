# -*- coding: utf-8 -*-

"""
***************************************************************************
    EditModelAction.py
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

from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import QgsApplication, QgsProcessingModelAlgorithm, QgsMessageLog
from processing.gui.ContextAction import ContextAction
from processing.modeler.ModelerDialog import ModelerDialog
from qgis.core import Qgis
from qgis.utils import iface


class EditModelAction(ContextAction):

    def __init__(self):
        self.name = QCoreApplication.translate('EditModelAction', 'Edit Model…')

    def isEnabled(self):
        return isinstance(self.itemData, QgsProcessingModelAlgorithm)

    def execute(self):
        alg = self.itemData
        ok, msg = alg.canExecute()
        if not ok:
            iface.messageBar().pushMessage(QCoreApplication.translate('EditModelAction', 'Cannot edit model: {}').format(msg), level=Qgis.Warning)
        else:
            dlg = ModelerDialog(alg)
            dlg.update_model.connect(self.updateModel)
            dlg.show()

    def updateModel(self):
        QgsApplication.processingRegistry().providerById('model').refreshAlgorithms()
