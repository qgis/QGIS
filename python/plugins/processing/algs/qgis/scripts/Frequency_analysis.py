##Vector analysis=group

#inputs

##Input=source
##Fields=field multiple Input
##Frequency=sink table


from processing.tools.vector import TableWriter
from collections import defaultdict
from qgis.core import QgsProcessingUtils, QgsFields, QgsField, QgsWkbTypes, QgsFeature
from qgis.PyQt.QtCore import QVariant
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException

inputFields = Input.fields()
fieldIdxs = []
out_fields = QgsFields()
for f in Fields:
    idx = inputFields.indexFromName(f)
    if idx == -1:
        raise GeoAlgorithmExecutionException('Field not found:' + f)
    fieldIdxs.append(idx)
    out_fields.append(inputFields.at(idx))

out_fields.append(QgsField('FREQ', QVariant.Int))

(sink, Frequency) = self.parameterAsSink(parameters, 'Frequency', context,
                                         out_fields)

counts = {}
feats = Input.getFeatures()
nFeats = Input.featureCount()
counts = defaultdict(int)
for i, feat in enumerate(feats):
    feedback.setProgress(int(100 * i / nFeats))
    if feedback.isCanceled():
        break

    attrs = feat.attributes()
    clazz = tuple([attrs[i] for i in fieldIdxs])
    counts[clazz] += 1

for c in counts:
    f = QgsFeature()
    f.setAttributes(list(c) + [counts[c]])
    sink.addFeature(f)
