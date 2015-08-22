# -*- coding: utf-8 -*-

"""
***************************************************************************
    Postprocessing.py
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
from PyQt4.QtGui import QApplication
from PyQt4.QtCore import QCoreApplication
from qgis.core import QgsMapLayerRegistry

from processing.core.ProcessingConfig import ProcessingConfig
from processing.core.ProcessingResults import ProcessingResults

from processing.gui.ResultsDialog import ResultsDialog
from processing.gui.RenderingStyles import RenderingStyles
from processing.gui.MessageDialog import MessageDialog
from processing.gui.SilentProgress import SilentProgress

from processing.core.outputs import OutputRaster
from processing.core.outputs import OutputVector
from processing.core.outputs import OutputTable
from processing.core.outputs import OutputHTML

from processing.tools import dataobjects


def handleAlgorithmResults(alg, progress=None, showResults=True):
    wrongLayers = []
    htmlResults = False
    if progress is None:
        progress = SilentProgress()
    progress.setText(QCoreApplication.translate('Postprocessing', 'Loading resulting layers'))
    i = 0
    for out in alg.outputs:
        progress.setPercentage(100 * i / float(len(alg.outputs)))
        if out.hidden or not out.open:
            continue
        if isinstance(out, (OutputRaster, OutputVector, OutputTable)):
            try:
                if out.value.startswith('memory:'):
                    layer = out.memoryLayer
                    QgsMapLayerRegistry.instance().addMapLayers([layer])
                else:
                    if ProcessingConfig.getSetting(
                            ProcessingConfig.USE_FILENAME_AS_LAYER_NAME):
                        name = os.path.basename(out.value)
                    else:
                        name = out.description
                    dataobjects.load(out.value, name, alg.crs,
                                     RenderingStyles.getStyle(alg.commandLineName(),
                                                              out.name))
            except Exception as e:
                wrongLayers.append(out.description)
        elif isinstance(out, OutputHTML):
            ProcessingResults.addResult(out.description, out.value)
            htmlResults = True
        i += 1
    if wrongLayers:
        QApplication.restoreOverrideCursor()
        dlg = MessageDialog()
        dlg.setTitle(QCoreApplication.translate('Postprocessing', 'Problem loading output layers'))
        msg = "The following layers were not correctly generated.<ul>"
        msg += "".join(["<li>%s</li>" % lay for lay in wrongLayers]) + "</ul>"
        msg += "You can check the <a href='log'>log messages</a> to find more information about the execution of the algorithm"
        dlg.setMessage(msg)
        dlg.exec_()

    if showResults and htmlResults and not wrongLayers:
        QApplication.restoreOverrideCursor()
        dlg = ResultsDialog()
        dlg.exec_()

    return len(wrongLayers) == 0
