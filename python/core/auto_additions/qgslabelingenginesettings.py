# The following has been generated automatically from src/core/labeling/qgslabelingenginesettings.h
try:
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsLabelingEngineSettings_addRule = QgsLabelingEngineSettings.addRule
    def __QgsLabelingEngineSettings_addRule_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLabelingEngineSettings_addRule(self, arg)
    QgsLabelingEngineSettings.addRule = _functools.update_wrapper(__QgsLabelingEngineSettings_addRule_wrapper, QgsLabelingEngineSettings.addRule)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsLabelingEngineSettings_setRules = QgsLabelingEngineSettings.setRules
    def __QgsLabelingEngineSettings_setRules_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLabelingEngineSettings_setRules(self, arg)
    QgsLabelingEngineSettings.setRules = _functools.update_wrapper(__QgsLabelingEngineSettings_setRules_wrapper, QgsLabelingEngineSettings.setRules)

    QgsLabelingEngineSettings.__group__ = ['labeling']
except (NameError, AttributeError):
    pass
