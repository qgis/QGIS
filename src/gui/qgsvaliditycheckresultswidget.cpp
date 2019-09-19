/***************************************************************************
  qgsvaliditycheckresultswidget.cpp
 ----------------------------------
 begin                : November 2018
 copyright            : (C) 2018 by Nyall Dawson
 email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvaliditycheckresultswidget.h"
#include "qgsvaliditycheckregistry.h"
#include "qgsapplication.h"
#include "qgsfeedback.h"
#include "qgsproxyprogresstask.h"
#include <QProgressDialog>
#include <QDialogButtonBox>
#include <QPushButton>

//
// QgsValidityCheckResultsModel
//

QgsValidityCheckResultsModel::QgsValidityCheckResultsModel( const QList<QgsValidityCheckResult> &results, QObject *parent )
  : QAbstractItemModel( parent )
  , mResults( results )
{

}

QModelIndex QgsValidityCheckResultsModel::index( int row, int column, const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return createIndex( row, column );
}

QModelIndex QgsValidityCheckResultsModel::parent( const QModelIndex &child ) const
{
  Q_UNUSED( child )
  return QModelIndex();
}

int QgsValidityCheckResultsModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return mResults.count();
}

int QgsValidityCheckResultsModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 1;
}

QVariant QgsValidityCheckResultsModel::data( const QModelIndex &index, int role ) const
{
  if ( index.row() >= mResults.count() || index.row() < 0 )
    return QVariant();

  const QgsValidityCheckResult &res = mResults.at( index.row() );
  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
      return res.title;

    case QgsValidityCheckResultsModel::DescriptionRole:
      return res.detailedDescription;

    case Qt::DecorationRole:
      switch ( res.type )
      {
        case QgsValidityCheckResult::Critical:
          return QgsApplication::getThemeIcon( QStringLiteral( "/mIconCritical.svg" ) );

        case QgsValidityCheckResult::Warning:
          return QgsApplication::getThemeIcon( QStringLiteral( "/mIconWarning.svg" ) );
      }
      break;

    default:
      return QVariant();
  }
  return QVariant();
}

//
// QgsValidityCheckResultsWidget
//

QgsValidityCheckResultsWidget::QgsValidityCheckResultsWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

}

void QgsValidityCheckResultsWidget::setResults( const QList<QgsValidityCheckResult> &results )
{
  if ( mResultsModel )
    mResultsModel->deleteLater();

  mResultsModel = new QgsValidityCheckResultsModel( results, this );
  mResultsListView->setModel( mResultsModel );

  connect( mResultsListView->selectionModel(), &QItemSelectionModel::currentChanged, this, &QgsValidityCheckResultsWidget::selectionChanged );

  if ( mResultsModel->rowCount() > 0 )
  {
    // auto select first result in list
    const QModelIndex firstResult( mResultsModel->index( 0, 0, QModelIndex() ) );
    mResultsListView->selectionModel()->select( firstResult, QItemSelectionModel::ClearAndSelect );
    selectionChanged( firstResult, QModelIndex() );
  }

  mDescriptionLabel->hide();
}

void QgsValidityCheckResultsWidget::setDescription( const QString &description )
{
  mDescriptionLabel->setText( description );
  mDescriptionLabel->setVisible( !description.isEmpty() );
}

bool QgsValidityCheckResultsWidget::runChecks( int type, const QgsValidityCheckContext *context, const QString &title, const QString &description, QWidget *parent )
{
  std::unique_ptr< QgsFeedback > feedback = qgis::make_unique< QgsFeedback >();
  std::unique_ptr< QProgressDialog > progressDialog = qgis::make_unique< QProgressDialog >( tr( "Running Checks…" ), tr( "Abort" ), 0, 100, parent );
  progressDialog->setWindowTitle( title );

  QgsProxyProgressTask *proxyTask = new QgsProxyProgressTask( tr( "Running Checks" ) );

  connect( feedback.get(), &QgsFeedback::progressChanged, progressDialog.get(), [ & ]( double progress )
  {
    progressDialog->setValue( static_cast< int >( progress ) );
    progressDialog->setLabelText( feedback->property( "progress" ).toString() ) ;

    proxyTask->setProxyProgress( progress );

#ifdef Q_OS_LINUX
    // For some reason on Windows hasPendingEvents() always return true,
    // but one iteration is actually enough on Windows to get good interactivity
    // whereas on Linux we must allow for far more iterations.
    // For safety limit the number of iterations
    int nIters = 0;
    while ( QCoreApplication::hasPendingEvents() && ++nIters < 100 )
#endif
    {
      QCoreApplication::processEvents();
    }

  } );
  connect( progressDialog.get(), &QProgressDialog::canceled, progressDialog.get(), [ & ]
  {
    feedback->cancel();
  } );

  QgsApplication::taskManager()->addTask( proxyTask );

  const QList<QgsValidityCheckResult> results = QgsApplication::validityCheckRegistry()->runChecks( type, context, feedback.get() );

  proxyTask->finalize( true );

  if ( feedback->isCanceled() )
    return false;

  if ( results.empty() )
    return true;

  QgsValidityCheckResultsWidget *w = new QgsValidityCheckResultsWidget( nullptr );
  w->setResults( results );
  w->setDescription( description );

  bool hasCritical = false;
  for ( const QgsValidityCheckResult &res : results )
  {
    if ( res.type == QgsValidityCheckResult::Critical )
    {
      hasCritical = true;
      break;
    }
  }

  QVBoxLayout *l = new QVBoxLayout();
  l->addWidget( w );

  QDialog dlg( parent );
  dlg.setWindowTitle( title );

  QDialogButtonBox *buttons = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg );
  connect( buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept );
  connect( buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject );
  if ( hasCritical )
  {
    buttons->button( QDialogButtonBox::Ok )->setEnabled( false );
    buttons->button( QDialogButtonBox::Ok )->setToolTip( tr( "Critical errors prevent this task from proceeding. Please address these issues and then retry." ) );
  }

  l->addWidget( buttons );

  dlg.setLayout( l );

  return dlg.exec();
}

void QgsValidityCheckResultsWidget::selectionChanged( const QModelIndex &current, const QModelIndex & )
{
  const QString desc = mResultsModel->data( current, QgsValidityCheckResultsModel::DescriptionRole ).toString();
  mDetailedDescriptionTextBrowser->setHtml( desc );
}
