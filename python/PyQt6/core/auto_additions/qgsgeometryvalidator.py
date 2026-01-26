# The following has been generated automatically from src/core/qgsgeometryvalidator.h
try:
    QgsGeometryValidator.__attribute_docs__ = {'errorFound': 'Sent when an error has been found during the validation process.\n\nThe ``error`` contains details about the error.\n', 'validationFinished': 'Sent when the validation is finished.\n\nThe result is in a human readable ``summary``, mentioning if the\nvalidation has been aborted, successfully been validated or how many\nerrors have been found.\n\n.. versionadded:: 3.6\n'}
    QgsGeometryValidator.validateGeometry = staticmethod(QgsGeometryValidator.validateGeometry)
    QgsGeometryValidator.__overridden_methods__ = ['run']
    QgsGeometryValidator.__signal_arguments__ = {'errorFound': ['error: QgsGeometry.Error'], 'validationFinished': ['summary: str']}
except (NameError, AttributeError):
    pass
