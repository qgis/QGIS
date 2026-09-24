"""
***************************************************************************
    menus.py
    ---------------------
    Date                 : February 2016
    Copyright            : (C) 2016 by Victor Olaya
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

__author__ = "Victor Olaya"
__date__ = "February 2016"
__copyright__ = "(C) 2016, Victor Olaya"

from qgis.core import (
    Qgis,
    QgsApplication,
)
from qgis.gui import QgsMessageViewer
from qgis.PyQt.QtCore import QCoreApplication
from qgis.utils import iface

from processing.gui.algorithm_widget import AlgorithmWidget
from processing.gui.AlgorithmExecutor import execute
from processing.gui.MessageBarProgress import MessageBarProgress
from processing.gui.Postprocessing import handleAlgorithmResults
from processing.tools import dataobjects


def _executeAlgorithm(alg_id):
    alg = QgsApplication.processingRegistry().createAlgorithmById(alg_id)
    if alg is None:
        dlg = QgsMessageViewer()
        dlg.setTitle(
            QCoreApplication.translate("ProcessingPlugin", "Missing Algorithm")
        )
        dlg.setMessage(
            QCoreApplication.translate(
                "ProcessingPlugin",
                'The algorithm "{}" is no longer available. (Perhaps a plugin was uninstalled?)',
            ).format(alg_id),
            Qgis.StringFormat.PlainText,
        )
        dlg.exec()
        return

    ok, message = alg.canExecute()
    if not ok:
        dlg = QgsMessageViewer()
        dlg.setTitle(
            QCoreApplication.translate("ProcessingPlugin", "Missing Dependency")
        )
        dlg.setMessage(
            QCoreApplication.translate(
                "ProcessingPlugin",
                "<h3>Missing dependency. This algorithm cannot be run </h3>\n{0}",
            ).format(message),
            Qgis.StringFormat.Html,
        )
        dlg.exec()
        return

    if (alg.countVisibleParameters()) > 0:
        widget = alg.createCustomParametersWidget(parent=iface.mainWindow())
        if not widget:
            widget = AlgorithmWidget(alg)
        widget.exec()
    else:
        feedback = MessageBarProgress()
        context = dataobjects.createContext(feedback)
        parameters = {}
        ret, results = execute(alg, parameters, context, feedback)
        handleAlgorithmResults(alg, context, feedback)
        feedback.close()
