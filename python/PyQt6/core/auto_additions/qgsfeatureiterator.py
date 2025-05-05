# The following has been generated automatically from src/core/qgsfeatureiterator.h
QgsAbstractFeatureIterator.NoCompilation = QgsAbstractFeatureIterator.CompileStatus.NoCompilation
QgsAbstractFeatureIterator.PartiallyCompiled = QgsAbstractFeatureIterator.CompileStatus.PartiallyCompiled
QgsAbstractFeatureIterator.Compiled = QgsAbstractFeatureIterator.CompileStatus.Compiled
# monkey patching scoped based enum
QgsAbstractFeatureIterator.RequestToSourceCrsResult.Success.__doc__ = "Request was successfully updated to the source CRS, or no changes were required"
QgsAbstractFeatureIterator.RequestToSourceCrsResult.DistanceWithinMustBeCheckedManually.__doc__ = "The distance within request cannot be losslessly updated to the source CRS, and callers will need to take appropriate steps to handle the distance within requirement manually during feature iteration"
QgsAbstractFeatureIterator.RequestToSourceCrsResult.__doc__ = """Possible results from the :py:func:`~QgsAbstractFeatureIterator.updateRequestToSourceCrs` method.

.. versionadded:: 3.22

* ``Success``: Request was successfully updated to the source CRS, or no changes were required
* ``DistanceWithinMustBeCheckedManually``: The distance within request cannot be losslessly updated to the source CRS, and callers will need to take appropriate steps to handle the distance within requirement manually during feature iteration

"""
# --
try:
    QgsAbstractFeatureIterator.__virtual_methods__ = ['nextFeature', 'isValid', 'nextFeatureFilterExpression', 'nextFeatureFilterFids', 'prepareSimplification']
    QgsAbstractFeatureIterator.__abstract_methods__ = ['rewind', 'close', 'fetchFeature']
except (NameError, AttributeError):
    pass
try:
    import functools as _functools
    __wrapped_QgsFeatureIterator_QgsFeatureIterator = QgsFeatureIterator.QgsFeatureIterator
    def __QgsFeatureIterator_QgsFeatureIterator_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsFeatureIterator_QgsFeatureIterator(self, arg)
    QgsFeatureIterator.QgsFeatureIterator = _functools.update_wrapper(__QgsFeatureIterator_QgsFeatureIterator_wrapper, QgsFeatureIterator.QgsFeatureIterator)

except (NameError, AttributeError):
    pass
