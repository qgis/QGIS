# The following has been generated automatically from src/core/qgsgeometryvalidator.h
try:
    QgsGeometryValidator.__attribute_docs__ = {'errorFound': 'Sent when an error has been found during the validation process.\n\nThe ``error`` contains details about the error.\n', 'validationFinished': 'Sent when the validation is finished.\n\nThe result is in a human readable ``summary``, mentioning\nif the validation has been aborted, successfully been validated\nor how many errors have been found.\n\n.. versionadded:: 3.6\n'}
    QgsGeometryValidator.validateGeometry = staticmethod(QgsGeometryValidator.validateGeometry)
    QgsGeometryValidator.__signal_arguments__ = {'errorFound': ['error: QgsGeometry.Error'], 'validationFinished': ['summary: str']}
except (NameError, AttributeError):
    pass
