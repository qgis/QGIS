/***************************************************************************
                         qgsprocessingfeedback.cpp
                         -------------------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingfeedback.h"
#include "qgsgeos.h"
#include "qgsprocessingprovider.h"
#include <ogr_api.h>
#include <gdal_version.h>
#include <proj.h>

#ifdef HAVE_PDAL_QGIS
#include <pdal/pdal.hpp>
#endif

QgsProcessingFeedback::QgsProcessingFeedback( bool logFeedback )
  : mLogFeedback( logFeedback )
{

}

void QgsProcessingFeedback::setProgressText( const QString & )
{
}

void QgsProcessingFeedback::log( const QString &htmlMessage, const QString &textMessage )
{
  constexpr int MESSAGE_COUNT_LIMIT = 10000;
  // Avoid logging too many messages, which might blow memory.
  if ( mMessageLoggedCount == MESSAGE_COUNT_LIMIT )
    return;
  ++mMessageLoggedCount;
  if ( mMessageLoggedCount == MESSAGE_COUNT_LIMIT )
  {
    mHtmlLog.append( QStringLiteral( "<span style=\"color:red\">%1</span><br/>" ).arg( tr( "Message log truncated" ) ) );
    mTextLog.append( tr( "Message log truncated" ) + '\n' );
  }
  else
  {
    mHtmlLog.append( htmlMessage );
    mTextLog.append( textMessage );
  }
}


void QgsProcessingFeedback::reportError( const QString &error, bool )
{
  if ( mLogFeedback )
    QgsMessageLog::logMessage( error, tr( "Processing" ), Qgis::MessageLevel::Critical );

  log( QStringLiteral( "<span style=\"color:red\">%1</span><br/>" ).arg( error.toHtmlEscaped() ).replace( '\n', QLatin1String( "<br>" ) ),
       error + '\n' );
}

void QgsProcessingFeedback::pushWarning( const QString &warning )
{
  if ( mLogFeedback )
    QgsMessageLog::logMessage( warning, tr( "Processing" ), Qgis::MessageLevel::Warning );

  log( QStringLiteral( "<span style=\"color:#b85a20;\">%1</span><br/>" ).arg( warning.toHtmlEscaped() ).replace( '\n', QLatin1String( "<br>" ) ) + QStringLiteral( "<br/>" ),
       warning + '\n' );
}

void QgsProcessingFeedback::pushInfo( const QString &info )
{
  if ( mLogFeedback )
    QgsMessageLog::logMessage( info, tr( "Processing" ), Qgis::MessageLevel::Info );

  mHtmlLog.append( info.toHtmlEscaped().replace( '\n', QLatin1String( "<br>" ) ) + QStringLiteral( "<br/>" ) );
  mTextLog.append( info + '\n' );
}

void QgsProcessingFeedback::pushCommandInfo( const QString &info )
{
  if ( mLogFeedback )
    QgsMessageLog::logMessage( info, tr( "Processing" ), Qgis::MessageLevel::Info );

  log( QStringLiteral( "<code>%1</code><br/>" ).arg( info.toHtmlEscaped().replace( '\n', QLatin1String( "<br>" ) ) ),
       info + '\n' );
}

void QgsProcessingFeedback::pushDebugInfo( const QString &info )
{
  if ( mLogFeedback )
    QgsMessageLog::logMessage( info, tr( "Processing" ), Qgis::MessageLevel::Info );

  log( QStringLiteral( "<span style=\"color:#777\">%1</span><br/>" ).arg( info.toHtmlEscaped().replace( '\n', QLatin1String( "<br>" ) ) ),
       info + '\n' );
}

void QgsProcessingFeedback::pushConsoleInfo( const QString &info )
{
  if ( mLogFeedback )
    QgsMessageLog::logMessage( info, tr( "Processing" ), Qgis::MessageLevel::Info );

  log( QStringLiteral( "<code style=\"color:#777\">%1</code><br/>" ).arg( info.toHtmlEscaped().replace( '\n', QLatin1String( "<br>" ) ) ),
       info + '\n' );
}

void QgsProcessingFeedback::pushVersionInfo( const QgsProcessingProvider *provider )
{
  pushDebugInfo( tr( "QGIS version: %1" ).arg( Qgis::version() ) );
  if ( QString( Qgis::devVersion() ) != QLatin1String( "exported" ) )
  {
    pushDebugInfo( tr( "QGIS code revision: %1" ).arg( Qgis::devVersion() ) );
  }
  pushDebugInfo( tr( "Qt version: %1" ).arg( qVersion() ) );
  pushDebugInfo( tr( "Python version: %1" ).arg( PYTHON_VERSION ) );
  pushDebugInfo( tr( "GDAL version: %1" ).arg( GDALVersionInfo( "RELEASE_NAME" ) ) );
  pushDebugInfo( tr( "GEOS version: %1" ).arg( GEOSversion() ) );

  const PJ_INFO info = proj_info();
  pushDebugInfo( tr( "PROJ version: %1" ).arg( info.release ) );

#ifdef HAVE_PDAL_QGIS
#if PDAL_VERSION_MAJOR_INT > 1 || (PDAL_VERSION_MAJOR_INT == 1 && PDAL_VERSION_MINOR_INT >= 7)
  pushDebugInfo( tr( "PDAL version: %1" ).arg( QString::fromStdString( pdal::Config::fullVersionString() ) ) );
#else
  pushDebugInfo( tr( "PDAL version: %1" ).arg( QString::fromStdString( pdal::GetFullVersionString() ) ) );
#endif
#endif

  if ( provider && !provider->versionInfo().isEmpty() )
  {
    pushDebugInfo( tr( "%1 version: %2" ).arg( provider->name(), provider->versionInfo() ) );
  }
}

QString QgsProcessingFeedback::htmlLog() const
{
  return mHtmlLog;
}

QString QgsProcessingFeedback::textLog() const
{
  return mTextLog;
}


QgsProcessingMultiStepFeedback::QgsProcessingMultiStepFeedback( int childAlgorithmCount, QgsProcessingFeedback *feedback )
  : mChildSteps( childAlgorithmCount )
  , mFeedback( feedback )
{
  connect( mFeedback, &QgsFeedback::canceled, this, &QgsFeedback::cancel, Qt::DirectConnection );
  connect( this, &QgsFeedback::progressChanged, this, &QgsProcessingMultiStepFeedback::updateOverallProgress );
}

void QgsProcessingMultiStepFeedback::setCurrentStep( int step )
{
  mCurrentStep = step;
  mFeedback->setProgress( 100.0 * static_cast< double >( mCurrentStep ) / mChildSteps );
}

void QgsProcessingMultiStepFeedback::setProgressText( const QString &text )
{
  mFeedback->setProgressText( text );
}

void QgsProcessingMultiStepFeedback::reportError( const QString &error, bool fatalError )
{
  mFeedback->reportError( error, fatalError );
}

void QgsProcessingMultiStepFeedback::pushWarning( const QString &warning )
{
  mFeedback->pushWarning( warning );
}

void QgsProcessingMultiStepFeedback::pushInfo( const QString &info )
{
  mFeedback->pushInfo( info );
}

void QgsProcessingMultiStepFeedback::pushCommandInfo( const QString &info )
{
  mFeedback->pushCommandInfo( info );
}

void QgsProcessingMultiStepFeedback::pushDebugInfo( const QString &info )
{
  mFeedback->pushDebugInfo( info );
}

void QgsProcessingMultiStepFeedback::pushConsoleInfo( const QString &info )
{
  mFeedback->pushConsoleInfo( info );
}

QString QgsProcessingMultiStepFeedback::htmlLog() const
{
  return mFeedback->htmlLog();
}

QString QgsProcessingMultiStepFeedback::textLog() const
{
  return mFeedback->textLog();
}

void QgsProcessingMultiStepFeedback::updateOverallProgress( double progress )
{
  const double baseProgress = 100.0 * static_cast< double >( mCurrentStep ) / mChildSteps;
  const double currentAlgorithmProgress = progress / mChildSteps;
  mFeedback->setProgress( baseProgress + currentAlgorithmProgress );
}

