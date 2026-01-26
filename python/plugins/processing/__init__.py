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

__author__ = "Victor Olaya"
__date__ = "August 2012"
__copyright__ = "(C) 2012, Victor Olaya"

from processing.tools.dataobjects import *  # NOQA
from processing.tools.dataobjects import createContext
from processing.tools.general import *  # NOQA
from processing.tools.general import (
    algorithmHelp,
    run,
    runAndLoadResults,
    createAlgorithmDialog,
    execAlgorithmDialog,
)
from processing.tools.vector import *  # NOQA
from processing.tools.raster import *  # NOQA
from processing.tools.system import *  # NOQA

# monkey patch Python specific Processing API into stable qgis.processing module
import qgis.processing

algorithmHelp.__doc__ = qgis.processing.algorithmHelp.__doc__
qgis.processing.algorithmHelp = algorithmHelp
run.__doc__ = qgis.processing.run.__doc__
qgis.processing.run = run
runAndLoadResults.__doc__ = qgis.processing.runAndLoadResults.__doc__
qgis.processing.runAndLoadResults = runAndLoadResults
createAlgorithmDialog.__doc__ = qgis.processing.createAlgorithmDialog.__doc__
qgis.processing.createAlgorithmDialog = createAlgorithmDialog
execAlgorithmDialog.__doc__ = qgis.processing.execAlgorithmDialog.__doc__
qgis.processing.execAlgorithmDialog = execAlgorithmDialog
createContext.__doc__ = qgis.processing.createContext.__doc__
qgis.processing.createContext = createContext


def classFactory(iface):
    from processing.ProcessingPlugin import ProcessingPlugin

    return ProcessingPlugin(iface)
