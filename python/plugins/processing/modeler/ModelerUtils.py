# -*- coding: utf-8 -*-

"""
***************************************************************************
    ModelerUtils.py
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

import os
from processing.tools.system import *
from processing.core.ProcessingConfig import ProcessingConfig


class ModelerUtils:

    allAlgs = {}
    providers = {}

    MODELS_FOLDER = 'MODELS_FOLDER'
    ACTIVATE_MODELS = 'ACTIVATE_MODELS'

    @staticmethod
    def modelsFolder():
        folder = ProcessingConfig.getSetting(ModelerUtils.MODELS_FOLDER)
        if folder is None:
            folder = unicode(os.path.join(userFolder(), 'models'))
        mkdir(folder)

        return os.path.abspath(folder)

    @staticmethod
    def getAlgorithm(name):
        for provider in ModelerUtils.allAlgs.values():
            if name in provider:
                return provider[name]
        return None

