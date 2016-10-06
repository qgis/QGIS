# -*- coding: utf-8 -*-

"""
***************************************************************************
    r_drain.py
    ----------
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

import os


def checkParameterValuesBeforeExecuting(alg):
    """ Verify if we have the right parameters """
    if alg.getParameterValue('start_coordinates') and alg.getParameterValue('start_points'):
        return alg.tr("You need to set either start coordinates OR a start points vector layer!")

    parameters = [alg.getParameterValue(f) for f in ['-c', '-a', '-n']]
    paramscore = [f for f in parameters if f]
    if len(paramscore) > 1:
        return alg.tr("-c, -a, -n parameters are mutually exclusive!")
    return None
