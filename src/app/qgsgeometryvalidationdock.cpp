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
  connect( mTopologyChecksPendingButton, &QToolButton::clicked, this, &QgsGeometryValidationDock::triggerTopologyChecks );

  mFeatureRubberband = new QgsRubberBand( mMapCanvas );
  mErrorRubberband = new QgsRubberBand( mMapCanvas );
  mErrorLocationRubberband = new QgsRubberBand( mMapCanvas );

  double scaleFactor = mMapCanvas->fontMetrics().xHeight() * .4;

  mFeatureRubberband->setWidth( scaleFactor );
  mFeatureRubberband->setStrokeColor( QColor( 100, 255, 100, 100 ) );

  mErrorRubberband->setColor( QColor( 180, 250, 180, 100 ) );
  mErrorRubberband->setWidth( scaleFactor );

  mErrorLocationRubberband->setIcon( QgsRubberBand::ICON_X );
  mErrorLocationRubberband->setWidth( scaleFactor );
  mErrorLocationRubberband->setIconSize( scaleFactor * 5 );
  mErrorLocationRubberband->setColor( QColor( 50, 255, 50, 255 ) );

  mProblemDetailWidget->setVisible( false );
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

void QgsGeometryValidationDock::triggerTopologyChecks()
{
  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( mMapCanvas->currentLayer() );
  if ( layer )
    mGeometryValidationService->triggerTopologyChecks( layer );
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

  mProblemDetailWidget->setVisible( current.isValid() );

  mNextButton->setEnabled( current.isValid() && current.row() < mGeometryValidationModel->rowCount() - 1 );
  mPreviousButton->setEnabled( current.isValid() && current.row() > 0 );

  mProblemDetailWidget->setVisible( current.isValid() );
  mProblemDescriptionLabel->setText( current.data( QgsGeometryValidationModel::DetailsRole ).toString() );

  QgsGeometryCheckError *error = current.data( QgsGeometryValidationModel::GeometryCheckErrorRole ).value<QgsGeometryCheckError *>();
  if ( error )
  {
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

  showHighlight( current );

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

void QgsGeometryValidationDock::showHighlight( const QModelIndex &current )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mMapCanvas->currentLayer() );
  if ( vlayer )
  {
    QgsGeometry featureGeometry = current.data( QgsGeometryValidationModel::FeatureGeometryRole ).value<QgsGeometry>();
    QgsGeometry errorGeometry = current.data( QgsGeometryValidationModel::ErrorGeometryRole ).value<QgsGeometry>();
    QgsPointXY locationGeometry = current.data( QgsGeometryValidationModel::ErrorLocationGeometryRole ).value<QgsPointXY>();

    mFeatureRubberband->setToGeometry( featureGeometry );


    QPropertyAnimation *animation = new QPropertyAnimation( mFeatureRubberband, "fillColor" );
    animation->setEasingCurve( QEasingCurve::OutQuad );
    connect( animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater );
    connect( animation, &QPropertyAnimation::valueChanged, this, [this]
    {
      mFeatureRubberband->update();
    } );

    animation->setDuration( 2000 );
    animation->setStartValue( QColor( 100, 255, 100, 255 ) );
    animation->setEndValue( QColor( 100, 255, 100, 0 ) );

    animation->start();

    // mErrorRubberband->setToGeometry( errorGeometry );
    mErrorLocationRubberband->setToGeometry( QgsGeometry( new QgsPoint( locationGeometry ) ) );
  }
}
