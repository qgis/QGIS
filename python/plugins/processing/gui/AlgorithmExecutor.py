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

import sys

from PyQt4.QtCore import QSettings, QCoreApplication
from qgis.core import QgsFeature, QgsVectorFileWriter
from processing.core.ProcessingLog import ProcessingLog
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.gui.Postprocessing import handleAlgorithmResults
from processing.tools import dataobjects
from processing.tools.system import getTempFilename
from processing.tools import vector
from processing.gui.SilentProgress import SilentProgress


def runalg(alg, progress=None):
    """Executes a given algorithm, showing its progress in the
    progress object passed along.

    Return true if everything went OK, false if the algorithm
    could not be completed.
    """
    if progress is None:
        progress = SilentProgress()
    try:
        alg.execute(progress)
        return True
    except GeoAlgorithmExecutionException as e:
        ProcessingLog.addToLog(sys.exc_info()[0], ProcessingLog.LOG_ERROR)
        progress.error(e.msg)
        return False

def runalgIterating(alg, paramToIter, progress):
    # Generate all single-feature layers
    settings = QSettings()
    systemEncoding = settings.value('/UI/encoding', 'System')
    layerfile = alg.getParameterValue(paramToIter)
    layer = dataobjects.getObjectFromUri(layerfile, False)
    feat = QgsFeature()
    filelist = []
    outputs = {}
    provider = layer.dataProvider()
    features = vector.features(layer)
    for feat in features:
        output = getTempFilename('shp')
        filelist.append(output)
        writer = QgsVectorFileWriter(output, systemEncoding,
                provider.fields(), provider.geometryType(), layer.crs())
        writer.addFeature(feat)
        del writer

    # store output values to use them later as basenames for all outputs
    for out in alg.outputs:
        outputs[out.name] = out.value

    # now run all the algorithms
    for i,f in enumerate(filelist):
        alg.setParameterValue(paramToIter, f)
        for out in alg.outputs:
            filename = outputs[out.name]
            if filename:
                filename = filename[:filename.rfind('.')] + '_' + str(i) \
                    + filename[filename.rfind('.'):]
            out.value = filename
        progress.setText(tr('Executing iteration %s/%s...' % (str(i), str(len(filelist)))))
        progress.setPercentage(i * 100 / len(filelist))
        if runalg(alg):
            handleAlgorithmResults(alg, None, False)
        else:
            return False

    return True

def tr(string, context=''):
    if context == '':
        context = 'AlgorithmExecutor'
    return QCoreApplication.translate(context, string)
