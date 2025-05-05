# The following has been generated automatically from src/core/qgspythonrunner.h
try:
    QgsPythonRunner.isValid = staticmethod(QgsPythonRunner.isValid)
    QgsPythonRunner.run = staticmethod(QgsPythonRunner.run)
    QgsPythonRunner.runFile = staticmethod(QgsPythonRunner.runFile)
    QgsPythonRunner.eval = staticmethod(QgsPythonRunner.eval)
    QgsPythonRunner.setArgv = staticmethod(QgsPythonRunner.setArgv)
    QgsPythonRunner.setInstance = staticmethod(QgsPythonRunner.setInstance)
    QgsPythonRunner.__abstract_methods__ = ['runCommand', 'runFileCommand', 'evalCommand', 'setArgvCommand']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsPythonRunner_setInstance = QgsPythonRunner.setInstance
    def __QgsPythonRunner_setInstance_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsPythonRunner_setInstance(self, arg)
    QgsPythonRunner.setInstance = _functools.update_wrapper(__QgsPythonRunner_setInstance_wrapper, QgsPythonRunner.setInstance)

except (NameError, AttributeError):
    pass
