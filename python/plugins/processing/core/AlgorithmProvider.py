# -*- coding: utf-8 -*-

"""
***************************************************************************
    AlgorithmProvider.py
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

from qgis.core import (QgsProcessingProvider)


class AlgorithmProvider(QgsProcessingProvider):

    """This is the base class for algorithms providers.

    An algorithm provider is a set of related algorithms, typically
    from the same external application or related to a common area
    of analysis.
    """

    def __init__(self):
        super().__init__()
        self.actions = []
        self.contextMenuActions = []
