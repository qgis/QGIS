# -*- coding: utf-8 -*-

"""
***************************************************************************
    TestTools.py
    ---------------------
    Date                 : February 2013
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
__date__ = 'February 2013'
__copyright__ = '(C) 2013, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from sextante.core.Sextante import Sextante
from sextante.outputs.OutputNumber import OutputNumber
from sextante.outputs.OutputString import OutputString
from sextante.outputs.OutputRaster import OutputRaster
from osgeo import gdal
from osgeo.gdalconst import GA_ReadOnly
from sextante.core.QGisLayers import QGisLayers
from sextante.outputs.OutputVector import OutputVector

def createTest(text):
    s = ""
    tokens =  text[len("sextante.runalg("):-1].split(",")
    cmdname = tokens[0][1:-1];
    methodname = "test_" + cmdname.replace(":","")
    s += "def " + methodname + "(self):\n"
    alg = Sextante.getAlgorithm(cmdname)
    execcommand = "sextante.runalg("
    i = 0
    for token in tokens:
        if i < alg.getVisibleParametersCount() + 1:
            if os.path.exists(token[1:-1]):
                token = os.path.basename(token[1:-1])[:-4] + "()"
            execcommand += token + ","
        else:
            execcommand += "None,"
        i+=1
    s += "\toutputs=" + execcommand[:-1] + ")\n"

    i = -1 * len(alg.outputs)
    for out in alg.outputs:
        filename = tokens[i][1:-1]
        if (tokens[i] == str(None)):
            QtGui.QMessageBox.critical(None, "Error", "Cannot create unit test for that algorithm execution.\nThe output cannot be a temporary file")
            return
        s+="\toutput=outputs['" + out.name + "']\n"
        if isinstance(out, (OutputNumber, OutputString)):
            s+="self.assertTrue(" + str(out) + ", output.value)\n"
        if isinstance(out, OutputRaster):
            dataset = gdal.Open(filename, GA_ReadOnly)
            strhash = hash(str(dataset.ReadAsArray(0).tolist()))
            s+="\tself.assertTrue(os.path.isfile(output))\n"
            s+="\tdataset=gdal.Open(output, GA_ReadOnly)\n"
            s+="\tstrhash=hash(str(dataset.ReadAsArray(0).tolist()))\n"
            s+="\tself.assertEqual(strhash," + str(strhash) + ")\n"
        if isinstance(out, OutputVector):
            layer = Sextante.getObject(filename)
            fields = layer.pendingFields()
            s+="\tlayer=QGisLayers.getObjectFromUri(output, True)\n"
            s+="\tfields=layer.pendingFields()\n"
            s+="\texpectednames=[" + ",".join(["'" + str(f.name()) + "'" for f in fields]) + "]\n"
            s+="\texpectedtypes=[" + ",".join(["'" + str(f.typeName()) +"'" for f in fields]) + "]\n"
            s+="\tnames=[str(f.name()) for f in fields]\n"
            s+="\ttypes=[str(f.typeName()) for f in fields]\n"
            s+="\tself.assertEqual(expectednames, names)\n"
            s+="\tself.assertEqual(expectedtypes, types)\n"
            features = QGisLayers.features(layer)
            numfeat = len(features)
            s+="\tfeatures=sextante.getfeatures(layer)\n"
            s+="\tself.assertEqual(" + str(numfeat) + ", len(features))\n"
            if numfeat > 0:
                feature = features.next()
                attrs = feature.attributes()
                s+="\tfeature=features.next()\n"
                s+="\tattrs=feature.attributes()\n"
                s+="\texpectedvalues=[" + ",".join(['"' + str(attr.toString()) + '"' for attr in attrs]) + "]\n"
                s+="\tvalues=[str(attr.toString()) for attr in attrs]\n"
                s+="\tself.assertEqual(expectedvalues, values)\n"
                s+="\twkt='" + str(feature.geometry().exportToWkt()) + "'\n"
                s+="\tself.assertEqual(wkt, str(feature.geometry().exportToWkt()))"

    dlg = ShowTestDialog(s)
    dlg.exec_()

from PyQt4 import QtCore, QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *

class ShowTestDialog(QtGui.QDialog):
    def __init__(self, s):
        QtGui.QDialog.__init__(self)
        self.setModal(True)
        self.resize(600,400)
        self.setWindowTitle("Unit test")
        layout = QVBoxLayout()
        self.text = QtGui.QTextEdit()
        self.text.setEnabled(True)
        self.text.setText(s)
        layout.addWidget(self.text)
        self.setLayout(layout)
        QtCore.QMetaObject.connectSlotsByName(self)


