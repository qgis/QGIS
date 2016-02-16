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
from processing.core.ProcessingConfig import ProcessingConfig
from processing.script.ScriptAlgorithm import ScriptAlgorithm
from processing.script.WrongScriptException import WrongScriptException
from processing.core.ProcessingLog import ProcessingLog
from processing.tools.system import mkdir, userFolder


class ScriptUtils:

    SCRIPTS_FOLDER = 'SCRIPTS_FOLDER'
    ACTIVATE_SCRIPTS = 'ACTIVATE_SCRIPTS'

    @staticmethod
    def scriptsFolder():
        folder = ProcessingConfig.getSetting(ScriptUtils.SCRIPTS_FOLDER)
        if folder is None:
            folder = unicode(os.path.join(userFolder(), 'scripts'))
        mkdir(folder)

        return os.path.abspath(folder)

    @staticmethod
    def loadFromFolder(folder):
        if not os.path.exists(folder):
            return []
        algs = []
        for path, subdirs, files in os.walk(folder):
            for descriptionFile in files:
                if descriptionFile.endswith('py'):
                    try:
                        fullpath = os.path.join(path, descriptionFile)
                        alg = ScriptAlgorithm(fullpath)
                        if alg.name.strip() != '':
                            algs.append(alg)
                    except WrongScriptException as e:
                        ProcessingLog.addToLog(ProcessingLog.LOG_ERROR, e.msg)
                    except Exception as e:
                        ProcessingLog.addToLog(ProcessingLog.LOG_ERROR,
                                               'Could not load script:' + descriptionFile + '\n'
                                               + unicode(e))
        return algs
