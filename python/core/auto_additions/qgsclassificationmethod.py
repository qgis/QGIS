# The following has been generated automatically from src/core/classification/qgsclassificationmethod.h
try:
    QgsClassificationMethod.rangesToBreaks = staticmethod(QgsClassificationMethod.rangesToBreaks)
    QgsClassificationMethod.create = staticmethod(QgsClassificationMethod.create)
    QgsClassificationMethod.makeBreaksSymmetric = staticmethod(QgsClassificationMethod.makeBreaksSymmetric)
    QgsClassificationMethod.__virtual_methods__ = ['icon', 'writeXml', 'readXml', 'valuesRequired']
    QgsClassificationMethod.__abstract_methods__ = ['clone', 'name', 'id']
    QgsClassificationMethod.__group__ = ['classification']
except (NameError, AttributeError):
    pass
try:
    QgsClassificationRange.__group__ = ['classification']
except (NameError, AttributeError):
    pass
