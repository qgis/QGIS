# The following has been generated automatically from src/core/labeling/rules/qgslabelingengineruleregistry.h
try:
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsLabelingEngineRuleRegistry_addRule = QgsLabelingEngineRuleRegistry.addRule
    def __QgsLabelingEngineRuleRegistry_addRule_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLabelingEngineRuleRegistry_addRule(self, arg)
    QgsLabelingEngineRuleRegistry.addRule = _functools.update_wrapper(__QgsLabelingEngineRuleRegistry_addRule_wrapper, QgsLabelingEngineRuleRegistry.addRule)

    QgsLabelingEngineRuleRegistry.__group__ = ['labeling', 'rules']
except (NameError, AttributeError):
    pass
