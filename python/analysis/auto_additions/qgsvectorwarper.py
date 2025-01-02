# The following has been generated automatically from src/analysis/georeferencing/qgsvectorwarper.h
# monkey patching scoped based enum
QgsVectorWarperTask.Result.Success.__doc__ = "Warping completed successfully"
QgsVectorWarperTask.Result.Canceled.__doc__ = "Task was canceled before completion"
QgsVectorWarperTask.Result.Error.__doc__ = "An error occurred while warping"
QgsVectorWarperTask.Result.__doc__ = """Task results

* ``Success``: Warping completed successfully
* ``Canceled``: Task was canceled before completion
* ``Error``: An error occurred while warping

"""
# --
try:
    QgsVectorWarper.__group__ = ['georeferencing']
except (NameError, AttributeError):
    pass
try:
    QgsVectorWarperTask.__group__ = ['georeferencing']
except (NameError, AttributeError):
    pass
