# -*- coding: utf-8 -*-

"""
***************************************************************************
    i_in_spotvgt.py
    ---------------
    Date                 : April 2016
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
__date__ = 'April 2016'
__copyright__ = '(C) 2016, Médéric Ribreux'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


def processInputs(alg):
    # Here, we apply directly the algorithm
    # So we just need to get the projection of the layer !
    layer = alg.getParameterValue('input')
    alg.setSessionProjectionFromLayer(layer, alg.commands)
