# -*- coding: utf-8 -*-

"""
***************************************************************************
    __init__.py
    ---------------------
    Date                 : May 2016
    Copyright            : (C) 2016 by Victor Olaya
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
__date__ = 'May 2016'
__copyright__ = '(C) 2016, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from processing.core.Processing import Processing


class ProcessingExampleScriptsPlugin:

    def initGui(self):
        Processing.addScripts(os.path.join(os.path.dirname(__file__), "scripts"))

    def unload(self):
        Processing.removeScripts(os.path.join(os.path.dirname(__file__), "scripts"))
