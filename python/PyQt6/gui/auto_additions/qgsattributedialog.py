# The following has been generated automatically from src/gui/qgsattributedialog.h
try:
    QgsAttributeDialog.__overridden_methods__ = ['event', 'showEvent', 'createActionContext', 'accept', 'reject']
    import functools as _functools
    __wrapped_QgsAttributeDialog_setExtraContextScope = QgsAttributeDialog.setExtraContextScope
    def __QgsAttributeDialog_setExtraContextScope_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsAttributeDialog_setExtraContextScope(self, arg)
    QgsAttributeDialog.setExtraContextScope = _functools.update_wrapper(__QgsAttributeDialog_setExtraContextScope_wrapper, QgsAttributeDialog.setExtraContextScope)

except (NameError, AttributeError):
    pass
