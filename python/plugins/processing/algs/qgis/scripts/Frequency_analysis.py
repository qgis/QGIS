##Table=group
##Input=vector
##Fields=Field Input
##Frequency=output table

from processing.tools.vector import TableWriter
from collections import defaultdict
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException

layer = processing.getObject(Input)
inputFields = layer.pendingFields()
fieldIdxs = []
fields = Fields.split(',')
for f in fields:
    idx = inputFields.indexFromName(f)
    if idx == -1:
        raise GeoAlgorithmExecutionException('Field not found:' + f)
    fieldIdxs.append(idx)
writer = TableWriter(Frequency, None, fields + ['FREQ'])

counts = {}
feats = processing.features(layer)
nFeats = len(feats)
counts = defaultdict(int)
for i, feat in enumerate(feats):
    progress.setPercentage(int(100 * i / nFeats))
    attrs = feat.attributes()
    clazz = tuple([attrs[i] for i in fieldIdxs])
    counts[clazz] += 1

for c in counts:
    writer.addRecord(list(c) + [counts[c]])
