# The following has been generated automatically from src/core/qgsdatetimestatisticalsummary.h
QgsDateTimeStatisticalSummary.Count = QgsDateTimeStatisticalSummary.Statistic.Count
QgsDateTimeStatisticalSummary.CountDistinct = QgsDateTimeStatisticalSummary.Statistic.CountDistinct
QgsDateTimeStatisticalSummary.CountMissing = QgsDateTimeStatisticalSummary.Statistic.CountMissing
QgsDateTimeStatisticalSummary.Min = QgsDateTimeStatisticalSummary.Statistic.Min
QgsDateTimeStatisticalSummary.Max = QgsDateTimeStatisticalSummary.Statistic.Max
QgsDateTimeStatisticalSummary.Range = QgsDateTimeStatisticalSummary.Statistic.Range
QgsDateTimeStatisticalSummary.All = QgsDateTimeStatisticalSummary.Statistic.All
QgsDateTimeStatisticalSummary.Statistics = lambda flags=0: QgsDateTimeStatisticalSummary.Statistic(flags)
def _force_int(v): return v if isinstance(v, int) else int(v.value)


QgsDateTimeStatisticalSummary.Statistic.__bool__ = lambda flag: _force_int(flag)
QgsDateTimeStatisticalSummary.Statistic.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsDateTimeStatisticalSummary.Statistic.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsDateTimeStatisticalSummary.Statistic.__or__ = lambda flag1, flag2: QgsDateTimeStatisticalSummary.Statistic(_force_int(flag1) | _force_int(flag2))
