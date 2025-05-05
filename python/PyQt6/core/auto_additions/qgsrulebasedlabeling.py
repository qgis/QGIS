# The following has been generated automatically from src/core/labeling/qgsrulebasedlabeling.h
QgsRuleBasedLabeling.Rule.Filtered = QgsRuleBasedLabeling.Rule.RegisterResult.Filtered
QgsRuleBasedLabeling.Rule.Inactive = QgsRuleBasedLabeling.Rule.RegisterResult.Inactive
QgsRuleBasedLabeling.Rule.Registered = QgsRuleBasedLabeling.Rule.RegisterResult.Registered
try:
    QgsRuleBasedLabeling.Rule.create = staticmethod(QgsRuleBasedLabeling.Rule.create)
    import functools as _functools
    __wrapped_QgsRuleBasedLabeling_Rule_setSettings = QgsRuleBasedLabeling.Rule.setSettings
    def __QgsRuleBasedLabeling_Rule_setSettings_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsRuleBasedLabeling_Rule_setSettings(self, arg)
    QgsRuleBasedLabeling.Rule.setSettings = _functools.update_wrapper(__QgsRuleBasedLabeling_Rule_setSettings_wrapper, QgsRuleBasedLabeling.Rule.setSettings)

    import functools as _functools
    __wrapped_QgsRuleBasedLabeling_Rule_appendChild = QgsRuleBasedLabeling.Rule.appendChild
    def __QgsRuleBasedLabeling_Rule_appendChild_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsRuleBasedLabeling_Rule_appendChild(self, arg)
    QgsRuleBasedLabeling.Rule.appendChild = _functools.update_wrapper(__QgsRuleBasedLabeling_Rule_appendChild_wrapper, QgsRuleBasedLabeling.Rule.appendChild)

    QgsRuleBasedLabeling.Rule.__group__ = ['labeling']
except (NameError, AttributeError):
    pass
try:
    QgsRuleBasedLabeling.create = staticmethod(QgsRuleBasedLabeling.create)
    QgsRuleBasedLabeling.__overridden_methods__ = ['type', 'clone', 'save', 'subProviders', 'settings', 'accept', 'setSettings', 'requiresAdvancedEffects', 'toSld', 'multiplyOpacity']
    import functools as _functools
    __wrapped_QgsRuleBasedLabeling_QgsRuleBasedLabeling = QgsRuleBasedLabeling.QgsRuleBasedLabeling
    def __QgsRuleBasedLabeling_QgsRuleBasedLabeling_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsRuleBasedLabeling_QgsRuleBasedLabeling(self, arg)
    QgsRuleBasedLabeling.QgsRuleBasedLabeling = _functools.update_wrapper(__QgsRuleBasedLabeling_QgsRuleBasedLabeling_wrapper, QgsRuleBasedLabeling.QgsRuleBasedLabeling)

    QgsRuleBasedLabeling.__group__ = ['labeling']
except (NameError, AttributeError):
    pass
