# -*- coding: utf-8 -*-

"""
***************************************************************************
    BatchAlgorithmDialog.py
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

import codecs
import time

from qgis.core import (QgsProcessingOutputHtml,
                       QgsProcessingOutputNumber,
                       QgsProcessingOutputString,
                       QgsProcessingOutputBoolean,
                       QgsProject)
from qgis.gui import QgsProcessingBatchAlgorithmDialogBase
from qgis.utils import iface

from processing.core.ProcessingResults import resultsList
from processing.gui.BatchPanel import BatchPanel
from processing.gui.Postprocessing import handleAlgorithmResults
from processing.tools import dataobjects
from processing.tools.system import getTempFilename


class BatchAlgorithmDialog(QgsProcessingBatchAlgorithmDialogBase):

    def __init__(self, alg, parent=None):
        super().__init__(parent)

        self.setAlgorithm(alg)

        self.setWindowTitle(self.tr('Batch Processing - {0}').format(self.algorithm().displayName()))
        self.setMainWidget(BatchPanel(self, self.algorithm()))

        self.context = None
        self.hideShortHelp()

    def runAsSingle(self):
        self.close()

        from processing.gui.AlgorithmDialog import AlgorithmDialog
        dlg = AlgorithmDialog(self.algorithm().create(), parent=iface.mainWindow())
        dlg.show()
        dlg.exec_()

    def processingContext(self):
        if self.context is None:
            self.feedback = self.createFeedback()
            self.context = dataobjects.createContext(self.feedback)
            self.context.setLogLevel(self.logLevel())
        return self.context

    def createContext(self, feedback):
        return dataobjects.createContext(feedback)

    def runAlgorithm(self):
        alg_parameters = []

        load_layers = self.mainWidget().checkLoadLayersOnCompletion.isChecked()
        project = QgsProject.instance() if load_layers else None

        for row in range(self.mainWidget().batchRowCount()):
            parameters, ok = self.mainWidget().parametersForRow(row, destinationProject=project, warnOnInvalid=True)
            if ok:
                alg_parameters.append(parameters)
        if not alg_parameters:
            return

        self.execute(alg_parameters)

    def handleAlgorithmResults(self, algorithm, context, feedback, parameters):
        handleAlgorithmResults(algorithm, context, feedback, False, parameters)

    def loadHtmlResults(self, results, num):
        for out in self.algorithm().outputDefinitions():
            if isinstance(out, QgsProcessingOutputHtml) and out.name() in results and results[out.name()]:
                resultsList.addResult(icon=self.algorithm().icon(), name='{} [{}]'.format(out.description(), num),
                                      result=results[out.name()])

    def createSummaryTable(self, algorithm_results, errors):
        createTable = False

        for out in self.algorithm().outputDefinitions():
            if isinstance(out, (QgsProcessingOutputNumber, QgsProcessingOutputString, QgsProcessingOutputBoolean)):
                createTable = True
                break

        if not createTable and not errors:
            return

        outputFile = getTempFilename('html')
        with codecs.open(outputFile, 'w', encoding='utf-8') as f:
            if createTable:
                for i, res in enumerate(algorithm_results):
                    results = res['results']
                    params = res['parameters']
                    if i > 0:
                        f.write('<hr>\n')
                    f.write(self.tr('<h3>Parameters</h3>\n'))
                    f.write('<table>\n')
                    for param in self.algorithm().parameterDefinitions():
                        if not param.isDestination():
                            if param.name() in params:
                                f.write('<tr><th>{}</th><td>{}</td></tr>\n'.format(param.description(),
                                                                                   params[param.name()]))
                    f.write('</table>\n')
                    f.write(self.tr('<h3>Results</h3>\n'))
                    f.write('<table>\n')
                    for out in self.algorithm().outputDefinitions():
                        if out.name() in results:
                            f.write('<tr><th>{}</th><td>{}</td></tr>\n'.format(out.description(), results[out.name()]))
                    f.write('</table>\n')
            if errors:
                f.write('<h2 style="color: red">{}</h2>\n'.format(self.tr('Errors')))
            for i, res in enumerate(errors):
                errors = res['errors']
                params = res['parameters']
                if i > 0:
                    f.write('<hr>\n')
                f.write(self.tr('<h3>Parameters</h3>\n'))
                f.write('<table>\n')
                for param in self.algorithm().parameterDefinitions():
                    if not param.isDestination():
                        if param.name() in params:
                            f.write(
                                '<tr><th>{}</th><td>{}</td></tr>\n'.format(param.description(), params[param.name()]))
                f.write('</table>\n')
                f.write('<h3>{}</h3>\n'.format(self.tr('Error')))
                f.write('<p style="color: red">{}</p>\n'.format('<br>'.join(errors)))

        resultsList.addResult(icon=self.algorithm().icon(),
                              name='{} [summary]'.format(self.algorithm().name()), timestamp=time.localtime(),
                              result=outputFile)
