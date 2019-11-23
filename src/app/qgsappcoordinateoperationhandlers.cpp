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

  connect( this, &QgsAppMissingGridHandler::missingRequiredGrid, this, &QgsAppMissingGridHandler::onMissingRequiredGrid, Qt::QueuedConnection );
  connect( this, &QgsAppMissingGridHandler::missingPreferredGrid, this, &QgsAppMissingGridHandler::onMissingPreferredGrid, Qt::QueuedConnection );
  connect( this, &QgsAppMissingGridHandler::coordinateOperationCreationError, this, &QgsAppMissingGridHandler::onCoordinateOperationCreationError, Qt::QueuedConnection );
  connect( this, &QgsAppMissingGridHandler::missingGridUsedByContextHandler, this, &QgsAppMissingGridHandler::onMissingGridUsedByContextHandler, Qt::QueuedConnection );

  connect( QgsProject::instance(), &QgsProject::cleared, this, [ = ] { mAlreadyWarnedPairsForProject.clear(); } );

}

QString displayIdentifierForCrs( const QgsCoordinateReferenceSystem &crs, bool shortString = false )
{
  if ( !crs.authid().isEmpty() )
  {
    if ( !shortString && !crs.description().isEmpty() )
      return QStringLiteral( "%1 [%2]" ).arg( crs.authid(), crs.description() );
    return crs.authid();
  }
  else if ( !crs.description().isEmpty() )
    return crs.description();
  else if ( !crs.toProj4().isEmpty() )
    return crs.toProj4();
  else
    return crs.toWkt();
}

void QgsAppMissingGridHandler::onMissingRequiredGrid( const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs, const QgsDatumTransform::GridDetails &grid )
{
  if ( !shouldWarnAboutPair( sourceCrs, destinationCrs ) )
    return;

  const QString shortMessage = tr( "No transform available between %1 and %2" ).arg( displayIdentifierForCrs( sourceCrs, true ),
                               displayIdentifierForCrs( destinationCrs, true ) );

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
                                  "<p>This transformation requires the grid file “%3”, which is not available for use on the system.</p>" ).arg( displayIdentifierForCrs( sourceCrs ),
                                      displayIdentifierForCrs( destinationCrs ),
                                      grid.shortName );

  QgsMessageBar *bar = QgisApp::instance()->messageBar();
  QgsMessageBarItem *widget = bar->createMessage( QString(), shortMessage );
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
  bar->pushWidget( widget, Qgis::Critical, 0 );
}

void QgsAppMissingGridHandler::onMissingPreferredGrid( const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs, const QgsDatumTransform::TransformDetails &preferredOperation, const QgsDatumTransform::TransformDetails &availableOperation )
{
  if ( !shouldWarnAboutPair( sourceCrs, destinationCrs ) )
    return;

  const QString shortMessage = tr( "Cannot use preferred transform between %1 and %2" ).arg( displayIdentifierForCrs( sourceCrs, true ),
                               displayIdentifierForCrs( destinationCrs, true ) );

  QString gridMessage;
  QString downloadMessage;
  QString gridName;
  for ( const QgsDatumTransform::GridDetails &grid : preferredOperation.grids )
  {
    if ( !grid.isAvailable )
    {
      QString m = tr( "This transformation requires the grid file “%1”, which is not available for use on the system." ).arg( grid.shortName );
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

  const QString longMessage = tr( "<p>The preferred transform between <i>%1</i> and <i>%2</i> is not available for use on the system.</p>" ).arg( displayIdentifierForCrs( sourceCrs ),
                              displayIdentifierForCrs( destinationCrs ) )
                              + gridMessage + accuracyMessage;

  QgsMessageBar *bar = QgisApp::instance()->messageBar();
  QgsMessageBarItem *widget = bar->createMessage( QString(), shortMessage );
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
  bar->pushWidget( widget, Qgis::Warning, 0 );
}

void QgsAppMissingGridHandler::onCoordinateOperationCreationError( const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs, const QString &error )
{
  if ( !shouldWarnAboutPairForCurrentProject( sourceCrs, destinationCrs ) )
    return;

  const QString shortMessage = tr( "No transform available between %1 and %2" ).arg( displayIdentifierForCrs( sourceCrs, true ), displayIdentifierForCrs( destinationCrs, true ) );
  const QString longMessage = tr( "<p>No transform is available between <i>%1</i> and <i>%2</i>.</p><p style=\"color: red\">%3</p>" ).arg( displayIdentifierForCrs( sourceCrs ), displayIdentifierForCrs( destinationCrs ), error );

  QgsMessageBar *bar = QgisApp::instance()->messageBar();
  QgsMessageBarItem *widget = bar->createMessage( QString(), shortMessage );
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
  bar->pushWidget( widget, Qgis::Critical, 0 );
}

void QgsAppMissingGridHandler::onMissingGridUsedByContextHandler( const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs, const QgsDatumTransform::TransformDetails &desired )
{
  if ( !shouldWarnAboutPairForCurrentProject( sourceCrs, destinationCrs ) )
    return;

  const QString shortMessage = tr( "Cannot use project transform between %1 and %2" ).arg( displayIdentifierForCrs( sourceCrs, true ),
                               displayIdentifierForCrs( destinationCrs, true ) );

  QString gridMessage;
  QString downloadMessage;
  QString gridName;
  for ( const QgsDatumTransform::GridDetails &grid : desired.grids )
  {
    if ( !grid.isAvailable )
    {
      gridName = grid.shortName;
      QString m = tr( "This transformation requires the grid file “%1”, which is not available for use on the system." ).arg( grid.shortName );
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

  const QString longMessage = tr( "<p>This project specifies a preset transform between <i>%1</i> and <i>%2</i>, which is not available for use on the system.</p>" ).arg( displayIdentifierForCrs( sourceCrs ),
                              displayIdentifierForCrs( destinationCrs ) )
                              + gridMessage
                              + tr( "<p>The operation specified for use in the project is:</p><p><code>%1</code></p>" ).arg( desired.proj ) ;

  QgsMessageBar *bar = QgisApp::instance()->messageBar();
  QgsMessageBarItem *widget = bar->createMessage( QString(), shortMessage );
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
  bar->pushWidget( widget, Qgis::Critical, 0 );
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
