# The following has been generated automatically from src/core/qgsfeatureiterator.h
# monkey patching scoped based enum
QgsAbstractFeatureIterator.RequestToSourceCrsResult.Success.__doc__ = "Request was successfully updated to the source CRS, or no changes were required"
QgsAbstractFeatureIterator.RequestToSourceCrsResult.DistanceWithinMustBeCheckedManually.__doc__ = "The distance within request cannot be losslessly updated to the source CRS, and callers will need to take appropriate steps to handle the distance within requirement manually during feature iteration"
QgsAbstractFeatureIterator.RequestToSourceCrsResult.__doc__ = """Possible results from the :py:func:`~QgsAbstractFeatureIterator.updateRequestToSourceCrs` method.

.. versionadded:: 3.22

* ``Success``: Request was successfully updated to the source CRS, or no changes were required
* ``DistanceWithinMustBeCheckedManually``: The distance within request cannot be losslessly updated to the source CRS, and callers will need to take appropriate steps to handle the distance within requirement manually during feature iteration

"""
# --
