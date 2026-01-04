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

#include <gdal_version.h>
#include <ogr_api.h>
#include <proj.h>

#include "qgsgeos.h"
#include "qgsmessagelog.h"
#include "qgsprocessingprovider.h"

#include "moc_qgsprocessingfeedback.cpp"

#ifdef HAVE_PDAL_QGIS
#include <pdal/pdal.hpp>
#endif

#ifdef WITH_SFCGAL
#include <SFCGAL/capi/sfcgal_c.h>
#endif

#ifdef WITH_GEOGRAPHICLIB
#include <GeographicLib/Constants.hpp>
#endif

QgsProcessingFeedback::QgsProcessingFeedback( bool logFeedback )
  : mLogFeedback( logFeedback )
{

}

void QgsProcessingFeedback::setProgressText( const QString &text )
{
  mHtmlLog.append( text.toHtmlEscaped().replace( '\n', "<br>"_L1 ) + u"<br/>"_s );
  mTextLog.append( text + '\n' );
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
    mHtmlLog.append( u"<span style=\"color:red\">%1</span><br/>"_s.arg( tr( "Message log truncated" ) ) );
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

  log( u"<span style=\"color:red\">%1</span><br/>"_s.arg( error.toHtmlEscaped() ).replace( '\n', "<br>"_L1 ),
       error + '\n' );
}

void QgsProcessingFeedback::pushWarning( const QString &warning )
{
  if ( mLogFeedback )
    QgsMessageLog::logMessage( warning, tr( "Processing" ), Qgis::MessageLevel::Warning );

  log( u"<span style=\"color:#b85a20;\">%1</span><br/>"_s.arg( warning.toHtmlEscaped() ).replace( '\n', "<br>"_L1 ) + u"<br/>"_s,
       warning + '\n' );
}

void QgsProcessingFeedback::pushInfo( const QString &info )
{
  if ( mLogFeedback )
    QgsMessageLog::logMessage( info, tr( "Processing" ), Qgis::MessageLevel::Info );

  mHtmlLog.append( info.toHtmlEscaped().replace( '\n', "<br>"_L1 ) + u"<br/>"_s );
  mTextLog.append( info + '\n' );
}

void QgsProcessingFeedback::pushFormattedMessage( const QString &html, const QString &text )
{
  if ( mLogFeedback )
    QgsMessageLog::logMessage( text, tr( "Processing" ), Qgis::MessageLevel::Info );

  mHtmlLog.append( html + u"<br/>"_s );
  mTextLog.append( text + '\n' );
}

void QgsProcessingFeedback::pushCommandInfo( const QString &info )
{
  if ( mLogFeedback )
    QgsMessageLog::logMessage( info, tr( "Processing" ), Qgis::MessageLevel::Info );

  log( u"<code>%1</code><br/>"_s.arg( info.toHtmlEscaped().replace( '\n', "<br>"_L1 ) ),
       info + '\n' );
}

void QgsProcessingFeedback::pushDebugInfo( const QString &info )
{
  if ( mLogFeedback )
    QgsMessageLog::logMessage( info, tr( "Processing" ), Qgis::MessageLevel::Info );

  log( u"<span style=\"color:#777\">%1</span><br/>"_s.arg( info.toHtmlEscaped().replace( '\n', "<br>"_L1 ) ),
       info + '\n' );
}

void QgsProcessingFeedback::pushConsoleInfo( const QString &info )
{
  if ( mLogFeedback )
    QgsMessageLog::logMessage( info, tr( "Processing" ), Qgis::MessageLevel::Info );

  log( u"<code style=\"color:#777\">%1</code><br/>"_s.arg( info.toHtmlEscaped().replace( '\n', "<br>"_L1 ) ),
       info + '\n' );
}

