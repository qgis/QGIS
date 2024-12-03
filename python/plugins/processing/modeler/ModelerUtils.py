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

__author__ = "Victor Olaya"
__date__ = "August 2012"
__copyright__ = "(C) 2012, Victor Olaya"

import os
from processing.tools.system import userFolder, mkdir
from processing.core.ProcessingConfig import ProcessingConfig


class ModelerUtils:
    MODELS_FOLDER = "MODELS_FOLDER"

    @staticmethod
    def defaultModelsFolder():
        folder = str(os.path.join(userFolder(), "models"))
        mkdir(folder)
        return os.path.abspath(folder)

    @staticmethod
    def modelsFolders():
        folder = ProcessingConfig.getSetting(ModelerUtils.MODELS_FOLDER)
        if folder is not None:
            return folder.split(";")
        else:
            return [ModelerUtils.defaultModelsFolder()]
