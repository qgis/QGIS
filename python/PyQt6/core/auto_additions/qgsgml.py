# The following has been generated automatically from src/core/qgsgml.h
try:
    QgsGml.__attribute_docs__ = {'dataReadProgress': 'Emitted when data reading progresses.\n\n:param progress: specifies the number of bytes processed so far\n', 'totalStepsUpdate': 'Emitted when the total number of bytes to read changes.\n\n:param totalSteps: specifies the total number of bytes which must be processed\n', 'dataProgressAndSteps': 'Emitted when data reading progresses or the total number of bytes to read changes.\n\n:param progress: specifies the number of bytes processed so far\n:param totalSteps: specifies the total number of bytes which must be processed\n'}
    QgsGml.__signal_arguments__ = {'dataReadProgress': ['progress: int'], 'totalStepsUpdate': ['totalSteps: int'], 'dataProgressAndSteps': ['progress: int', 'totalSteps: int']}
except (NameError, AttributeError):
    pass
