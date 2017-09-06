# -*- coding: utf-8 -*-

"""
***************************************************************************
    AutoincrementalField.py
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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.PyQt.QtCore import QVariant
from qgis.core import (QgsField)
from processing.algs.qgis.QgisAlgorithm import QgisFeatureBasedAlgorithm


class AutoincrementalField(QgisFeatureBasedAlgorithm):

    def __init__(self):
        super().__init__()
        self.current = 0

    def group(self):
        return self.tr('Vector table')

    def name(self):
        return 'addautoincrementalfield'

    def displayName(self):
        return self.tr('Add autoincremental field')

    def outputName(self):
        return self.tr('Incremented')

    def outputFields(self, inputFields):
        inputFields.append(QgsField('AUTO', QVariant.Int))
        return inputFields

    def processFeature(self, feature, feedback):
        attributes = feature.attributes()
        attributes.append(self.current)
        self.current += 1
        feature.setAttributes(attributes)
        return feature
