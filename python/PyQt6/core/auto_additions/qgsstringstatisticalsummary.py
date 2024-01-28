# The following has been generated automatically from src/core/qgsstringstatisticalsummary.h
QgsStringStatisticalSummary.Count = QgsStringStatisticalSummary.Statistic.Count
QgsStringStatisticalSummary.CountDistinct = QgsStringStatisticalSummary.Statistic.CountDistinct
QgsStringStatisticalSummary.CountMissing = QgsStringStatisticalSummary.Statistic.CountMissing
QgsStringStatisticalSummary.Min = QgsStringStatisticalSummary.Statistic.Min
QgsStringStatisticalSummary.Max = QgsStringStatisticalSummary.Statistic.Max
QgsStringStatisticalSummary.MinimumLength = QgsStringStatisticalSummary.Statistic.MinimumLength
QgsStringStatisticalSummary.MaximumLength = QgsStringStatisticalSummary.Statistic.MaximumLength
QgsStringStatisticalSummary.MeanLength = QgsStringStatisticalSummary.Statistic.MeanLength
QgsStringStatisticalSummary.Minority = QgsStringStatisticalSummary.Statistic.Minority
QgsStringStatisticalSummary.Majority = QgsStringStatisticalSummary.Statistic.Majority
QgsStringStatisticalSummary.All = QgsStringStatisticalSummary.Statistic.All
QgsStringStatisticalSummary.Statistics = lambda flags=0: QgsStringStatisticalSummary.Statistic(flags)
def _force_int(v): return v if isinstance(v, int) else int(v.value)


QgsStringStatisticalSummary.Statistic.__bool__ = lambda flag: _force_int(flag)
QgsStringStatisticalSummary.Statistic.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsStringStatisticalSummary.Statistic.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsStringStatisticalSummary.Statistic.__or__ = lambda flag1, flag2: QgsStringStatisticalSummary.Statistic(_force_int(flag1) | _force_int(flag2))
