#ifndef QGSRUNTIMEPROFILER_H
#define QGSRUNTIMEPROFILER_H

#include <QTime>
#include <QPair>


class CORE_EXPORT QgsRuntimeProfiler
{
public:
  QgsRuntimeProfiler();

  void start( const QString& name );

  void end();

  const QList<QPair<QString, double>> profileTimes() { return mProfileTimes; }

  void clear();

  double totalTime();

private:
  QTime mProfileTime;
  QString mCurrentName;
  QList<QPair<QString, double>> mProfileTimes;
};

#endif // QGSRUNTIMEPROFILER_H
