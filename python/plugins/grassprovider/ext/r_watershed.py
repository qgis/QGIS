# -*- coding: utf-8 -*-

"""
***************************************************************************
    r_watershed.py
    --------
    Date                 : June 2021
    Copyright            : (C) 2021 by Nicolas Godet
    Email                : nicolas dot godet at outlook dot fr
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Nicolas Godet'
__date__ = 'June 2021'
__copyright__ = '(C) 2021, Nicolas Godet'


def checkParameterValuesBeforeExecuting(alg, parameters, context):
    """ Verify if we have the right parameters """
    # Verify that threshold is a positive integer
    if alg.parameterAsInt(parameters, 'threshold', context) < 1:
        return False, alg.tr("Minimum size of exterior watershed basin must be a positive integer.")

    return True, None
