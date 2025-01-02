# The following has been generated automatically from src/gui/editorwidgets/qgsjsoneditwidget.h
# monkey patching scoped based enum
QgsJsonEditWidget.View.Text.__doc__ = "JSON data displayed as text."
QgsJsonEditWidget.View.Tree.__doc__ = "JSON data displayed as tree. Tree view is disabled for invalid JSON data."
QgsJsonEditWidget.View.__doc__ = """View mode, text or tree.

* ``Text``: JSON data displayed as text.
* ``Tree``: JSON data displayed as tree. Tree view is disabled for invalid JSON data.

"""
# --
# monkey patching scoped based enum
QgsJsonEditWidget.FormatJson.Indented.__doc__ = "JSON data formatted with regular indentation"
QgsJsonEditWidget.FormatJson.Compact.__doc__ = "JSON data formatted as a compact one line string"
QgsJsonEditWidget.FormatJson.Disabled.__doc__ = "JSON data is not formatted"
QgsJsonEditWidget.FormatJson.__doc__ = """Format mode in the text view

* ``Indented``: JSON data formatted with regular indentation
* ``Compact``: JSON data formatted as a compact one line string
* ``Disabled``: JSON data is not formatted

"""
# --
try:
    QgsJsonEditWidget.__group__ = ['editorwidgets']
except (NameError, AttributeError):
    pass
