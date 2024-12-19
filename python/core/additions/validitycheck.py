"""
***************************************************************************
    validitycheck.py
    ---------------------
    Date                 : January 2019
    Copyright            : (C) 2019 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

from qgis._core import QgsAbstractValidityCheck, QgsApplication


class CheckFactory:
    """
    Constructs QgsAbstractValidityChecks using a decorator.

    To use, Python based checks should use the decorator syntax:

    .. highlight:: python
    .. code-block:: python
        @check.register(type=QgsAbstractValidityCheck.TypeLayoutCheck)
        def my_layout_check(context, feedback):
            results = ...
            return results

    """

    def __init__(self):
        # unfortunately /Transfer/ annotation isn't working correct on validityCheckRegistry().addCheck(),
        # so we manually need to store a reference to all checks we register
        self.checks = []

    def register(self, type, *args, **kwargs):
        """
        Implements a decorator for registering Python based checks.

        :param type: check type, e.g. QgsAbstractValidityCheck.TypeLayoutCheck
        """

        def dec(f):
            check = CheckWrapper(check_type=type, check_func=f)
            self.checks.append(check)
            QgsApplication.validityCheckRegistry().addCheck(check)

        return dec


class CheckWrapper(QgsAbstractValidityCheck):
    """
    Wrapper object used to create new validity checks from @check.
    """

    def __init__(self, check_type, check_func):
        """
        Initializer for CheckWrapper.

        :param check_type: check type, e.g. QgsAbstractValidityCheck.TypeLayoutCheck
        :param check_func: test function, should return a list of QgsValidityCheckResult results
        """
        super().__init__()
        self._check_type = check_type
        self._results = []
        self._check_func = check_func

    def create(self):
        return CheckWrapper(check_type=self._check_type, check_func=self._check_func)

    def id(self):
        return self._check_func.__name__

    def checkType(self):
        return self._check_type

    def prepareCheck(self, context, feedback):
        self._results = self._check_func(context, feedback)
        if self._results is None:
            self._results = []
        return True

    def runCheck(self, context, feedback):
        return self._results


check = CheckFactory()
