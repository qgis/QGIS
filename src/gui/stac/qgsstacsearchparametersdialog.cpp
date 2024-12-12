/***************************************************************************
    qgsstacsearchparametersdialog.cpp
    ---------------------
    begin                : November 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstacsearchparametersdialog.h"
#include "moc_qgsstacsearchparametersdialog.cpp"
#include "qgsgui.h"
#include "qgsmapcanvas.h"
#include "qgsprojecttimesettings.h"
#include "qgsstaccollection.h"

#include <QPushButton>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMenu>
#include <QTextDocument>

///@cond PRIVATE

QgsStacSearchParametersDialog::QgsStacSearchParametersDialog( QgsMapCanvas *canvas, QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  mExtent = QgsRectangle( -180., -90., 180., 90. );
  mExtentCrs = QgsCoordinateReferenceSystem::fromEpsgId( 4326 );
  mSpatialExtent->setMapCanvas( canvas );
  mSpatialExtent->setCurrentExtent( mExtent, mExtentCrs );
  mCollectionsModel = new QStandardItemModel( mCollectionsListView );
  mCollectionsProxyModel = new QSortFilterProxyModel( mCollectionsModel );
  mCollectionsProxyModel->setSourceModel( mCollectionsModel );
  mCollectionsProxyModel->setFilterCaseSensitivity( Qt::CaseInsensitive );
  mCollectionsProxyModel->setSortCaseSensitivity( Qt::CaseInsensitive );
  mCollectionsProxyModel->sort( 0 );
  mCollectionsListView->setModel( mCollectionsProxyModel );
  mFromDateTimeEdit->setAllowNull( true );
  mFromDateTimeEdit->setDateTime( QDateTime() );
  mFromDateTimeEdit->setNullRepresentation( tr( "Not set" ) );
  mToDateTimeEdit->setAllowNull( true );
  mToDateTimeEdit->setDateTime( QDateTime() );
  mToDateTimeEdit->setNullRepresentation( tr( "Not set" ) );
  mCollectionsFilterLineEdit->setShowSearchIcon( true );

  mMenu = new QMenu( this );
  mTemporalExtentFromProjectAction = new QAction( tr( "Use Project's Temporal Extent" ), this );
  connect( mTemporalExtentFromProjectAction, &QAction::triggered, this, &QgsStacSearchParametersDialog::readTemporalExtentsFromProject );

  mMenu->addAction( mTemporalExtentFromProjectAction );
  mTemporalToolButton->setMenu( mMenu );
  mTemporalToolButton->setPopupMode( QToolButton::InstantPopup );

  connect( mCollectionsFilterLineEdit, &QgsFilterLineEdit::textChanged, mCollectionsProxyModel, &QSortFilterProxyModel::setFilterFixedString );
  connect( mSelectAllCollectionsButton, &QPushButton::clicked, this, &QgsStacSearchParametersDialog::selectAllCollections );
  connect( mDeselectAllCollectionsButton, &QPushButton::clicked, this, &QgsStacSearchParametersDialog::deselectAllCollections );
  connect( mTemporalToolButton, &QToolButton::clicked, this, &QgsStacSearchParametersDialog::readTemporalExtentsFromProject );
}

QgsStacSearchParametersDialog::~QgsStacSearchParametersDialog()
{
  qDeleteAll( mCollections );
}

void QgsStacSearchParametersDialog::accept()
{
  mSpatialFilterEnabled = mSpatialGroupBox->isChecked();
  mTemporalFilterEnabled = mTemporalGroupBox->isChecked();
  mCollectionsFilterEnabled = mCollectionsGroupBox->isChecked();
  mExtent = mSpatialExtent->outputExtent();
  mExtentCrs = mSpatialExtent->outputCrs();
  mTemporalFrom = mFromDateTimeEdit->dateTime();
  mTemporalTo = mToDateTimeEdit->dateTime();

  mSelectedCollections.clear();
  for ( int i = 0; i < mCollectionsModel->rowCount(); ++i )
  {
    const QModelIndex index = mCollectionsModel->index( i, 0 );
    if ( mCollectionsModel->data( index, Qt::CheckStateRole ) == Qt::Checked )
      mSelectedCollections.insert( mCollectionsModel->data( index, Qt::UserRole ).toString() );
  }

  // if none was checked, check them all!
  if ( mSelectedCollections.isEmpty() )
  {
    for ( int i = 0; i < mCollectionsModel->rowCount(); ++i )
    {
      const QModelIndex index = mCollectionsModel->index( i, 0 );
      mCollectionsModel->setData( index, Qt::Checked, Qt::CheckStateRole );
      mSelectedCollections.insert( mCollectionsModel->data( index, Qt::UserRole ).toString() );
    }
  }

  QDialog::accept();
}

void QgsStacSearchParametersDialog::reject()
{
  mSpatialGroupBox->setChecked( mSpatialFilterEnabled );
  mTemporalGroupBox->setChecked( mTemporalFilterEnabled );
  mCollectionsGroupBox->setChecked( mCollectionsFilterEnabled );
  mSpatialExtent->setCurrentExtent( mExtent, mExtentCrs );
  mFromDateTimeEdit->setDateTime( mTemporalFrom );
  mToDateTimeEdit->setDateTime( mTemporalTo );

  for ( int i = 0; i < mCollectionsModel->rowCount(); ++i )
  {
    const QModelIndex index = mCollectionsModel->index( i, 0 );
    const bool isChecked = mSelectedCollections.contains( mCollectionsModel->data( index, Qt::UserRole ).toString() );
    mCollectionsModel->setData( index, isChecked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
  }
  QDialog::reject();
}

void QgsStacSearchParametersDialog::setMapCanvas( QgsMapCanvas *canvas )
{
  mSpatialExtent->setMapCanvas( canvas );
}

bool QgsStacSearchParametersDialog::hasTemporalFilter() const
{
  return mTemporalFilterEnabled && !( mFromDateTimeEdit->dateTime().isNull() || mToDateTimeEdit->dateTime().isNull() );
}

bool QgsStacSearchParametersDialog::hasSpatialFilter() const
{
  return mSpatialFilterEnabled;
}

bool QgsStacSearchParametersDialog::hasCollectionsFilter() const
{
  return mCollectionsFilterEnabled;
}

QgsGeometry QgsStacSearchParametersDialog::spatialExtent() const
{
  QgsGeometry geom = QgsGeometry::fromRect( mExtent );

  QgsCoordinateTransform ct( mExtentCrs, QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), QgsProject::instance() );
  try
  {
    geom.transform( ct );
  }
  catch ( QgsCsException &e )
  {
    QgsDebugError( QStringLiteral( "Could not transform extent to WGS84: %1" ).arg( e.what() ) );
  }

  return geom.intersection( QgsGeometry::fromRect( QgsRectangle( -180., -90., 180., 90. ) ) );
}

QgsDateTimeRange QgsStacSearchParametersDialog::temporalRange() const
{
  if ( mFromDateTimeEdit->dateTime() <= mToDateTimeEdit->dateTime() || mFromDateTimeEdit->dateTime().isNull() || mToDateTimeEdit->dateTime().isNull() )
    return QgsDateTimeRange( mFromDateTimeEdit->dateTime(), mToDateTimeEdit->dateTime() );
  else
    return QgsDateTimeRange( mToDateTimeEdit->dateTime(), mFromDateTimeEdit->dateTime() );
}

QSet<QString> QgsStacSearchParametersDialog::selectedCollections() const
{
  return mSelectedCollections;
}

void QgsStacSearchParametersDialog::setCollections( const QVector<QgsStacCollection *> &collections )
{
  qDeleteAll( mCollections );
  mSelectedCollections.clear();
  mCollectionsModel->clear();
  mCollectionsGroupBox->setChecked( false );
  mCollections = collections;

  QTextDocument descr;
  for ( QgsStacCollection *c : std::as_const( mCollections ) )
  {
    descr.setMarkdown( c->description() );

    QStandardItem *i = new QStandardItem( c->title() );
    i->setData( c->id(), Qt::UserRole );
    i->setData( descr.toHtml(), Qt::ToolTipRole );
    i->setCheckable( true );
    i->setCheckState( Qt::Checked );
    mCollectionsModel->appendRow( i );
    mSelectedCollections.insert( c->id() );
  }
}

QVector<QgsStacCollection *> QgsStacSearchParametersDialog::collections() const
{
  return mCollections;
}

QString QgsStacSearchParametersDialog::activeFiltersPreview()
{
  QString str = tr( "%1/%2 Collections" )
                  .arg( mCollectionsFilterEnabled ? mSelectedCollections.size() : mCollectionsModel->rowCount() )
                  .arg( mCollectionsModel->rowCount() );

  if ( mSpatialFilterEnabled )
  {
    str += QStringLiteral( ", " ) + tr( "Spatial Extent" );
  }

  if ( mTemporalFilterEnabled && !( mTemporalFrom.isNull() && mTemporalTo.isNull() ) )
  {
    str += QStringLiteral( ", " ) + tr( "Temporal Range" );
  }
  return str;
}

void QgsStacSearchParametersDialog::selectAllCollections()
{
  for ( int i = 0; i < mCollectionsProxyModel->rowCount(); ++i )
  {
    const QModelIndex index = mCollectionsProxyModel->index( i, 0 );
    mCollectionsProxyModel->setData( index, Qt::Checked, Qt::CheckStateRole );
  }
}

void QgsStacSearchParametersDialog::deselectAllCollections()
{
  for ( int i = 0; i < mCollectionsProxyModel->rowCount(); ++i )
  {
    const QModelIndex index = mCollectionsProxyModel->index( i, 0 );
    mCollectionsProxyModel->setData( index, Qt::Unchecked, Qt::CheckStateRole );
  }
}

void QgsStacSearchParametersDialog::readTemporalExtentsFromProject()
{
  const QgsDateTimeRange projectRange = QgsProject::instance()->timeSettings()->temporalRange();
  mFromDateTimeEdit->setDateTime( projectRange.begin() );
  mToDateTimeEdit->setDateTime( projectRange.end() );
}

///@endcond
