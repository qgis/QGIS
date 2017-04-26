##Table=group
##Input=vector
##Fields=Field Input
##Frequency=output table

from processing.tools.vector import TableWriter
from collections import defaultdict
from qgis.core import QgsProcessingUtils
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.tools import dataobjects

layer = dataobjects.getLayerFromString(Input)
inputFields = layer.fields()
fieldIdxs = []
fields = Fields.split(',')
for f in fields:
    idx = inputFields.indexFromName(f)
    if idx == -1:
        raise GeoAlgorithmExecutionException('Field not found:' + f)
    fieldIdxs.append(idx)
writer = TableWriter(Frequency, None, fields + ['FREQ'])

counts = {}
feats = QgsProcessingUtils.getFeatures(layer, context)
nFeats = QgsProcessingUtils.featureCount(layer, context)
counts = defaultdict(int)
for i, feat in enumerate(feats):
    feedback.setProgress(int(100 * i / nFeats))
    attrs = feat.attributes()
    clazz = tuple([attrs[i] for i in fieldIdxs])
    counts[clazz] += 1

for c in counts:
    writer.addRecord(list(c) + [counts[c]])
