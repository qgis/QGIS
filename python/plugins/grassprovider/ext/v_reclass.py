"""
***************************************************************************
    v_reclass.py
    ----------
    Date                 : June 2023
    Copyright            : (C) 2023 by Andrea Giudiceandrea
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
__date__ = "June 2023"
__copyright__ = "(C) 2023, Andrea Giudiceandrea"


def checkParameterValuesBeforeExecuting(alg, parameters, context):
    """Verify if we have the right parameters"""
    # rules and column parameters are mutually exclusive
    rules = alg.parameterAsString(parameters, "rules", context)
    column = alg.parameterAsString(parameters, "column", context)
    if (rules and column) or (not rules and not column):
        return False, alg.tr("You need to set either a rules file or a column!")

    return True, None
