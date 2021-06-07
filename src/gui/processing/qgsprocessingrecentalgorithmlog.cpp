/***************************************************************************
                             qgsprocessingrecentalgorithmlog.cpp
                             ------------------------------------
    Date                 : July 2018
    Copyright            : (C) 2018 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingrecentalgorithmlog.h"
#include "qgssettings.h"

///@cond PRIVATE

const int MAX_LOG_LENGTH = 5;

QgsProcessingRecentAlgorithmLog::QgsProcessingRecentAlgorithmLog( QObject *parent )
  : QObject( parent )
{
  QgsSettings settings;
  mRecentAlgorithmIds = settings.value( QStringLiteral( "processing/recentAlgorithms" ), QVariant(), QgsSettings::Gui ).toStringList();
}

QStringList QgsProcessingRecentAlgorithmLog::recentAlgorithmIds() const
{
  return mRecentAlgorithmIds;
}

void QgsProcessingRecentAlgorithmLog::push( const QString &id )
{
  const QStringList previous = mRecentAlgorithmIds;
  mRecentAlgorithmIds.removeAll( id );
  mRecentAlgorithmIds.insert( 0, id );
  if ( mRecentAlgorithmIds.length() > MAX_LOG_LENGTH )
    mRecentAlgorithmIds = mRecentAlgorithmIds.mid( 0, MAX_LOG_LENGTH );

  QgsSettings settings;
  settings.setValue( QStringLiteral( "processing/recentAlgorithms" ), mRecentAlgorithmIds, QgsSettings::Gui );

  if ( previous != mRecentAlgorithmIds )
    emit changed();
}

///@endcond
