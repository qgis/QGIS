# The following has been generated automatically from src/gui/inputcontroller/qgsinputcontrollermanager.h
try:
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsInputControllerManager_register2DMapController = QgsInputControllerManager.register2DMapController
    def __QgsInputControllerManager_register2DMapController_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsInputControllerManager_register2DMapController(self, arg)
    QgsInputControllerManager.register2DMapController = _functools.update_wrapper(__QgsInputControllerManager_register2DMapController_wrapper, QgsInputControllerManager.register2DMapController)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsInputControllerManager_register3DMapController = QgsInputControllerManager.register3DMapController
    def __QgsInputControllerManager_register3DMapController_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsInputControllerManager_register3DMapController(self, arg)
    QgsInputControllerManager.register3DMapController = _functools.update_wrapper(__QgsInputControllerManager_register3DMapController_wrapper, QgsInputControllerManager.register3DMapController)

    QgsInputControllerManager.__group__ = ['inputcontroller']
except (NameError, AttributeError):
    pass
