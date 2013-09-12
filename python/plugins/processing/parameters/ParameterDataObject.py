# -*- coding: utf-8 -*-

"""
***************************************************************************
    ParameterDataObject.py
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

from processing.parameters.Parameter import Parameter
from processing.tools.system import *

class ParameterDataObject(Parameter):

    def getValueAsCommandLineParameter(self):
        if self.value == None:
            return str(None)
        else:
            if not isWindows():
                return "\"" + unicode(self.value) + "\""
            else:
                return "\"" + unicode(self.value).replace("\\", "\\\\") + "\""