# The following has been generated automatically from src/analysis/georeferencing/qgsvectorwarper.h
# monkey patching scoped based enum
QgsVectorWarperTask.Result.Success.__doc__ = "Warping completed successfully"
QgsVectorWarperTask.Success = QgsVectorWarperTask.Result.Success
QgsVectorWarperTask.Result.Canceled.__doc__ = "Task was canceled before completion"
QgsVectorWarperTask.Canceled = QgsVectorWarperTask.Result.Canceled
QgsVectorWarperTask.Result.Error.__doc__ = "An error occurred while warping"
QgsVectorWarperTask.Error = QgsVectorWarperTask.Result.Error
QgsVectorWarperTask.Result.__doc__ = "Task results\n\n" + '* ``Success``: ' + QgsVectorWarperTask.Result.Success.__doc__ + '\n' + '* ``Canceled``: ' + QgsVectorWarperTask.Result.Canceled.__doc__ + '\n' + '* ``Error``: ' + QgsVectorWarperTask.Result.Error.__doc__
# --
