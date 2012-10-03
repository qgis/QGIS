from PyQt4.QtCore import *

from qgis.core import *

from sextante.core.SextanteLog import SextanteLog

def buffering(progress, writer, distance, field, useSelection, useField, layer, dissolve, segments):
    GEOS_EXCEPT = True
    FEATURE_EXCEPT = True

    layer.select(layer.pendingAllAttributesList())

    if useField:
        field = layer.fieldNameIndex(field)

    outFeat = QgsFeature()
    inFeat = QgsFeature()
    inGeom = QgsGeometry()
    outGeom = QgsGeometry()

    current = 0

    # there is selection in input layer
    if useSelection:
        total = 100.0 / float(layer.selectedFeatureCount())
        selection = layer.selectedFeatures()

        # with dissolve
        if dissolve:
            first = True
            for inFeat in selection:
                atMap = inFeat.attributeMap()
                if useField:
                    value = atMap[field].doDouble()[0]
                else:
                    value = distance

                inGeom = QgsGeometry(inFeat.geometry())
                try:
                    outGeom = inGeom.buffer(float(value), segments)
                    if first:
                      tempGeom = QgsGeometry(outGeom)
                      first = False
                    else:
                      try:
                        tempGeom = tempGeom.combine( outGeom )
                      except:
                        GEOS_EXCEPT = False
                        continue
                except:
                    GEOS_EXCEPT = False
                    continue

                current += 1
                progress.setPercentage(int(current * total))
            try:
                outFeat.setGeometry(tempGeom)
                writer.addFeature(outFeat)
            except:
                FEATURE_EXCEPT = False
        # without dissolve
        else:
            for inFeat in selection:
                atMap = inFeat.attributeMap()
                if useField:
                    value = atMap[field].toDouble()[0]
                else:
                    value = distance

                inGeom = QgsGeometry(inFeat.geometry())
                try:
                    outGeom = inGeom.buffer(float(value), segments)
                    try:
                        outFeat.setGeometry(outGeom)
                        outFeat.setAttributeMap(atMap)
                        writer.addFeature(outFeat)
                    except:
                        FEATURE_EXCEPT = False
                        continue
                except:
                    GEOS_EXCEPT = False
                    continue

                current += 1
                progress.setPercentage(int(current * total))
    # there is no selection in input layer
    else:
      total = 100.0 / float(layer.featureCount())

      # with dissolve
      if dissolve:
          first = True
          while layer.nextFeature(inFeat):
              atMap = inFeat.attributeMap()
              if useField:
                  value = atMap[field].toDouble()[0]
              else:
                  value = distance

              inGeom = QgsGeometry(inFeat.geometry())
              try:
                  outGeom = inGeom.buffer(float(value), segments)
                  if first:
                      tempGeom = QgsGeometry(outGeom)
                      first = False
                  else:
                      try:
                        tempGeom = tempGeom.combine(outGeom)
                      except:
                        GEOS_EXCEPT = False
                        continue
              except:
                  GEOS_EXCEPT = False
                  continue

              current += 1
              progress.setPercentage(int(current * total))
          try:
              outFeat.setGeometry(tempGeom)
              writer.addFeature(outFeat)
          except:
              FEATURE_EXCEPT = False
      # without dissolve
      else:
          while layer.nextFeature(inFeat):
              atMap = inFeat.attributeMap()
              if useField:
                  value = atMap[field].toDouble()[0]
              else:
                  value = distance

              inGeom = QgsGeometry(inFeat.geometry())
              try:
                  outGeom = inGeom.buffer(float(value), segments)
                  try:
                      outFeat.setGeometry(outGeom)
                      outFeat.setAttributeMap(atMap)
                      writer.addFeature(outFeat)
                  except:
                      FEATURE_EXCEPT = False
                      continue
              except:
                  GEOS_EXCEPT = False
                  continue

              current += 1
              progress.setPercentage(int(current * total))

    del writer

    if not GEOS_EXCEPT:
        SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Geometry exception while computing buffer")
    if not FEATURE_EXCEPT:
        SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Feature exception while computing buffer")
