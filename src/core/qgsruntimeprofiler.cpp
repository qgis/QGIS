#include "qgsruntimeprofiler.h"
#include "qgslogger.h"


QgsRuntimeProfiler* QgsRuntimeProfiler::mInstance = nullptr;

QgsRuntimeProfiler* QgsRuntimeProfiler::instance()
{
  if ( !mInstance )
    mInstance = new QgsRuntimeProfiler();
  return mInstance;
}

QgsRuntimeProfiler::QgsRuntimeProfiler()
{

}

void QgsRuntimeProfiler::beginGroup( const QString &name )
{
  mGroupStack.push( name );
  if ( !name.isEmpty() )
  {
    mGroupPrefix += name;
    mGroupPrefix += QLatin1Char( '/' );
  }
}

void QgsRuntimeProfiler::endGroup()
{
  if ( mGroupStack.isEmpty() )
  {
    qWarning( "QSettings::endGroup: No matching beginGroup()" );
    return;
  }

  QString group = mGroupStack.pop();
  int len = group.size();
  if ( len > 0 )
    mGroupPrefix.truncate( mGroupPrefix.size() - ( len + 1 ) );
}

void QgsRuntimeProfiler::start( const QString &name )
{
  mProfileTime.restart();
  mCurrentName = name;
}

void QgsRuntimeProfiler::end()
{
  QString name = mCurrentName;
  name.prepend( mGroupPrefix );
  double timing =  mProfileTime.elapsed() / 1000.0;
  mProfileTimes.append( QPair<QString, double>( name, timing ) );
  QString message =  QString( "PROFILE: %1 - %2" ).arg( name ).arg( timing );
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
    total += ( *it ).second;
  }
  return total;
}
