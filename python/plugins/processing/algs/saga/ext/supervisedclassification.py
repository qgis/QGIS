# -*- coding: utf-8 -*-

"""
***************************************************************************
    supervisedclassification.py
    ---------------------
    Date                 : July 2013
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
__date__ = 'July 2013'
__copyright__ = '(C) 2013, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from processing.core.ProcessingConfig import ProcessingConfig
from processing.algs.saga.SagaUtils import SagaUtils
from processing.tests.TestData import table


def editCommands(commands):
    saga208 = ProcessingConfig.getSetting(SagaUtils.SAGA_208)
    if saga208 is not None and not saga208:
        commands[-3] = commands[-3] + ' -STATS ' + table()
        return commands
    else:
        return commands
