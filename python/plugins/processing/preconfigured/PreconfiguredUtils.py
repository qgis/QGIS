# -*- coding: utf-8 -*-

"""
***************************************************************************
    PreconfiguredUtils.py
    ---------------------
    Date                 : April 2016
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

import os
from processing.tools.system import mkdir, userFolder


def preconfiguredAlgorithmsFolder():
    folder = str(os.path.join(userFolder(), 'preconfigured'))
    mkdir(folder)
    return folder


def algAsDict(alg):
    params = {
        param.name: param.value
        for param in alg.parameters
    }
    outputs = {
        out.name: out.value
        for out in alg.outputs
    }
    return {"parameters": params, "outputs": outputs, "algname": alg.id()}
