# -*- coding: utf-8 -*-

"""
***************************************************************************
    ScriptUtils.py
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
from processing.core.ProcessingUtils import ProcessingUtils
from processing.core.ProcessingUtils import mkdir
from processing.core.ProcessingConfig import ProcessingConfig

class ScriptUtils:

    SCRIPTS_FOLDER = "SCRIPTS_FOLDER"
    ACTIVATE_SCRIPTS = "ACTIVATE_SCRIPTS"

    @staticmethod
    def scriptsFolder():
        folder = ProcessingConfig.getSetting(ScriptUtils.SCRIPTS_FOLDER)
        if folder == None:
            folder = unicode(os.path.join(ProcessingUtils.userFolder(), "scripts"))
        mkdir(folder)

        return os.path.abspath(folder)
