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
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from sextante.core.QGisLayers import QGisLayers
from sextante.core.SextanteUtils import SextanteUtils
from sextante.gui.SextantePostprocessing import SextantePostprocessing
from sextante.core.SilentProgress import SilentProgress
import traceback

class UnthreadedAlgorithmExecutor:

    @staticmethod
    def runalg(alg, progress):
        '''Executes a given algorithm, showing its progress in the progress object passed along.
        Return true if everything went OK, false if the algorithm could not be completed'''
        try:
            alg.execute(progress)
            return True
        except GeoAlgorithmExecutionException, e :
            QMessageBox.critical(None, "Error", e.msg)
            return False
        except Exception:
            QMessageBox.critical(None, "Uncaught error", traceback.format_exc())
            return False

    @staticmethod
    def runalgIterating(alg,paramToIter,progress):
        #generate all single-feature layers
        settings = QSettings()
        systemEncoding = settings.value( "/UI/encoding", "System" ).toString()
        layerfile = alg.getParameterValue(paramToIter)
        layer = QGisLayers.getObjectFromUri(layerfile, False)
        provider = layer.dataProvider()
        allAttrs = provider.attributeIndexes()
        provider.select( allAttrs )
        feat = QgsFeature()
        filelist = []
        outputs = {}
        while provider.nextFeature(feat):
            output = SextanteUtils.getTempFilename("shp")
            filelist.append(output)
            writer = QgsVectorFileWriter(output, systemEncoding,provider.fields(), provider.geometryType(), layer.crs() )
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
                SextantePostprocessing.handleAlgorithmResults(alg, progress, False)
                i+=1
            else:
                return False;

        return True

