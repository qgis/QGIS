/***************************************************************************
    qgsappcoordinateoperationhandlers.cpp
    -------------------------
    begin                : May 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsappcoordinateoperationhandlers.h"
#include "qgscoordinatetransform.h"
#include "qgisapp.h"
#include "qgsmessagebar.h"
#include "qgsmessagebaritem.h"
#include "qgsmessageoutput.h"
#include "qgsproject.h"
#include "qgsinstallgridshiftdialog.h"

//
// QgsAppMissingRequiredGridHandler
//
QgsAppMissingGridHandler::QgsAppMissingGridHandler( QObject *parent )
  : QObject( parent )
{
  QgsCoordinateTransform::setCustomMissingRequiredGridHandler( [ = ]( const QgsCoordinateReferenceSystem & sourceCrs,
      const QgsCoordinateReferenceSystem & destinationCrs,
      const QgsDatumTransform::GridDetails & grid )
  {
    emit missingRequiredGrid( sourceCrs, destinationCrs, grid );
  } );

  QgsCoordinateTransform::setCustomMissingPreferredGridHandler( [ = ]( const QgsCoordinateReferenceSystem & sourceCrs,
      const QgsCoordinateReferenceSystem & destinationCrs,
      const QgsDatumTransform::TransformDetails & preferredOperation,
      const QgsDatumTransform::TransformDetails & availableOperation )
  {
    emit missingPreferredGrid( sourceCrs, destinationCrs, preferredOperation, availableOperation );
  } );

  QgsCoordinateTransform::setCustomCoordinateOperationCreationErrorHandler( [ = ]( const QgsCoordinateReferenceSystem & sourceCrs,
      const QgsCoordinateReferenceSystem & destinationCrs,
      const QString & error )
  {
    emit coordinateOperationCreationError( sourceCrs, destinationCrs, error );
  } );

  QgsCoordinateTransform::setCustomMissingGridUsedByContextHandler( [ = ]( const QgsCoordinateReferenceSystem & sourceCrs,
      const QgsCoordinateReferenceSystem & destinationCrs,
      const QgsDatumTransform::TransformDetails & desired )
  {
    emit missingGridUsedByContextHandler( sourceCrs, destinationCrs, desired );
  } );

  QgsCoordinateTransform::setFallbackOperationOccurredHandler( [ = ]( const QgsCoordinateReferenceSystem & sourceCrs,
      const QgsCoordinateReferenceSystem & destinationCrs,
      const QString & desired )
  {
    emit fallbackOperationOccurred( sourceCrs, destinationCrs, desired );
  } );

  QgsCoordinateTransform::setDynamicCrsToDynamicCrsWarningHandler( [ = ]( const QgsCoordinateReferenceSystem & sourceCrs,
      const QgsCoordinateReferenceSystem & destinationCrs )
  {
    emit dynamicToDynamicWarning( sourceCrs, destinationCrs );
  } );

  connect( this, &QgsAppMissingGridHandler::missingRequiredGrid, this, &QgsAppMissingGridHandler::onMissingRequiredGrid, Qt::QueuedConnection );
  connect( this, &QgsAppMissingGridHandler::missingPreferredGrid, this, &QgsAppMissingGridHandler::onMissingPreferredGrid, Qt::QueuedConnection );
  connect( this, &QgsAppMissingGridHandler::coordinateOperationCreationError, this, &QgsAppMissingGridHandler::onCoordinateOperationCreationError, Qt::QueuedConnection );
  connect( this, &QgsAppMissingGridHandler::missingGridUsedByContextHandler, this, &QgsAppMissingGridHandler::onMissingGridUsedByContextHandler, Qt::QueuedConnection );
  connect( this, &QgsAppMissingGridHandler::fallbackOperationOccurred, this, &QgsAppMissingGridHandler::onFallbackOperationOccurred, Qt::QueuedConnection );
  connect( this, &QgsAppMissingGridHandler::dynamicToDynamicWarning, this, &QgsAppMissingGridHandler::onDynamicToDynamicWarning, Qt::QueuedConnection );

  connect( QgsProject::instance(), &QgsProject::cleared, this, [ = ]
  {
    mAlreadyWarnedPairsForProject.clear();
    mAlreadyWarnedBallparkPairsForProject.clear();
  } );

}

void QgsAppMissingGridHandler::onMissingRequiredGrid( const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs, const QgsDatumTransform::GridDetails &grid )
{
  if ( !shouldWarnAboutPair( sourceCrs, destinationCrs ) )
    return;

  const QString shortMessage = tr( "No transform available between %1 and %2" ).arg( sourceCrs.userFriendlyIdentifier( QgsCoordinateReferenceSystem::ShortString ),
                               destinationCrs.userFriendlyIdentifier( QgsCoordinateReferenceSystem::ShortString ) );

  QString downloadMessage;
  const QString gridName = grid.shortName;
  if ( !grid.url.isEmpty() )
  {
    if ( !grid.packageName.isEmpty() )
    {
      downloadMessage = tr( "This grid is part of the “<i>%1</i>” package, available for download from <a href=\"%2\">%2</a>." ).arg( grid.packageName, grid.url );
    }
    else
    {
      downloadMessage = tr( "This grid is available for download from <a href=\"%1\">%1</a>." ).arg( grid.url );
    }
  }

  const QString longMessage = tr( "<p>No transform is available between <i>%1</i> and <i>%2</i>.</p>"
                                  "<p>This transformation requires the grid file “%3”, which is not available for use on the system.</p>" ).arg( sourceCrs.userFriendlyIdentifier(),
                                      destinationCrs.userFriendlyIdentifier(),
                                      grid.shortName );

  QgsMessageBar *bar = QgisApp::instance()->messageBar();
  QgsMessageBarItem *widget = QgsMessageBar::createMessage( QString(), shortMessage );
  QPushButton *detailsButton = new QPushButton( tr( "Details" ) );
  connect( detailsButton, &QPushButton::clicked, this, [longMessage, downloadMessage, bar, widget, gridName]
  {
    QgsInstallGridShiftFileDialog *dlg = new QgsInstallGridShiftFileDialog( gridName, QgisApp::instance() );
    dlg->setAttribute( Qt::WA_DeleteOnClose );
    dlg->setWindowTitle( tr( "No Transformations Available" ) );
    dlg->setDescription( longMessage );
    dlg->setDownloadMessage( downloadMessage );
    if ( dlg->exec() )
    {
      bar->popWidget( widget );
    }
  } );

  widget->layout()->addWidget( detailsButton );
  bar->pushWidget( widget, Qgis::MessageLevel::Critical, 0 );
}

void QgsAppMissingGridHandler::onMissingPreferredGrid( const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs, const QgsDatumTransform::TransformDetails &preferredOperation, const QgsDatumTransform::TransformDetails &availableOperation )
{
  if ( !shouldWarnAboutPair( sourceCrs, destinationCrs ) )
    return;

  const QString shortMessage = tr( "Cannot use preferred transform between %1 and %2" ).arg( sourceCrs.userFriendlyIdentifier( QgsCoordinateReferenceSystem::ShortString ),
                               destinationCrs.userFriendlyIdentifier( QgsCoordinateReferenceSystem::ShortString ) );

  QString gridMessage;
  QString downloadMessage;
  QString gridName;
  for ( const QgsDatumTransform::GridDetails &grid : preferredOperation.grids )
  {
    if ( !grid.isAvailable )
    {
      const QString m = tr( "This transformation requires the grid file “%1”, which is not available for use on the system." ).arg( grid.shortName );
      gridName = grid.shortName;
      if ( !grid.url.isEmpty() )
      {
        if ( !grid.packageName.isEmpty() )
        {
          downloadMessage = tr( "This grid is part of the <i>%1</i> package, available for download from <a href=\"%2\">%2</a>." ).arg( grid.packageName, grid.url );
        }
        else
        {
          downloadMessage = tr( "This grid is available for download from <a href=\"%1\">%1</a>." ).arg( grid.url );
        }
      }
      gridMessage += QStringLiteral( "<li>%1</li>" ).arg( m );
    }
  }
  if ( !gridMessage.isEmpty() )
  {
    gridMessage = "<ul>" + gridMessage + "</ul>";
  }

  QString accuracyMessage;
  if ( availableOperation.accuracy >= 0 && preferredOperation.accuracy >= 0 )
    accuracyMessage = tr( "<p>Current transform “<i>%1</i>” has an accuracy of %2 meters, while the preferred transformation “<i>%3</i>” has accuracy %4 meters.</p>" ).arg( availableOperation.name )
                      .arg( availableOperation.accuracy ).arg( preferredOperation.name ).arg( preferredOperation.accuracy );
  else if ( preferredOperation.accuracy >= 0 )
    accuracyMessage = tr( "<p>Current transform “<i>%1</i>” has an unknown accuracy, while the preferred transformation “<i>%2</i>” has accuracy %3 meters.</p>" ).arg( availableOperation.name )
                      .arg( preferredOperation.name ).arg( preferredOperation.accuracy );

  const QString longMessage = tr( "<p>The preferred transform between <i>%1</i> and <i>%2</i> is not available for use on the system.</p>" ).arg( sourceCrs.userFriendlyIdentifier(),
                              destinationCrs.userFriendlyIdentifier() )
                              + gridMessage + accuracyMessage;

  QgsMessageBar *bar = QgisApp::instance()->messageBar();
  QgsMessageBarItem *widget = QgsMessageBar::createMessage( QString(), shortMessage );
  QPushButton *detailsButton = new QPushButton( tr( "Details" ) );
  connect( detailsButton, &QPushButton::clicked, this, [longMessage, downloadMessage, gridName, widget, bar]
  {
    QgsInstallGridShiftFileDialog *dlg = new QgsInstallGridShiftFileDialog( gridName, QgisApp::instance() );
    dlg->setAttribute( Qt::WA_DeleteOnClose );
    dlg->setWindowTitle( tr( "Preferred Transformation Not Available" ) );
    dlg->setDescription( longMessage );
    dlg->setDownloadMessage( downloadMessage );
    if ( dlg->exec() )
    {
      bar->popWidget( widget );
    }
  } );

  widget->layout()->addWidget( detailsButton );
  bar->pushWidget( widget, Qgis::MessageLevel::Warning, 0 );
}

void QgsAppMissingGridHandler::onCoordinateOperationCreationError( const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs, const QString &error )
{
  if ( !shouldWarnAboutPairForCurrentProject( sourceCrs, destinationCrs ) )
    return;

  const QString shortMessage = tr( "No transform available between %1 and %2" ).arg( sourceCrs.userFriendlyIdentifier( QgsCoordinateReferenceSystem::ShortString ), destinationCrs.userFriendlyIdentifier( QgsCoordinateReferenceSystem::ShortString ) );
  const QString longMessage = tr( "<p>No transform is available between <i>%1</i> and <i>%2</i>.</p><p style=\"color: red\">%3</p>" ).arg( sourceCrs.userFriendlyIdentifier(), destinationCrs.userFriendlyIdentifier(), error );

  QgsMessageBar *bar = QgisApp::instance()->messageBar();
  QgsMessageBarItem *widget = QgsMessageBar::createMessage( QString(), shortMessage );
  QPushButton *detailsButton = new QPushButton( tr( "Details" ) );
  connect( detailsButton, &QPushButton::clicked, this, [longMessage]
  {
    // dlg has deleted on close
    QgsMessageOutput * dlg( QgsMessageOutput::createMessageOutput() );
    dlg->setTitle( tr( "No Transformations Available" ) );
    dlg->setMessage( longMessage, QgsMessageOutput::MessageHtml );
    dlg->showMessage();
  } );

  widget->layout()->addWidget( detailsButton );
  bar->pushWidget( widget, Qgis::MessageLevel::Critical, 0 );
}

void QgsAppMissingGridHandler::onMissingGridUsedByContextHandler( const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs, const QgsDatumTransform::TransformDetails &desired )
{
  if ( !shouldWarnAboutPairForCurrentProject( sourceCrs, destinationCrs ) )
    return;

  const QString shortMessage = tr( "Cannot use project transform between %1 and %2" ).arg( sourceCrs.userFriendlyIdentifier( QgsCoordinateReferenceSystem::ShortString ),
                               destinationCrs.userFriendlyIdentifier( QgsCoordinateReferenceSystem::ShortString ) );

  QString gridMessage;
  QString downloadMessage;
  QString gridName;
  for ( const QgsDatumTransform::GridDetails &grid : desired.grids )
  {
    if ( !grid.isAvailable )
    {
      gridName = grid.shortName;
      const QString m = tr( "This transformation requires the grid file “%1”, which is not available for use on the system." ).arg( grid.shortName );
      if ( !grid.url.isEmpty() )
      {
        if ( !grid.packageName.isEmpty() )
        {
          downloadMessage = tr( "This grid is part of the <i>%1</i> package, available for download from <a href=\"%2\">%2</a>." ).arg( grid.packageName, grid.url );
        }
        else
        {
          downloadMessage = tr( "This grid is available for download from <a href=\"%1\">%1</a>." ).arg( grid.url );
        }
      }
      gridMessage += QStringLiteral( "<li>%1</li>" ).arg( m );
    }
  }
  if ( !gridMessage.isEmpty() )
  {
    gridMessage = "<ul>" + gridMessage + "</ul>";
  }

  const QString longMessage = tr( "<p>This project specifies a preset transform between <i>%1</i> and <i>%2</i>, which is not available for use on the system.</p>" ).arg( sourceCrs.userFriendlyIdentifier(),
                              destinationCrs.userFriendlyIdentifier() )
                              + gridMessage
                              + tr( "<p>The operation specified for use in the project is:</p><p><code>%1</code></p>" ).arg( desired.proj ) ;

  QgsMessageBar *bar = QgisApp::instance()->messageBar();
  QgsMessageBarItem *widget = QgsMessageBar::createMessage( QString(), shortMessage );
  QPushButton *detailsButton = new QPushButton( tr( "Details" ) );
  connect( detailsButton, &QPushButton::clicked, this, [longMessage, gridName, downloadMessage, bar, widget]
  {
    QgsInstallGridShiftFileDialog *dlg = new QgsInstallGridShiftFileDialog( gridName, QgisApp::instance() );
    dlg->setAttribute( Qt::WA_DeleteOnClose );
    dlg->setWindowTitle( tr( "Project Transformation Not Available" ) );
    dlg->setDescription( longMessage );
    dlg->setDownloadMessage( downloadMessage );
    if ( dlg->exec() )
    {
      bar->popWidget( widget );
    }
  } );

  widget->layout()->addWidget( detailsButton );
  bar->pushWidget( widget, Qgis::MessageLevel::Critical, 0 );
}

void QgsAppMissingGridHandler::onFallbackOperationOccurred( const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs, const QString &desired )
{
  if ( !shouldWarnAboutBallparkPairForCurrentProject( sourceCrs, destinationCrs ) )
    return;

  const QString shortMessage = tr( "Used a ballpark transform from %1 to %2" ).arg( sourceCrs.userFriendlyIdentifier( QgsCoordinateReferenceSystem::ShortString ), destinationCrs.userFriendlyIdentifier( QgsCoordinateReferenceSystem::ShortString ) );
  const QString longMessage = tr( "<p>An alternative, ballpark-only transform was used when transforming coordinates between <i>%1</i> and <i>%2</i>. The results may not match those obtained by using the preferred operation:</p><code>%3</code><p style=\"font-weight: bold\">Possibly an incorrect choice of operation was made for transformations between these reference systems. Check the Project Properties and ensure that the selected transform operations are applicable over the whole extent of the current project." ).arg( sourceCrs.userFriendlyIdentifier(), destinationCrs.userFriendlyIdentifier(), desired );

  QgsMessageBar *bar = QgisApp::instance()->messageBar();
  QgsMessageBarItem *widget = QgsMessageBar::createMessage( QString(), shortMessage );
  QPushButton *detailsButton = new QPushButton( tr( "Details" ) );
  connect( detailsButton, &QPushButton::clicked, this, [longMessage]
  {
    // dlg has deleted on close
    QgsMessageOutput * dlg( QgsMessageOutput::createMessageOutput() );
    dlg->setTitle( tr( "Ballpark Transform Occurred" ) );
    dlg->setMessage( longMessage, QgsMessageOutput::MessageHtml );
    dlg->showMessage();
  } );

  widget->layout()->addWidget( detailsButton );
  bar->pushWidget( widget, Qgis::MessageLevel::Warning, 0 );
}

void QgsAppMissingGridHandler::onDynamicToDynamicWarning( const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs )
{
  if ( !shouldWarnAboutDynamicCrsForCurrentProject( sourceCrs, destinationCrs ) )
    return;

  const QString shortMessage = tr( "Cannot transform between dynamic CRS at difference coordinate epochs" );
  const QString longMessage = tr( "<p>Transformation between %1 and %2 is not currently supported.</p><p><b>The results will be unpredictable and should not be used for high accuracy work.</b>" ).arg( sourceCrs.userFriendlyIdentifier(), destinationCrs.userFriendlyIdentifier() );

  QgsMessageBar *bar = QgisApp::instance()->messageBar();
  QgsMessageBarItem *widget = QgsMessageBar::createMessage( QString(), shortMessage );
  QPushButton *detailsButton = new QPushButton( tr( "Details" ) );
  connect( detailsButton, &QPushButton::clicked, this, [longMessage]
  {
    // dlg has deleted on close
    QgsMessageOutput * dlg( QgsMessageOutput::createMessageOutput() );
    dlg->setTitle( tr( "Unsupported Transformation" ) );
    dlg->setMessage( longMessage, QgsMessageOutput::MessageHtml );
    dlg->showMessage();
  } );

  widget->layout()->addWidget( detailsButton );
  bar->pushWidget( widget, Qgis::MessageLevel::Critical, 0 );
}

bool QgsAppMissingGridHandler::shouldWarnAboutPair( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &dest )
{
  if ( mAlreadyWarnedPairs.contains( qMakePair( source, dest ) ) || mAlreadyWarnedPairs.contains( qMakePair( dest, source ) ) )
  {
    return false;
  }

  mAlreadyWarnedPairs.append( qMakePair( source, dest ) );
  return true;
}

bool QgsAppMissingGridHandler::shouldWarnAboutPairForCurrentProject( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &dest )
{
  if ( mAlreadyWarnedPairsForProject.contains( qMakePair( source, dest ) ) || mAlreadyWarnedPairsForProject.contains( qMakePair( dest, source ) ) )
  {
    return false;
  }

  mAlreadyWarnedPairsForProject.append( qMakePair( source, dest ) );
  return true;
}

bool QgsAppMissingGridHandler::shouldWarnAboutBallparkPairForCurrentProject( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &dest )
{
  if ( mAlreadyWarnedBallparkPairsForProject.contains( qMakePair( source, dest ) ) || mAlreadyWarnedBallparkPairsForProject.contains( qMakePair( dest, source ) ) )
  {
    return false;
  }

  mAlreadyWarnedBallparkPairsForProject.append( qMakePair( source, dest ) );
  return true;
}

bool QgsAppMissingGridHandler::shouldWarnAboutDynamicCrsForCurrentProject( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &dest )
{
  if ( mAlreadyWarnedDynamicCrsForProject.contains( qMakePair( source, dest ) ) || mAlreadyWarnedDynamicCrsForProject.contains( qMakePair( dest, source ) ) )
  {
    return false;
  }

  mAlreadyWarnedDynamicCrsForProject.append( qMakePair( source, dest ) );
  return true;
}
