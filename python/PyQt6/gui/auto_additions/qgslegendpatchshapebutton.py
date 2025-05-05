# The following has been generated automatically from src/gui/qgslegendpatchshapebutton.h
try:
    QgsLegendPatchShapeButton.__attribute_docs__ = {'changed': "Emitted when the shape's settings are changed.\n\n.. seealso:: :py:func:`shape`\n\n.. seealso:: :py:func:`setShape`\n"}
    QgsLegendPatchShapeButton.__overridden_methods__ = ['minimumSizeHint', 'sizeHint', 'changeEvent', 'showEvent', 'resizeEvent', 'mousePressEvent']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsLegendPatchShapeButton_setPreviewSymbol = QgsLegendPatchShapeButton.setPreviewSymbol
    def __QgsLegendPatchShapeButton_setPreviewSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLegendPatchShapeButton_setPreviewSymbol(self, arg)
    QgsLegendPatchShapeButton.setPreviewSymbol = _functools.update_wrapper(__QgsLegendPatchShapeButton_setPreviewSymbol_wrapper, QgsLegendPatchShapeButton.setPreviewSymbol)

except (NameError, AttributeError):
    pass
