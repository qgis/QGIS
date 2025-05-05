# The following has been generated automatically from src/gui/qgsfieldexpressionwidget.h
try:
    QgsFieldExpressionWidget.__attribute_docs__ = {'fieldChanged': 'fieldChanged signal with indication of the validity of the expression\n', 'allowEvalErrorsChanged': "Allow accepting expressions with evaluation errors. This can be useful\nwhen we are not able to provide an expression context of which we are\nsure it's completely populated.\n", 'buttonVisibleChanged': 'Emitted when the button visibility changes\n\n.. versionadded:: 3.36\n'}
    QgsFieldExpressionWidget.__overridden_methods__ = ['changeEvent', 'eventFilter']
    QgsFieldExpressionWidget.__signal_arguments__ = {'fieldChanged': ['fieldName: str', 'isValid: bool']}
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsFieldExpressionWidget_appendScope = QgsFieldExpressionWidget.appendScope
    def __QgsFieldExpressionWidget_appendScope_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsFieldExpressionWidget_appendScope(self, arg)
    QgsFieldExpressionWidget.appendScope = _functools.update_wrapper(__QgsFieldExpressionWidget_appendScope_wrapper, QgsFieldExpressionWidget.appendScope)

except (NameError, AttributeError):
    pass
