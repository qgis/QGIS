# The following has been generated automatically from src/core/pointcloud/qgspointcloudrgbrenderer.h
try:
    QgsPointCloudRgbRenderer.create = staticmethod(QgsPointCloudRgbRenderer.create)
    QgsPointCloudRgbRenderer.__overridden_methods__ = ['type', 'clone', 'renderBlock', 'save', 'usedAttributes']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsPointCloudRgbRenderer_setRedContrastEnhancement = QgsPointCloudRgbRenderer.setRedContrastEnhancement
    def __QgsPointCloudRgbRenderer_setRedContrastEnhancement_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsPointCloudRgbRenderer_setRedContrastEnhancement(self, arg)
    QgsPointCloudRgbRenderer.setRedContrastEnhancement = _functools.update_wrapper(__QgsPointCloudRgbRenderer_setRedContrastEnhancement_wrapper, QgsPointCloudRgbRenderer.setRedContrastEnhancement)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsPointCloudRgbRenderer_setGreenContrastEnhancement = QgsPointCloudRgbRenderer.setGreenContrastEnhancement
    def __QgsPointCloudRgbRenderer_setGreenContrastEnhancement_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsPointCloudRgbRenderer_setGreenContrastEnhancement(self, arg)
    QgsPointCloudRgbRenderer.setGreenContrastEnhancement = _functools.update_wrapper(__QgsPointCloudRgbRenderer_setGreenContrastEnhancement_wrapper, QgsPointCloudRgbRenderer.setGreenContrastEnhancement)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsPointCloudRgbRenderer_setBlueContrastEnhancement = QgsPointCloudRgbRenderer.setBlueContrastEnhancement
    def __QgsPointCloudRgbRenderer_setBlueContrastEnhancement_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsPointCloudRgbRenderer_setBlueContrastEnhancement(self, arg)
    QgsPointCloudRgbRenderer.setBlueContrastEnhancement = _functools.update_wrapper(__QgsPointCloudRgbRenderer_setBlueContrastEnhancement_wrapper, QgsPointCloudRgbRenderer.setBlueContrastEnhancement)

    QgsPointCloudRgbRenderer.__group__ = ['pointcloud']
except (NameError, AttributeError):
    pass
