# The following has been generated automatically from src/core/qgsmapsettingsutils.h
# monkey patching scoped based enum
QgsMapSettingsUtils.EffectsCheckFlag.IgnoreGeoPdfSupportedEffects.__doc__ = "Ignore advanced effects which are supported in GeoPDF exports"
QgsMapSettingsUtils.EffectsCheckFlag.__doc__ = """Flags for controlling the behavior of :py:func:`~QgsMapSettingsUtils.containsAdvancedEffects`

.. versionadded:: 3.14

* ``IgnoreGeoPdfSupportedEffects``: Ignore advanced effects which are supported in GeoPDF exports

"""
# --
try:
    QgsMapSettingsUtils.containsAdvancedEffects = staticmethod(QgsMapSettingsUtils.containsAdvancedEffects)
    QgsMapSettingsUtils.worldFileParameters = staticmethod(QgsMapSettingsUtils.worldFileParameters)
    QgsMapSettingsUtils.worldFileContent = staticmethod(QgsMapSettingsUtils.worldFileContent)
except NameError:
    pass
