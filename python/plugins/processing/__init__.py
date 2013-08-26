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

from processing.tools.general import runalg, runandload, alghelp, alglist, algoptions, load, extent, getobject
from processing.tools.vector import getfeatures, spatialindex, values, uniquevalues
from processing.tools.raster import scanraster
from processing.tests.TestData import loadTestData

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

def classFactory(iface):
    from processing.ProcessingPlugin import ProcessingPlugin
    return ProcessingPlugin(iface)
