# The following has been generated automatically from src/gui/settings/qgssettingseditorwidgetregistry.h
try:
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsSettingsEditorWidgetRegistry_addWrapper = QgsSettingsEditorWidgetRegistry.addWrapper
    def __QgsSettingsEditorWidgetRegistry_addWrapper_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsSettingsEditorWidgetRegistry_addWrapper(self, arg)
    QgsSettingsEditorWidgetRegistry.addWrapper = _functools.update_wrapper(__QgsSettingsEditorWidgetRegistry_addWrapper_wrapper, QgsSettingsEditorWidgetRegistry.addWrapper)

    QgsSettingsEditorWidgetRegistry.__group__ = ['settings']
except (NameError, AttributeError):
    pass
