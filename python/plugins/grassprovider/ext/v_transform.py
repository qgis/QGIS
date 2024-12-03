"""
***************************************************************************
    v_transform.py
    ----------
    Date                 : February 2024
    Copyright            : (C) 2024 by Andrea Giudiceandrea
    Email                : andreaerdna at libero dot it
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   any later version.                                                    *
*                                                                         *
***************************************************************************
"""

__author__ = "Andrea Giudiceandrea"
__date__ = "February 2024"
__copyright__ = "(C) 2024, Andrea Giudiceandrea"


def checkParameterValuesBeforeExecuting(alg, parameters, context):
    """Verify if we have the right parameters"""
    # -w, -x and -y parameters are mutually exclusive
    w = alg.parameterAsBoolean(parameters, "-w", context)
    x = alg.parameterAsBoolean(parameters, "-x", context)
    y = alg.parameterAsBoolean(parameters, "-y", context)
    if sum([w, x, y]) > 1:
        return False, alg.tr(
            "The 'Swap coordinates' parameters -w, -x and -y are mutually exclusive. You need to set either none or only one of them!"
        )

    return True, None
