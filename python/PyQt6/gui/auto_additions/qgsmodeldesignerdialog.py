# The following has been generated automatically from src/gui/processing/models/qgsmodeldesignerdialog.h
# monkey patching scoped based enum
QgsModelDesignerDialog.SaveAction.SaveAsFile.__doc__ = "Save model as a file"
QgsModelDesignerDialog.SaveAction.SaveInProject.__doc__ = "Save model into project"
QgsModelDesignerDialog.SaveAction.__doc__ = """Save action.

.. versionadded:: 3.24

* ``SaveAsFile``: Save model as a file
* ``SaveInProject``: Save model into project

"""
# --
try:
    QgsModelDesignerDialog.__abstract_methods__ = ['repaintModel', 'addAlgorithm', 'addInput', 'exportAsScriptAlgorithm', 'saveModel', 'createExecutionDialog']
    QgsModelDesignerDialog.__overridden_methods__ = ['closeEvent']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsModelDesignerDialog_setModel = QgsModelDesignerDialog.setModel
    def __QgsModelDesignerDialog_setModel_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsModelDesignerDialog_setModel(self, arg)
    QgsModelDesignerDialog.setModel = _functools.update_wrapper(__QgsModelDesignerDialog_setModel_wrapper, QgsModelDesignerDialog.setModel)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsModelDesignerDialog_setModelScene = QgsModelDesignerDialog.setModelScene
    def __QgsModelDesignerDialog_setModelScene_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsModelDesignerDialog_setModelScene(self, arg)
    QgsModelDesignerDialog.setModelScene = _functools.update_wrapper(__QgsModelDesignerDialog_setModelScene_wrapper, QgsModelDesignerDialog.setModelScene)

    QgsModelDesignerDialog.__group__ = ['processing', 'models']
except (NameError, AttributeError):
    pass
try:
    QgsModelChildDependenciesWidget.__group__ = ['processing', 'models']
except (NameError, AttributeError):
    pass
