#include "qgsruntimeprofiler.h"
#include "qgslogger.h"

QgsRuntimeProfiler::QgsRuntimeProfiler()
{

}

void QgsRuntimeProfiler::start(const QString &name)
{
   mProfileTime.restart();
   mCurrentName = name;
}

void QgsRuntimeProfiler::end()
{
    QString name = mCurrentName.toUpper();
    double timing =  mProfileTime.elapsed() / 1000.0;
    mProfileTimes.append(QPair<QString, double>(name, timing));
    QString message =  QString( "PROFILE: %1 - %2" ).arg(name).arg(timing);
    QgsDebugMsg( message );
}

void QgsRuntimeProfiler::clear()
{
 mProfileTimes.clear();
}

double QgsRuntimeProfiler::totalTime()
{
 double total = 0;
 QList<QPair<QString, double> >::const_iterator it = mProfileTimes.constBegin();
  for ( ; it != mProfileTimes.constEnd(); ++it )
    {
      total += (*it).second;
    }
  return total;
}
