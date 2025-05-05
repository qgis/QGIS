# The following has been generated automatically from src/gui/qgspanelwidgetstack.h
try:
    QgsPanelWidgetStack.__overridden_methods__ = ['sizeHint', 'minimumSizeHint', 'mouseReleaseEvent', 'keyPressEvent']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsPanelWidgetStack_setMainPanel = QgsPanelWidgetStack.setMainPanel
    def __QgsPanelWidgetStack_setMainPanel_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsPanelWidgetStack_setMainPanel(self, arg)
    QgsPanelWidgetStack.setMainPanel = _functools.update_wrapper(__QgsPanelWidgetStack_setMainPanel_wrapper, QgsPanelWidgetStack.setMainPanel)

except (NameError, AttributeError):
    pass
