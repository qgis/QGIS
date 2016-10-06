# -*- coding: utf-8 -*-

"""
***************************************************************************
    r_topmodel.py
    -------------
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


def processCommand(alg):
    # We temporary remove the output
    out = alg.getOutputFromName('out')
    topidx = alg.getParameterValue('topidx')
    command = "r.topmodel parameters=\"{}\" topidxstats=\"{}\" input=\"{}\" output=\"{}\" {} {}--overwrite".format(
        alg.getParameterValue('parameters'),
        alg.getParameterValue('topidxstats'),
        alg.getParameterValue('input'),
        alg.getOutputValue('output'),
        'timestep={}'.format(alg.getParameterValue('timestep')) if alg.getParameterValue('timestep') else '',
        'topidxclass={}'.format(alg.getParameterValue('topidxclass')) if alg.getParameterValue('topidxclass') else ''
    )
    alg.commands.append(command)
