# -*- coding: utf-8 -*-

"""
***************************************************************************
    __init__.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
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

from sextante.core.Sextante import runalg, runandload, alghelp, alglist, algoptions, load, \
                                    extent, getObjectFromName, getObjectFromUri, getobject, getfeatures

from sextante.tests.TestData import loadTestData

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

def name():
    return "SEXTANTE"

def description():
    return "SEXTANTE Geoprocessing Platform for QGIS"

def version():
    return "1.0.9"

def icon():
    return "images/toolbox.png"

def category():
  return "Analysis"

def qgisMinimumVersion():
    return "1.8"

def classFactory(iface):
    from sextante.SextantePlugin import SextantePlugin
    return SextantePlugin(iface)
