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

#include <QButtonGroup>
#include <QToolButton>

#include "qgsgeometryvalidationdock.h"
#include "qgsgeometryvalidationmodel.h"
#include "qgsgeometryvalidationservice.h"
#include "qgsmapcanvas.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgsgeometrycheck.h"
#include "qgsgeometrycheckerror.h"
#include "qgsanalysis.h"
#include "qgsgeometrycheckregistry.h"
#include "qgsgeometryoptions.h"
#include "qgsgeometrycheckfactory.h"


QgsGeometryValidationDock::QgsGeometryValidationDock( const QString &title, QgsMapCanvas *mapCanvas, QWidget *parent, Qt::WindowFlags flags )
  : QgsDockWidget( title, parent, flags )
  , mMapCanvas( mapCanvas )
{
  setupUi( this );

  QFont font = mProblemDescriptionLabel->font();
  font.setBold( true );
  mProblemDescriptionLabel->setFont( font );
  mErrorListView->setAlternatingRowColors( true );

  connect( mNextButton, &QToolButton::clicked, this, &QgsGeometryValidationDock::gotoNextError );
  connect( mPreviousButton, &QToolButton::clicked, this, &QgsGeometryValidationDock::gotoPreviousError );
  connect( mZoomToProblemButton, &QToolButton::clicked, this, &QgsGeometryValidationDock::zoomToProblem );
  connect( mZoomToFeatureButton, &QToolButton::clicked, this, &QgsGeometryValidationDock::zoomToFeature );
  connect( mMapCanvas, &QgsMapCanvas::currentLayerChanged, this, &QgsGeometryValidationDock::onCurrentLayerChanged );
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

  mErrorRubberband->setColor( QColor( "#ffee58ff" ) );
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
  connect( mGeometryValidationModel, &QgsGeometryValidationModel::rowsRemoved, this, &QgsGeometryValidationDock::updateCurrentError );
  connect( mGeometryValidationModel, &QgsGeometryValidationModel::rowsInserted, this, &QgsGeometryValidationDock::raise );
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
  if ( !featureExtent.isEmpty() )
  {
    QgsRectangle mapExtent = mLayerTransform.transform( featureExtent );
    mMapCanvas->zoomToFeatureExtent( mapExtent );
  }
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

void QgsGeometryValidationDock::updateCurrentError()
{
  mFeatureRubberband->hide();
  mErrorRubberband->hide();
  mErrorLocationRubberband->hide();

  onCurrentErrorChanged( currentIndex(), QModelIndex() );
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

  if ( !current.isValid() )
    return;

  mProblemDescriptionLabel->setText( current.data( QgsGeometryValidationModel::DetailsRole ).toString() );

  QgsGeometryCheckError *error = current.data( QgsGeometryValidationModel::GeometryCheckErrorRole ).value<QgsGeometryCheckError *>();
  if ( error )
  {
    delete mResolutionWidget->layout();
    const QStringList resolutionMethods = error->check()->resolutionMethods();
    QGridLayout *layout = new QGridLayout( mResolutionWidget );
    int resolutionIndex = 0;
    for ( const QString &resolutionMethod : resolutionMethods )
    {
      QToolButton *resolveBtn = new QToolButton( mResolutionWidget );
      resolveBtn->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmCheckGeometry.svg" ) ) );
      layout->addWidget( resolveBtn, resolutionIndex, 0 );
      QLabel *resolveLabel = new QLabel( resolutionMethod, mResolutionWidget );
      resolveLabel->setWordWrap( true );
      layout->addWidget( resolveLabel, resolutionIndex, 1 );
      connect( resolveBtn, &QToolButton::clicked, this, [resolutionIndex, error, this]()
      {
        mGeometryValidationService->fixError( error, resolutionIndex );
      } );
      resolutionIndex++;
    }
    mResolutionWidget->setLayout( layout );
  }

  bool hasFeature = !FID_IS_NULL( current.data( QgsGeometryValidationModel::ErrorFeatureIdRole ) );
  mZoomToFeatureButton->setEnabled( hasFeature );

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

void QgsGeometryValidationDock::onCurrentLayerChanged( QgsMapLayer *layer )
{
  // activate icon
  bool enabled = false;
  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer );
  if ( vl && vl->isSpatial() )
  {
    const QList<QgsGeometryCheckFactory *> topologyCheckFactories = QgsAnalysis::instance()->geometryCheckRegistry()->geometryCheckFactories( vl, QgsGeometryCheck::LayerCheck, QgsGeometryCheck::Flag::AvailableInValidation );
    const QStringList activeChecks = vl->geometryOptions()->geometryChecks();
    for ( const QgsGeometryCheckFactory *factory : topologyCheckFactories )
    {
      if ( activeChecks.contains( factory->id() ) )
      {
        enabled = true;
        break;
      }
    }
  }
  mTopologyChecksPendingButton->setEnabled( enabled );
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

    QPropertyAnimation *featureAnimation = new QPropertyAnimation( mFeatureRubberband, "fillColor" );
    featureAnimation->setEasingCurve( QEasingCurve::OutQuad );
    connect( featureAnimation, &QPropertyAnimation::finished, featureAnimation, &QPropertyAnimation::deleteLater );
    connect( featureAnimation, &QPropertyAnimation::valueChanged, this, [this]
    {
      mFeatureRubberband->update();
    } );

    featureAnimation->setDuration( 2000 );
    featureAnimation->setStartValue( QColor( 100, 255, 100, 255 ) );
    featureAnimation->setEndValue( QColor( 100, 255, 100, 0 ) );

    featureAnimation->start();

    mErrorRubberband->setToGeometry( errorGeometry );

    QPropertyAnimation *errorAnimation = new QPropertyAnimation( mErrorRubberband, "fillColor" );
    errorAnimation->setEasingCurve( QEasingCurve::OutQuad );
    connect( errorAnimation, &QPropertyAnimation::finished, featureAnimation, &QPropertyAnimation::deleteLater );
    connect( errorAnimation, &QPropertyAnimation::valueChanged, this, [this]
    {
      mErrorRubberband->update();
    } );

    errorAnimation->setStartValue( QColor( "#ffee58ff" ) );
    errorAnimation->setEndValue( QColor( "#ffee5800" ) );

    errorAnimation->setDuration( 2000 );
    errorAnimation->start();

    mErrorLocationRubberband->setToGeometry( QgsGeometry( qgis::make_unique<QgsPoint>( locationGeometry ) ) );
  }
}
