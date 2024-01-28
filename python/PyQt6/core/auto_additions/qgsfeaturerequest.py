# The following has been generated automatically from src/core/qgsfeaturerequest.h
QgsFeatureRequest.NoFlags = QgsFeatureRequest.Flag.NoFlags
QgsFeatureRequest.NoGeometry = QgsFeatureRequest.Flag.NoGeometry
QgsFeatureRequest.SubsetOfAttributes = QgsFeatureRequest.Flag.SubsetOfAttributes
QgsFeatureRequest.ExactIntersect = QgsFeatureRequest.Flag.ExactIntersect
QgsFeatureRequest.IgnoreStaticNodesDuringExpressionCompilation = QgsFeatureRequest.Flag.IgnoreStaticNodesDuringExpressionCompilation
QgsFeatureRequest.EmbeddedSymbols = QgsFeatureRequest.Flag.EmbeddedSymbols
QgsFeatureRequest.Flags = lambda flags=0: QgsFeatureRequest.Flag(flags)
QgsFeatureRequest.FilterNone = QgsFeatureRequest.FilterType.FilterNone
QgsFeatureRequest.FilterFid = QgsFeatureRequest.FilterType.FilterFid
QgsFeatureRequest.FilterExpression = QgsFeatureRequest.FilterType.FilterExpression
QgsFeatureRequest.FilterFids = QgsFeatureRequest.FilterType.FilterFids
QgsFeatureRequest.GeometryNoCheck = QgsFeatureRequest.InvalidGeometryCheck.GeometryNoCheck
QgsFeatureRequest.GeometrySkipInvalid = QgsFeatureRequest.InvalidGeometryCheck.GeometrySkipInvalid
QgsFeatureRequest.GeometryAbortOnInvalid = QgsFeatureRequest.InvalidGeometryCheck.GeometryAbortOnInvalid
_force_int = lambda v: v if isinstance(v, int) else int(v.value)


QgsFeatureRequest.Flag.__bool__ = lambda flag: _force_int(flag)
QgsFeatureRequest.Flag.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsFeatureRequest.Flag.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsFeatureRequest.Flag.__or__ = lambda flag1, flag2: QgsFeatureRequest.Flag(_force_int(flag1) | _force_int(flag2))
