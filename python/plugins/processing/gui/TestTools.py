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
import re
import yaml
import hashlib

from osgeo import gdal
from osgeo.gdalconst import GA_ReadOnly

from numpy import nan_to_num

from qgis.PyQt.QtCore import QCoreApplication, QMetaObject
from qgis.PyQt.QtWidgets import QDialog, QVBoxLayout, QTextEdit

from processing.core.Processing import Processing
from processing.core.outputs import (
    OutputNumber,
    OutputString,
    OutputRaster,
    OutputVector,
    OutputHTML,
    OutputFile
)

from processing.core.parameters import (
    ParameterRaster,
    ParameterVector,
    ParameterMultipleInput,
    ParameterFile,
    ParameterString,
    ParameterNumber,
    ParameterBoolean
)


def extractSchemaPath(filepath):
    """
    Trys to find where the file is relative to the QGIS source code directory.
    If it is already placed in the processing or QGIS testdata directory it will
    return an appropriate schema and relative filepath

    Args:
        filepath: The path of the file to examine

    Returns:
        A tuple (schema, relative_file_path) where the schema is 'qgs' or 'proc'
        if we can assume that the file is in this testdata directory.
    """
    parts = []
    schema = None
    localpath = ''
    path = filepath
    part = True

    while part and filepath:
        (path, part) = os.path.split(path)
        if part == 'testdata' and not localpath:
            localparts = parts
            localparts.reverse()
            localpath = os.path.join(*localparts)

        parts.append(part)

    parts.reverse()

    try:
        testsindex = parts.index('tests')
    except ValueError:
        return '', filepath

    if parts[testsindex - 1] == 'processing':
        schema = 'proc'

    return schema, localpath


def parseParameters(command):
    """
    Parse alg string to grab parameters value.
    Can handle quotes and comma.
    """
    pos = 0
    exp = re.compile(r"""(['"]?)(.*?)\1(,|$)""")
    while True:
        m = exp.search(command, pos)
        result = m.group(2)
        separator = m.group(3)

        # Handle special values:
        if result == 'None':
            result = None
        elif result.lower() == unicode(True).lower():
            result = True
        elif result.lower() == unicode(False).lower():
            result = False

        yield result

        if not separator:
            break

        pos = m.end(0)


def createTest(text):
    definition = {}

    tokens = list(parseParameters(text[len('processing.runalg('):-1]))
    cmdname = tokens[0]
    alg = Processing.getAlgorithm(cmdname)

    definition['name'] = 'Test ({})'.format(cmdname)
    definition['algorithm'] = cmdname

    params = {}
    results = {}

    i = 0
    for param in alg.parameters:
        if param.hidden:
            continue

        i += 1
        token = tokens[i]
        # Handle empty parameters that are optionals
        if param.optional and token is None:
            continue

        if isinstance(param, ParameterVector):
            schema, filepath = extractSchemaPath(token)
            p = {
                'type': 'vector',
                'name': filepath
            }
            if not schema:
                p['location'] = '[The source data is not in the testdata directory. Please use data in the processing/tests/testdata folder.]'

            params[param.name] = p
        elif isinstance(param, ParameterRaster):
            schema, filepath = extractSchemaPath(token)
            p = {
                'type': 'raster',
                'name': filepath
            }
            if not schema:
                p['location'] = '[The source data is not in the testdata directory. Please use data in the processing/tests/testdata folder.]'

            params[param.name] = p
        elif isinstance(param, ParameterMultipleInput):
            multiparams = token.split(';')
            newparam = []

            # Handle datatype detection
            dataType = param.dataType()
            if dataType in ['points', 'lines', 'polygons', 'any vectors']:
                dataType = 'vector'
            else:
                dataType = 'raster'

            for mp in multiparams:
                schema, filepath = extractSchemaPath(mp)
                newparam.append({
                    'type': dataType,
                    'name': filepath
                })
            p = {
                'type': 'multi',
                'params': newparam
            }
            if not schema:
                p['location'] = '[The source data is not in the testdata directory. Please use data in the processing/tests/testdata folder.]'

            params[param.name] = p
        elif isinstance(param, ParameterFile):
            schema, filepath = extractSchemaPath(token)
            p = {
                'type': 'file',
                'name': filepath
            }
            if not schema:
                p['location'] = '[The source data is not in the testdata directory. Please use data in the processing/tests/testdata folder.]'

            params[param.name] = p
        elif isinstance(param, ParameterString):
            params[param.name] = token
        elif isinstance(param, ParameterBoolean):
            params[param.name] = token
        elif isinstance(param, ParameterNumber):
            if param.isInteger:
                params[param.name] = int(token)
            else:
                params[param.name] = float(token)
        else:
            if token[0] == '"':
                token = token[1:]
            if token[-1] == '"':
                token = token[:-1]
            params[param.name] = token

    definition['params'] = params

    for i, out in enumerate([out for out in alg.outputs if not out.hidden]):
        token = tokens[i - alg.getVisibleOutputsCount()]

        if isinstance(out, (OutputNumber, OutputString)):
            results[out.name] = unicode(out)
        elif isinstance(out, OutputRaster):
            dataset = gdal.Open(token, GA_ReadOnly)
            dataArray = nan_to_num(dataset.ReadAsArray(0))
            strhash = hashlib.sha224(dataArray.data).hexdigest()

            results[out.name] = {
                'type': 'rasterhash',
                'hash': strhash
            }
        elif isinstance(out, OutputVector):
            schema, filepath = extractSchemaPath(token)
            results[out.name] = {
                'type': 'vector',
                'name': filepath
            }
            if not schema:
                results[out.name]['location'] = '[The expected result data is not in the testdata directory. Please write it to processing/tests/testdata/expected. Prefer gml files.]'
        elif isinstance(out, OutputHTML) or isinstance(out, OutputFile):
            schema, filepath = extractSchemaPath(token)
            results[out.name] = {
                'type': 'file',
                'name': filepath
            }
            if not schema:
                results[out.name]['location'] = '[The expected result file is not in the testdata directory. Please redirect the output to processing/tests/testdata/expected.]'

    definition['results'] = results
    dlg = ShowTestDialog(yaml.dump([definition], default_flow_style=False))
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
        self.text.setFontFamily("monospace")
        self.text.setEnabled(True)
        # Add two spaces in front of each text for faster copy/paste
        self.text.setText('  {}'.format(s.replace('\n', '\n  ')))
        layout.addWidget(self.text)
        self.setLayout(layout)
        QMetaObject.connectSlotsByName(self)
