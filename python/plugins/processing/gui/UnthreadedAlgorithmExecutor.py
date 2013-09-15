# -*- coding: utf-8 -*-

"""
***************************************************************************
    UnthreadedAlgorithmExecutor.py
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
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.tools import dataobjects
from processing.tools.system import *
from processing.tools import vector 
from processing.core.ProcessingLog import ProcessingLog
from processing.gui.Postprocessing import Postprocessing
from processing.core.SilentProgress import SilentProgress

class UnthreadedAlgorithmExecutor:

    @staticmethod
    def runalg(alg, progress):
        '''Executes a given algorithm, showing its progress in the progress object passed along.
        Return true if everything went OK, false if the algorithm could not be completed'''
        try:
            alg.execute(progress)
            return True
        except GeoAlgorithmExecutionException, e :
            ProcessingLog.addToLog(sys.exc_info()[0], ProcessingLog.LOG_ERROR)            
            QMessageBox.critical(None, "Error", e.msg)
            return False
        except Exception:
            msg = "Error executing " + str(alg.name) + "\nSee log for more information"
            ProcessingLog.addToLog(sys.exc_info()[0], ProcessingLog.LOG_ERROR)                        
            QMessageBox.critical(None, "Uncaught error", msg)
            return False

    @staticmethod
    def runalgIterating(alg,paramToIter,progress):
        #generate all single-feature layers
        settings = QSettings()
        systemEncoding = settings.value( "/UI/encoding", "System" )
        layerfile = alg.getParameterValue(paramToIter)
        layer = dataobjects.getObjectFromUri(layerfile, False)
        feat = QgsFeature()
        filelist = []
        outputs = {}
        provider = layer.dataProvider()
        features = vector.features(layer)
        for feat in features:
            output = getTempFilename("shp")
            filelist.append(output)
            writer = QgsVectorFileWriter(output, systemEncoding, provider.fields(), provider.geometryType(), layer.crs() )
            writer.addFeature(feat)
            del writer

        #store output values to use them later as basenames for all outputs
        for out in alg.outputs:
            outputs[out.name] = out.value

        #now run all the algorithms
        i = 1
        for f in filelist:
            alg.setParameterValue(paramToIter, f)
            for out in alg.outputs:
                filename = outputs[out.name]
                if filename:
                    filename = filename[:filename.rfind(".")] + "_" + str(i) + filename[filename.rfind("."):]
                out.value = filename
            progress.setText("Executing iteration " + str(i) + "/" + str(len(filelist)) + "...")
            progress.setPercentage((i * 100) / len(filelist))
            if UnthreadedAlgorithmExecutor.runalg(alg, SilentProgress()):
                Postprocessing.handleAlgorithmResults(alg, SilentProgress(), False)
                i+=1
            else:
                return False;

        return True

