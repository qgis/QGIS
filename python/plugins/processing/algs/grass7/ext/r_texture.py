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

methodRef = ['asm', 'contrast', 'corr', 'var', 'idm',
             'sa', 'se', 'sv', 'entr', 'dv', 'de',
             'moc1', 'moc2']


def checkParameterValuesBeforeExecuting(alg, parameters, context):
    methodList = alg.parameterAsString(parameters, 'method', context).split(",")
    if len([f for f in methodList if f not in methodRef]) > 0 and not alg.getParameterValue('-a'):
        return alg.tr("You need to set the method list with the following values only: asm, contrast, corr, var, idm, sa, se, sv, entr, dv, de, moc1, moc2!")

    return None