void QgsProcessingFeedback::pushVersionInfo( const QgsProcessingProvider *provider )
{
  pushDebugInfo( tr( "QGIS version: %1" ).arg( Qgis::version() ) );
  if ( QString( Qgis::devVersion() ) != "exported"_L1 )
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

#ifdef WITH_SFCGAL
  pushDebugInfo( tr( "SFCGAL version: %1" ).arg( sfcgal_version() ) );
#else
  pushDebugInfo( tr( "No support for SFCGAL" ) );
#endif

#ifdef WITH_GEOGRAPHICLIB
  pushDebugInfo( tr( "GeographicLib version: %1.%2.%3" ).arg( GEOGRAPHICLIB_VERSION_MAJOR ).arg( GEOGRAPHICLIB_VERSION_MINOR ).arg( GEOGRAPHICLIB_VERSION_PATCH ) );
#else
  pushDebugInfo( tr( "No support for GeographicLib" ) );
#endif

  if ( provider && !provider->versionInfo().isEmpty() )
  {
    pushDebugInfo( tr( "%1 version: %2" ).arg( provider->name(), provider->versionInfo() ) );
  }
}

void QgsProcessingFeedback::pushFormattedResults( const QgsProcessingAlgorithm *algorithm, QgsProcessingContext &context, const QVariantMap &results )
{
  if ( results.empty() )
    return;

  pushInfo( tr( "Results:" ) );

  const QList< const QgsProcessingOutputDefinition * > outputs = algorithm->outputDefinitions();
  for ( const QgsProcessingOutputDefinition *output : outputs )
  {
    const QString outputName = output->name();
    if ( outputName == "CHILD_RESULTS"_L1 || outputName == "CHILD_INPUTS"_L1 )
      continue;

    if ( !results.contains( outputName ) )
      continue;

    bool ok = false;
    const QString textValue = output->valueAsString( results.value( output->name() ), context, ok );
    const QString formattedValue = output->valueAsFormattedString( results.value( output->name() ), context, ok );
    if ( ok )
    {
      pushFormattedMessage( u"<code>&nbsp;&nbsp;%1: %2</code>"_s.arg( output->name(), formattedValue ),
                            u"  %1: %2"_s.arg( output->name(), textValue ) );
    }
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
  if ( mFeedback )
  {
    connect( mFeedback, &QgsFeedback::canceled, this, &QgsFeedback::cancel, Qt::DirectConnection );
    connect( this, &QgsFeedback::progressChanged, this, &QgsProcessingMultiStepFeedback::updateOverallProgress );
  }
}

void QgsProcessingMultiStepFeedback::setCurrentStep( int step )
{
  mCurrentStep = step;

  if ( mFeedback )
    mFeedback->setProgress( 100.0 * static_cast< double >( mCurrentStep ) / mChildSteps );
}

void QgsProcessingMultiStepFeedback::setProgressText( const QString &text )
{
  if ( mFeedback )
    mFeedback->setProgressText( text );
}

void QgsProcessingMultiStepFeedback::reportError( const QString &error, bool fatalError )
{
  if ( mFeedback )
    mFeedback->reportError( error, fatalError );
}

void QgsProcessingMultiStepFeedback::pushWarning( const QString &warning )
{
  if ( mFeedback )
    mFeedback->pushWarning( warning );
}

void QgsProcessingMultiStepFeedback::pushInfo( const QString &info )
{
  if ( mFeedback )
    mFeedback->pushInfo( info );
}

void QgsProcessingMultiStepFeedback::pushCommandInfo( const QString &info )
{
  if ( mFeedback )
    mFeedback->pushCommandInfo( info );
}

void QgsProcessingMultiStepFeedback::pushDebugInfo( const QString &info )
{
  if ( mFeedback )
    mFeedback->pushDebugInfo( info );
}

void QgsProcessingMultiStepFeedback::pushConsoleInfo( const QString &info )
{
  if ( mFeedback )
    mFeedback->pushConsoleInfo( info );
}

void QgsProcessingMultiStepFeedback::pushFormattedMessage( const QString &html, const QString &text )
{
  if ( mFeedback )
    mFeedback->pushFormattedMessage( html, text );
}

QString QgsProcessingMultiStepFeedback::htmlLog() const
{
  if ( mFeedback )
    return mFeedback->htmlLog();
  return QString();
}

QString QgsProcessingMultiStepFeedback::textLog() const
{
  if ( mFeedback )
    return mFeedback->textLog();
  return QString();
}

void QgsProcessingMultiStepFeedback::updateOverallProgress( double progress )
{
  const double baseProgress = 100.0 * static_cast< double >( mCurrentStep ) / mChildSteps;
  const double currentAlgorithmProgress = progress / mChildSteps;
  if ( mFeedback )
    mFeedback->setProgress( baseProgress + currentAlgorithmProgress );
}
