# The following has been generated automatically from src/gui/tableeditor/qgstableeditorwidget.h
try:
    QgsTableEditorWidget.__attribute_docs__ = {'tableChanged': 'Emitted whenever the table contents are changed.\n', 'activeCellChanged': 'Emitted whenever the active (or selected) cell changes in the widget.\n'}
    QgsTableEditorWidget.__overridden_methods__ = ['keyPressEvent']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsTableEditorWidget_setSelectionNumericFormat = QgsTableEditorWidget.setSelectionNumericFormat
    def __QgsTableEditorWidget_setSelectionNumericFormat_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsTableEditorWidget_setSelectionNumericFormat(self, arg)
    QgsTableEditorWidget.setSelectionNumericFormat = _functools.update_wrapper(__QgsTableEditorWidget_setSelectionNumericFormat_wrapper, QgsTableEditorWidget.setSelectionNumericFormat)

    QgsTableEditorWidget.__group__ = ['tableeditor']
except (NameError, AttributeError):
    pass
