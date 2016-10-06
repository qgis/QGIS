# -*- coding: utf-8 -*-

"""
***************************************************************************
    v_in_lidar.py
    -------------
    Date                 : March 2016
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
__date__ = 'March 2016'
__copyright__ = '(C) 2016, Médéric Ribreux'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


def processCommand(alg):
    # Handle the extent
    extent = alg.getParameterFromName('spatial')
    oldExtent = extent.value
    if extent.value:
        l = extent.value.split(',')
        extent.value = ','.join([l[0], l[2], l[1], l[3]])

    alg.processCommand()
    if extent.value:
        extent.value = oldExtent
