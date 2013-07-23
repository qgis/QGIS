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
from sextante.tests.TestData import table
__author__ = 'Victor Olaya'
__date__ = 'July 2013'
__copyright__ = '(C) 2013, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'


def editCommands(commands):    
    commands[-1] = commands[-1] + " -STATS" + table()
    return commands 
    

