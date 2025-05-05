# The following has been generated automatically from src/gui/qgssymbolbutton.h
try:
    QgsSymbolButton.__attribute_docs__ = {'changed': "Emitted when the symbol's settings are changed.\n\n.. seealso:: :py:func:`symbol`\n\n.. seealso:: :py:func:`setSymbol`\n"}
    QgsSymbolButton.__overridden_methods__ = ['minimumSizeHint', 'sizeHint', 'changeEvent', 'showEvent', 'resizeEvent', 'mousePressEvent', 'mouseMoveEvent', 'mouseReleaseEvent', 'keyPressEvent', 'dragEnterEvent', 'dragLeaveEvent', 'dropEvent', 'wheelEvent']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsSymbolButton_setDefaultSymbol = QgsSymbolButton.setDefaultSymbol
    def __QgsSymbolButton_setDefaultSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsSymbolButton_setDefaultSymbol(self, arg)
    QgsSymbolButton.setDefaultSymbol = _functools.update_wrapper(__QgsSymbolButton_setDefaultSymbol_wrapper, QgsSymbolButton.setDefaultSymbol)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsSymbolButton_setSymbol = QgsSymbolButton.setSymbol
    def __QgsSymbolButton_setSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsSymbolButton_setSymbol(self, arg)
    QgsSymbolButton.setSymbol = _functools.update_wrapper(__QgsSymbolButton_setSymbol_wrapper, QgsSymbolButton.setSymbol)

except (NameError, AttributeError):
    pass
