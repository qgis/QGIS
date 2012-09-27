from PyQt4.QtCore import *

from qgis.core import *

def createSpatialIndex(provider):
    ft = QgsFeature()
    idx = QgsSpatialIndex()
    provider.rewind()
    provider.select()
    while provider.nextFeature( ft ):
        idx.insertFeature( ft )
    return idx

def createUniqueFieldName(fieldName, fieldList):
    shortName = fieldName[:10]

    if len(fieldList) == 0:
        return shortName

    if shortName not in fieldList:
        return shortName

    shortName = fieldName[:8] + "_1"
    changed = True
    while changed:
        changed = False
        for n in fieldList:
            if n == shortName:
                # create unique field name
                num = int(shortName[-1:])
                if num < 9:
                    shortName = shortName[:8] + "_" + str(num + 1)
                else:
                    shortName = shortName[:7] + "_" + str(num + 1)

                changed = True

    return shortName

def findOrCreateField(layer, fieldList, fieldName, fieldLen = 24, fieldPrec = 15):
    idx = layer.fieldNameIndex(fieldName)
    if idx == -1:
        idx = len(fieldList)
        if idx == max(fieldList.keys()):
            idx += 1
        fn = createUniqueFieldName(fieldName, fieldList)
        field =  QgsField(fn, QVariant.Double, "", fieldLen, fieldPrec)
        fieldList[idx] = field
    return idx, fieldList
