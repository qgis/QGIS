/***************************************************************************
                      qgsgeometryvalidationdock.cpp
                     --------------------------------------
Date                 : 7.9.2018
Copyright            : (C) 2018 by Matthias Kuhn
email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometryvalidationdock.h"
#include "qgsgeometryvalidationmodel.h"
#include "qgsgeometryvalidationservice.h"
#include "qgsmapcanvas.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgsgeometrycheck.h"
#include "qgsgeometrycheckerror.h"

#include <QButtonGroup>

QgsGeometryValidationDock::QgsGeometryValidationDock( const QString &title, QgsMapCanvas *mapCanvas, QWidget *parent, Qt::WindowFlags flags )
  : QgsDockWidget( title, parent, flags )
  , mMapCanvas( mapCanvas )
{
  setupUi( this );

  connect( mNextButton, &QPushButton::clicked, this, &QgsGeometryValidationDock::gotoNextError );
  connect( mPreviousButton, &QPushButton::clicked, this, &QgsGeometryValidationDock::gotoPreviousError );
  connect( mZoomToProblemButton, &QPushButton::clicked, this, &QgsGeometryValidationDock::zoomToProblem );
  connect( mZoomToFeatureButton, &QPushButton::clicked, this, &QgsGeometryValidationDock::zoomToFeature );
  connect( mMapCanvas, &QgsMapCanvas::currentLayerChanged, this, &QgsGeometryValidationDock::updateLayerTransform );
  connect( mMapCanvas, &QgsMapCanvas::destinationCrsChanged, this, &QgsGeometryValidationDock::updateLayerTransform );
  connect( mMapCanvas, &QgsMapCanvas::transformContextChanged, this, &QgsGeometryValidationDock::updateLayerTransform );

  mFeatureRubberband = new QgsRubberBand( mMapCanvas );
  mErrorRubberband = new QgsRubberBand( mMapCanvas );
  mErrorLocationRubberband = new QgsRubberBand( mMapCanvas );

  double scaleFactor = mMapCanvas->fontMetrics().xHeight() * .2;

  mFeatureRubberband->setColor( QColor( 250, 180, 180, 100 ) );
  mFeatureRubberband->setWidth( scaleFactor );
  mErrorRubberband->setColor( QColor( 180, 250, 180, 100 ) );
  mErrorRubberband->setWidth( scaleFactor );
  mErrorLocationRubberband->setIcon( QgsRubberBand::ICON_X );
  mErrorLocationRubberband->setWidth( scaleFactor * 3 );
  mErrorLocationRubberband->setColor( QColor( 180, 180, 250, 100 ) );
}


QgsGeometryValidationModel *QgsGeometryValidationDock::geometryValidationModel() const
{
  return mGeometryValidationModel;
}

void QgsGeometryValidationDock::setGeometryValidationModel( QgsGeometryValidationModel *geometryValidationModel )
{
  mGeometryValidationModel = geometryValidationModel;
  mErrorListView->setModel( mGeometryValidationModel );

  connect( mErrorListView->selectionModel(), &QItemSelectionModel::currentChanged, this, &QgsGeometryValidationDock::onCurrentErrorChanged );
}

void QgsGeometryValidationDock::gotoNextError()
{
  QItemSelectionModel *selectionModel = mErrorListView->selectionModel();
  selectionModel->setCurrentIndex( mGeometryValidationModel->index( selectionModel->currentIndex().row() + 1, 0, QModelIndex() ), QItemSelectionModel::ClearAndSelect );
}

void QgsGeometryValidationDock::gotoPreviousError()
{
  QItemSelectionModel *selectionModel = mErrorListView->selectionModel();
  selectionModel->setCurrentIndex( mGeometryValidationModel->index( selectionModel->currentIndex().row() - 1, 0, QModelIndex() ), QItemSelectionModel::ClearAndSelect );
}

void QgsGeometryValidationDock::zoomToProblem()
{
  mLastZoomToAction = ZoomToProblem;
  if ( !currentIndex().isValid() )
    return;

  QgsRectangle problemExtent = currentIndex().data( QgsGeometryValidationModel::ProblemExtentRole ).value<QgsRectangle>();
  QgsRectangle mapExtent = mLayerTransform.transform( problemExtent );
  mMapCanvas->zoomToFeatureExtent( mapExtent );
}

void QgsGeometryValidationDock::zoomToFeature()
{
  mLastZoomToAction = ZoomToFeature;
  if ( !currentIndex().isValid() )
    return;

  QgsRectangle featureExtent = currentIndex().data( QgsGeometryValidationModel::FeatureExtentRole ).value<QgsRectangle>();
  QgsRectangle mapExtent = mLayerTransform.transform( featureExtent );
  mMapCanvas->zoomToFeatureExtent( mapExtent );
}

void QgsGeometryValidationDock::updateLayerTransform()
{
  if ( !mMapCanvas->currentLayer() )
    return;

  mLayerTransform = QgsCoordinateTransform( mMapCanvas->currentLayer()->crs(), mMapCanvas->mapSettings().destinationCrs(), mMapCanvas->mapSettings().transformContext() );
}

QgsGeometryValidationService *QgsGeometryValidationDock::geometryValidationService() const
{
  return mGeometryValidationService;
}

void QgsGeometryValidationDock::setGeometryValidationService( QgsGeometryValidationService *geometryValidationService )
{
  mGeometryValidationService = geometryValidationService;
}

QModelIndex QgsGeometryValidationDock::currentIndex() const
{
  return mErrorListView->selectionModel()->currentIndex();
}

void QgsGeometryValidationDock::onCurrentErrorChanged( const QModelIndex &current, const QModelIndex &previous )
{
  Q_UNUSED( previous )
  mNextButton->setEnabled( current.isValid() && current.row() < mGeometryValidationModel->rowCount() - 1 );
  mPreviousButton->setEnabled( current.isValid() && current.row() > 0 );

  mProblemDetailWidget->setVisible( current.isValid() );
  mProblemDescriptionLabel->setText( current.data().toString() );
  {
    QgsGeometryCheckError *error = current.data( QgsGeometryValidationModel::GeometryCheckErrorRole ).value<QgsGeometryCheckError *>();
    while ( QPushButton *btn =  mResolutionWidget->findChild<QPushButton *>() )
      delete btn;
    const QStringList resolutionMethods = error->check()->resolutionMethods();
    int resolutionIndex = 0;
    for ( const QString &resolutionMethod : resolutionMethods )
    {
      QPushButton *resolveBtn = new QPushButton( resolutionMethod );
      connect( resolveBtn, &QPushButton::clicked, this, [resolutionIndex, error, this]()
      {
        mGeometryValidationService->fixError( error, resolutionIndex );
      } );
      mResolutionWidget->layout()->addWidget( resolveBtn );
      resolutionIndex++;
    }
  }

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mMapCanvas->currentLayer() );
  if ( vlayer )
  {
    QgsGeometry featureGeometry = current.data( QgsGeometryValidationModel::FeatureGeometryRole ).value<QgsGeometry>();
    QgsGeometry errorGeometry = current.data( QgsGeometryValidationModel::ErrorGeometryRole ).value<QgsGeometry>();
    QgsPointXY locationGeometry = current.data( QgsGeometryValidationModel::ErrorLocationGeometryRole ).value<QgsPointXY>();
    qDebug() << "feature geom : " << featureGeometry.asWkt();
    qDebug() << "error   geom : " << errorGeometry.asWkt();
    qDebug() << "locationgeom : " << QgsGeometry( new QgsPoint( locationGeometry ) ).asWkt();

    mFeatureRubberband->setToGeometry( featureGeometry );
    mErrorRubberband->setToGeometry( errorGeometry );
    mErrorLocationRubberband->setToGeometry( QgsGeometry( new QgsPoint( locationGeometry ) ) );
  }

  switch ( mLastZoomToAction )
  {
    case  ZoomToProblem:
      zoomToProblem();
      break;

    case ZoomToFeature:
      zoomToFeature();
      break;
  }
}
