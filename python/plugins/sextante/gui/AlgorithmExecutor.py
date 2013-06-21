# -*- coding: utf-8 -*-

"""
***************************************************************************
    AlgorithmExecutor.py
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

from PyQt4.QtGui import *
from PyQt4.QtCore import *
from qgis.core import *
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from sextante.core.QGisLayers import QGisLayers
from sextante.core.SextanteUtils import SextanteUtils
from sextante.tools.vector import getfeatures
from sextante.gui.SextantePostprocessing import SextantePostprocessing
import sys

class AlgorithmExecutor(QThread):
    percentageChanged = pyqtSignal(int)
    textChanged = pyqtSignal(str)
    error = pyqtSignal(str)
    internalError = pyqtSignal(BaseException)
    iterated = pyqtSignal(int)
    infoSet = pyqtSignal(str)
    commandSet = pyqtSignal(str)
    debugInfoSet = pyqtSignal(str)
    consoleInfoSet = pyqtSignal(str)
    algExecuted = pyqtSignal()
    #started & finished inherited from QThread

    def __init__(self, alg, iterParam = None, parent = None):
        QThread.__init__(self, parent)
        self.algorithm = alg
        self.parameterToIterate = iterParam

        class Progress:
            def __init__(self, algex):
                self.algorithmExecutor = algex
            def setText(self, text):
                self.algorithmExecutor.textChanged.emit(text)
            def setPercentage(self, p):
                self.algorithmExecutor.percentageChanged.emit(p)
            def setInfo(self, info):
                self.algorithmExecutor.infoSet.emit(info)
            def setCommand(self, cmd):
                self.algorithmExecutor.commandSet.emit(cmd)
            def setDebugInfo(self, info):
                self.algorithmExecutor.debugInfoSet.emit(info)
            def setConsoleInfo(self, info):
                self.algorithmExecutor.consoleInfoSet.emit(info)
        self.progress = Progress(self)
        if self.parameterToIterate:
            self.run = self.runalgIterating

            #generate all single-feature layers
            settings = QSettings()
            systemEncoding = settings.value( "/UI/encoding", "System" )
            layerfile = alg.getParameterValue(self.parameterToIterate)
            layer = QGisLayers.getObjectFromUri(layerfile, False)
            provider = layer.dataProvider()
            features = getfeatures(layer)
            self.filelist = []
            for feat in features:
                output = SextanteUtils.getTempFilename("shp")
                self.filelist.append(output)
                writer = QgsVectorFileWriter(output, systemEncoding,provider.fields(), provider.geometryType(), layer.crs() )
                writer.addFeature(feat)
                del writer
        else:
            self.run = self.runalg
        self.internalError.connect(self.raiseInternalError)

    def raiseInternalError(self, error):
        raise error

    def runalg(self):
        try:
            self.algorithm.execute(self.progress)
            if not self.parameterToIterate:
                self.algExecuted.emit()
        except GeoAlgorithmExecutionException, e :
            self.error.emit(e.msg)
        except BaseException, e:
            self.internalError.emit(e)
        # catch *all* errors, because QGIS tries to handle them in the GUI, which is fatal, this
        # being a separate thread.
        except:
            msg = "Error executing " + str(self.alg.name) + "\n" + sys.exc_info()[0]
            print msg
            self.internalError.emit(msg)

    def runalgIterating(self):
        try:
            outputs = {}
            #store output values to use them later as basenames for all outputs
            for out in self.algorithm.outputs:
                outputs[out.name] = out.value
            i = 1
            for f in self.filelist:
                self.algorithm.setParameterValue(self.parameterToIterate, f)
                for out in self.algorithm.outputs:
                    filename = outputs[out.name]
                    if filename:
                        filename = filename[:filename.rfind(".")] + "_" + str(i) + filename[filename.rfind("."):]
                    out.value = filename
                self.progress.setText("Executing iteration " + str(i) + "/" + str(len(self.filelist)) + "...")
                self.progress.setPercentage((i * 100) / len(self.filelist))
                self.runalg()
                SextantePostprocessing.handleAlgorithmResults(self.algorithm, self.progress, False)
                self.iterated.emit(i)
                i += 1
            self.algExecuted.emit()
        except BaseException, e:
            self.error.emit(str(e))
            print "Error iterating " + str(e)
        except:
            print "Error iterating " + str(self)
