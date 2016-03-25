# -*- coding: utf-8 -*-

"""
***************************************************************************
    r_texture.py
    ------------
    Date                 : February 2016
    Copyright            : (C) 2016 by Médéric Ribreux
    Email                : medspx at medspx dot fr
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Médéric Ribreux'
__date__ = 'February 2016'
__copyright__ = '(C) 2016, Médéric Ribreux'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

# Damn layer naming, I am forced to make a dict to handle this !
methodRef = {'asm': 'ASM', 'contrast': 'Contr', 'corr': 'Corr',
             'var': 'Var', 'idm': 'IDM', 'sa': 'SA', 'se': 'SE',
             'sv': 'SV', 'entr': 'Entr', 'dv': 'DV', 'de': 'DE',
             'moc1': 'MOC-2', 'moc2': 'MOC-2'}


def checkParameterValuesBeforeExecuting(alg):
    methodList = alg.getParameterValue('method').split(",")
    if len([f for f in methodList if f not in methodRef.keys()]) > 0 and not alg.getParameterValue('-a'):
        return alg.tr("You need to set the method list with the following values only: asm, contrast, corr, var, idm, sa, se, sv, entr, dv, de, moc1, moc2!")

    return None


def processOutputs(alg):
    # The name of the output depends on the method
    if alg.getParameterValue('-a'):
        methodList = methodRef.keys()
    else:
        methodList = alg.getParameterValue('method').split(",")

    # handle -s option
    if alg.getParameterValue('-s'):
        angles = ['_0', '_45', '_90', '_135']
    else:
        angles = ['']

    ext = alg.provider.getSupportedOutputRasterLayerExtensions()[0]
    for method in methodList:
        out = alg.getOutputValue(u'output')
        for angle in angles:
            inputRaster = "{}_{}{}".format(alg.exportedLayers[out], methodRef[method], angle)
            outputFile = "{}/{}.{}".format(out, inputRaster, ext)
            command = u"r.out.gdal --overwrite -c createopt=\"TFW=YES,COMPRESS=LZW\" input={} output=\"{}\"".format(
                inputRaster, outputFile)
            alg.commands.append(command)
            alg.outputCommands.append(command)
