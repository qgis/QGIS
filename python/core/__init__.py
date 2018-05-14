# -*- coding: utf-8 -*-

"""
***************************************************************************
    __init__.py
    ---------------------
    Date                 : May 2014
    Copyright            : (C) 2014 by Nathan Woodrow
    Email                : woodrow dot nathan at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Nathan Woodrow'
__date__ = 'May 2014'
__copyright__ = '(C) 2014, Nathan Woodrow'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'


from qgis._core import *

from .additions.readwritecontextentercategory import ReadWriteContextEnterCategory
from .additions.projectdirtyblocker import ProjectDirtyBlocker
from .additions.qgstaskwrapper import QgsTaskWrapper
from .additions.qgsfunction import register_function, qgsfunction
from .additions.edit import edit, QgsEditError
from .additions.fromfunction import fromFunction
from .additions.processing import processing_output_layer_repr, processing_source_repr
from .additions.qgsgeometry import _geometryNonZero
from .additions.qgsdefaultvalue import _isValid

# Injections into classes
QgsGeometry.__nonzero__ = _geometryNonZero
QgsGeometry.__bool__ = _geometryNonZero
QgsDefaultValue.__bool__ = _isValid
QgsReadWriteContext.enterCategory = ReadWriteContextEnterCategory
QgsProject.blockDirtying = ProjectDirtyBlocker
QgsTask.fromFunction = fromFunction
QgsProcessingFeatureSourceDefinition.__repr__ = processing_source_repr
QgsProcessingOutputLayerDefinition.__repr__ = processing_output_layer_repr
