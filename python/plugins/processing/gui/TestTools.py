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
from osgeo import gdal
from osgeo.gdalconst import GA_ReadOnly

from PyQt4.QtCore import QCoreApplication, QMetaObject
from PyQt4.QtGui import QMessageBox, QDialog, QVBoxLayout, QTextEdit

from processing.core.Processing import Processing
from processing.core.outputs import OutputNumber
from processing.core.outputs import OutputString
from processing.core.outputs import OutputRaster
from processing.core.outputs import OutputVector
from processing.tools import vector, dataobjects


def createTest(text):
    s = ''
    tokens = text[len('processing.runalg('):-1].split(',')
    cmdname = (tokens[0])[1:-1]
    methodname = 'test_' + cmdname.replace(':', '')
    s += 'def ' + methodname + '(self):\n'
    alg = Processing.getAlgorithm(cmdname)
    execcommand = 'processing.runalg('
    i = 0
    for token in tokens:
        if i < alg.getVisibleParametersCount() + 1:
            if os.path.exists(token[1:-1]):
                token = os.path.basename(token[1:-1])[:-4] + '()'
            execcommand += token + ','
        else:
            execcommand += 'None,'
        i += 1
    s += '\toutputs=' + execcommand[:-1] + ')\n'

    i = -1 * len(alg.outputs)
    for out in alg.outputs:
        filename = (tokens[i])[1:-1]
        if tokens[i] == unicode(None):
            QMessageBox.critical(None, tr('Error'),
                                 tr('Cannot create unit test for that algorithm execution. The '
                                    'output cannot be a temporary file'))
            return
        s += "\toutput=outputs['" + out.name + "']\n"
        if isinstance(out, (OutputNumber, OutputString)):
            s += 'self.assertTrue(' + unicode(out) + ', output.value)\n'
        if isinstance(out, OutputRaster):
            dataset = gdal.Open(filename, GA_ReadOnly)
            strhash = hash(unicode(dataset.ReadAsArray(0).tolist()))
            s += '\tself.assertTrue(os.path.isfile(output))\n'
            s += '\tdataset=gdal.Open(output, GA_ReadOnly)\n'
            s += '\tstrhash=hash(unicode(dataset.ReadAsArray(0).tolist()))\n'
            s += '\tself.assertEqual(strhash,' + unicode(strhash) + ')\n'
        if isinstance(out, OutputVector):
            layer = dataobjects.getObject(filename)
            fields = layer.pendingFields()
            s += '\tlayer=dataobjects.getObjectFromUri(output, True)\n'
            s += '\tfields=layer.pendingFields()\n'
            s += '\texpectednames=[' + ','.join(["'" + unicode(f.name()) + "'"
                                                 for f in fields]) + ']\n'
            s += '\texpectedtypes=[' + ','.join(["'" + unicode(f.typeName()) + "'"
                                                 for f in fields]) + ']\n'
            s += '\tnames=[unicode(f.name()) for f in fields]\n'
            s += '\ttypes=[unicode(f.typeName()) for f in fields]\n'
            s += '\tself.assertEqual(expectednames, names)\n'
            s += '\tself.assertEqual(expectedtypes, types)\n'
            features = vector.features(layer)
            numfeat = len(features)
            s += '\tfeatures=processing.features(layer)\n'
            s += '\tself.assertEqual(' + unicode(numfeat) + ', len(features))\n'
            if numfeat > 0:
                feature = features.next()
                attrs = feature.attributes()
                s += '\tfeature=features.next()\n'
                s += '\tattrs=feature.attributes()\n'
                s += '\texpectedvalues=[' + ','.join(['"' + unicode(attr) + '"'
                                                      for attr in attrs]) + ']\n'
                s += '\tvalues=[unicode(attr) for attr in attrs]\n'
                s += '\tself.assertEqual(expectedvalues, values)\n'
                s += "\twkt='" + unicode(feature.geometry().exportToWkt()) + "'\n"
                s += '\tself.assertEqual(wkt, \
                      unicode(feature.geometry().exportToWkt()))'

    dlg = ShowTestDialog(s)
    dlg.exec_()


def tr(string):
    return QCoreApplication.translate('TestTools', string)


class ShowTestDialog(QDialog):

    def __init__(self, s):
        QDialog.__init__(self)
        self.setModal(True)
        self.resize(600, 400)
        self.setWindowTitle(self.tr('Unit test'))
        layout = QVBoxLayout()
        self.text = QTextEdit()
        self.text.setEnabled(True)
        self.text.setText(s)
        layout.addWidget(self.text)
        self.setLayout(layout)
        QMetaObject.connectSlotsByName(self)
