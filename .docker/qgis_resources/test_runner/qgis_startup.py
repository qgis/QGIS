"""
Disable QGIS modal error dialog.

This script is meant to be run automatically when QGIS starts.
Is should be renamed to `startup.py` and placed into
~/.qgis3/python/startup.py

"""
from qgis.core import Qgis
from qgis import utils
import traceback


def _showException(type, value, tb, msg, messagebar=False, level=Qgis.Warning):
    print(msg)
    logmessage = ''
    for s in traceback.format_exception(type, value, tb):
        logmessage += s.decode('utf-8', 'replace') if hasattr(s, 'decode') else s
    print(logmessage)


def _open_stack_dialog(type, value, tb, msg, pop_error=True):
    print(msg)


utils.showException = _showException
utils.open_stack_dialog = _open_stack_dialog
