# The following has been generated automatically from src/gui/raster/qgsrasterlayertemporalpropertieswidget.h
try:
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsRasterLayerTemporalPropertiesWidget_addWidget = QgsRasterLayerTemporalPropertiesWidget.addWidget
    def __QgsRasterLayerTemporalPropertiesWidget_addWidget_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsRasterLayerTemporalPropertiesWidget_addWidget(self, arg)
    QgsRasterLayerTemporalPropertiesWidget.addWidget = _functools.update_wrapper(__QgsRasterLayerTemporalPropertiesWidget_addWidget_wrapper, QgsRasterLayerTemporalPropertiesWidget.addWidget)

    QgsRasterLayerTemporalPropertiesWidget.__group__ = ['raster']
except (NameError, AttributeError):
    pass
