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
#include <QPropertyAnimation>

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
#include "qgisapp.h"
#include "qgsapplication.h"


QgsGeometryValidationDock::QgsGeometryValidationDock( const QString &title, QgsMapCanvas *mapCanvas, QgisApp *parent, Qt::WindowFlags flags )
  : QgsDockWidget( title, parent, flags )
  , mMapCanvas( mapCanvas )
  , mQgisApp( parent )
{
  setupUi( this );

  mProblemDescriptionLabel->setStyleSheet( QStringLiteral( "font: bold" ) );
  mErrorListView->setAlternatingRowColors( true );
  mErrorListView->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( mErrorListView, &QWidget::customContextMenuRequested, this, &QgsGeometryValidationDock::showErrorContextMenu );

  connect( mNextButton, &QToolButton::clicked, this, &QgsGeometryValidationDock::gotoNextError );
  connect( mPreviousButton, &QToolButton::clicked, this, &QgsGeometryValidationDock::gotoPreviousError );
  connect( mZoomToProblemButton, &QToolButton::clicked, this, &QgsGeometryValidationDock::zoomToProblem );
  connect( mZoomToFeatureButton, &QToolButton::clicked, this, &QgsGeometryValidationDock::zoomToFeature );
  connect( mMapCanvas, &QgsMapCanvas::currentLayerChanged, this, &QgsGeometryValidationDock::onCurrentLayerChanged );
  connect( mMapCanvas, &QgsMapCanvas::currentLayerChanged, this, &QgsGeometryValidationDock::updateLayerTransform );
  connect( mMapCanvas, &QgsMapCanvas::destinationCrsChanged, this, &QgsGeometryValidationDock::updateLayerTransform );
  connect( mMapCanvas, &QgsMapCanvas::transformContextChanged, this, &QgsGeometryValidationDock::updateLayerTransform );

  mFeatureRubberband = new QgsRubberBand( mMapCanvas );
  mErrorRubberband = new QgsRubberBand( mMapCanvas );
  mErrorLocationRubberband = new QgsRubberBand( mMapCanvas );
  mGeometryErrorContextMenu = new QMenu( this );

  const double scaleFactor = mMapCanvas->fontMetrics().xHeight() * .4;

  mFeatureRubberband->setWidth( scaleFactor );
  mFeatureRubberband->setStrokeColor( QColor( 100, 255, 100, 100 ) );

  mErrorRubberband->setColor( QColor( 255, 238, 88, 255 ) );
  mErrorRubberband->setWidth( scaleFactor );

  mErrorLocationRubberband->setIcon( QgsRubberBand::ICON_X );
  mErrorLocationRubberband->setWidth( scaleFactor );
  mErrorLocationRubberband->setIconSize( scaleFactor * 5 );
  mErrorLocationRubberband->setColor( QColor( 50, 255, 50, 255 ) );

  mProblemDetailWidget->setVisible( false );

  // Some problem resolutions are unstable, show all of them only if the user opted in
  const bool showUnreliableResolutionMethods = QgsSettings().value( QStringLiteral( "geometry_validation/enable_problem_resolution" ) ).toString().compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0;
  mResolutionWidget->setVisible( showUnreliableResolutionMethods );
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
  connect( mErrorListView->selectionModel(), &QItemSelectionModel::currentChanged, this, [this]() { updateMapCanvasExtent(); } );
  connect( mGeometryValidationModel, &QgsGeometryValidationModel::dataChanged, this, &QgsGeometryValidationDock::onDataChanged );
  connect( mGeometryValidationModel, &QgsGeometryValidationModel::rowsRemoved, this, &QgsGeometryValidationDock::updateCurrentError );
  connect( mGeometryValidationModel, &QgsGeometryValidationModel::rowsInserted, this, &QgsGeometryValidationDock::onRowsInserted );

  // We cannot connect to the regular aboutToRemoveRows signal, because we need this to happen
  // before the currentIndex is changed.
  connect( mGeometryValidationModel, &QgsGeometryValidationModel::aboutToRemoveSingleGeometryCheck, this, [this]() { mPreventZoomToError = true; } );
  connect( mGeometryValidationModel, &QgsGeometryValidationModel::rowsRemoved, this, [this]() { mPreventZoomToError = false; } );
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

  const QgsRectangle problemExtent = currentIndex().data( QgsGeometryValidationModel::ProblemExtentRole ).value<QgsRectangle>();
  QgsRectangle mapExtent = mLayerTransform.transform( problemExtent );
  mMapCanvas->zoomToFeatureExtent( mapExtent );
}

void QgsGeometryValidationDock::zoomToFeature()
{
  mLastZoomToAction = ZoomToFeature;
  if ( !currentIndex().isValid() )
    return;

  const QgsRectangle featureExtent = currentIndex().data( QgsGeometryValidationModel::FeatureExtentRole ).value<QgsRectangle>();
  if ( !featureExtent.isEmpty() )
  {
    QgsRectangle mapExtent = mLayerTransform.transform( featureExtent );
    mMapCanvas->zoomToFeatureExtent( mapExtent );
  }
}

void QgsGeometryValidationDock::updateLayerTransform()
{
  if ( !mMapCanvas->currentLayer() )
    return;

  mLayerTransform = QgsCoordinateTransform( mMapCanvas->currentLayer()->crs(), mMapCanvas->mapSettings().destinationCrs(), mMapCanvas->mapSettings().transformContext() );
}

void QgsGeometryValidationDock::onDataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles )
{
  Q_UNUSED( bottomRight )
  Q_UNUSED( roles )

  if ( currentIndex() == topLeft )
    updateCurrentError();
}

void QgsGeometryValidationDock::onRowsInserted()
{
  if ( !isVisible() )
  {
    mQgisApp->addDockWidget( Qt::RightDockWidgetArea, this );
  }
  setUserVisible( true );
}

void QgsGeometryValidationDock::showErrorContextMenu( const QPoint &pos )
{
  const bool showUnreliableResolutionMethods = QgsSettings().value( QStringLiteral( "geometry_validation/enable_problem_resolution" ) ).toString().compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0;

  const QModelIndex index = mErrorListView->indexAt( pos );
  QgsGeometryCheckError *error = index.data( QgsGeometryValidationModel::GeometryCheckErrorRole ).value<QgsGeometryCheckError *>();
  if ( error )
  {
    const QList<QgsGeometryCheckResolutionMethod> resolutionMethods = error->check()->availableResolutionMethods();

    mGeometryErrorContextMenu->clear();
    for ( const QgsGeometryCheckResolutionMethod &resolutionMethod : resolutionMethods )
    {
      if ( resolutionMethod.isStable() || showUnreliableResolutionMethods )
      {
        QAction *action = new QAction( resolutionMethod.name(), mGeometryErrorContextMenu );
        action->setToolTip( resolutionMethod.description() );
        const int fixId = resolutionMethod.id();
        connect( action, &QAction::triggered, this, [ fixId, error, this ]()
        {
          mGeometryValidationService->fixError( error, fixId );
        } );
        mGeometryErrorContextMenu->addAction( action );
      }
    }
    mGeometryErrorContextMenu->popup( mErrorListView->viewport()->mapToGlobal( pos ) );
  }
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
  if ( mGeometryValidationModel->rowCount() == 0 )
    mErrorListView->selectionModel()->clearCurrentIndex();

  mFeatureRubberband->hide();
  mFeatureRubberband->update();
  mErrorRubberband->hide();
  mErrorRubberband->update();
  mErrorLocationRubberband->hide();
  mErrorLocationRubberband->update();

  onCurrentErrorChanged( currentIndex(), QModelIndex() );
}

QModelIndex QgsGeometryValidationDock::currentIndex() const
{
  if ( !mGeometryValidationModel->rowCount() )
    return QModelIndex();
  else
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
    while ( QWidget *w = mResolutionWidget->findChild<QWidget *>() )
      delete w;

    delete mResolutionWidget->layout();

    if ( error->status() != QgsGeometryCheckError::StatusFixed )
    {
      const QList<QgsGeometryCheckResolutionMethod> resolutionMethods = error->check()->availableResolutionMethods();
      QGridLayout *layout = new QGridLayout( mResolutionWidget );
      int resolutionIndex = 0;
      for ( const QgsGeometryCheckResolutionMethod &resolutionMethod : resolutionMethods )
      {
        QToolButton *resolveBtn = new QToolButton( mResolutionWidget );
        resolveBtn->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmCheckGeometry.svg" ) ) );
        resolveBtn->setToolTip( resolutionMethod.description() );
        layout->addWidget( resolveBtn, resolutionIndex, 0 );
        QLabel *resolveLabel = new QLabel( resolutionMethod.name(), mResolutionWidget );
        resolveLabel->setToolTip( resolutionMethod.description() );
        resolveLabel->setWordWrap( true );
        layout->addWidget( resolveLabel, resolutionIndex, 1 );
        const int fixId = resolutionMethod.id();
        connect( resolveBtn, &QToolButton::clicked, this, [fixId, error, this]()
        {
          mGeometryValidationService->fixError( error, fixId );
        } );
        resolutionIndex++;
      }

      mResolutionWidget->setLayout( layout );

      showHighlight( current );
    }
  }

  const bool hasContextRectangle = !current.data( QgsGeometryValidationModel::FeatureExtentRole ).isNull();
  mZoomToFeatureButton->setEnabled( hasContextRectangle );
}

void QgsGeometryValidationDock::updateMapCanvasExtent()
{
  if ( !mPreventZoomToError )
  {
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
}

void QgsGeometryValidationDock::onCurrentLayerChanged( QgsMapLayer *layer )
{
  if ( layer == mCurrentLayer )
    return;

  if ( mCurrentLayer )
  {
    disconnect( mCurrentLayer, &QgsVectorLayer::editingStarted, this, &QgsGeometryValidationDock::onLayerEditingStatusChanged );
    disconnect( mCurrentLayer, &QgsVectorLayer::editingStopped, this, &QgsGeometryValidationDock::onLayerEditingStatusChanged );
    disconnect( mCurrentLayer, &QgsVectorLayer::destroyed, this, &QgsGeometryValidationDock::onLayerDestroyed );
  }

  mCurrentLayer = qobject_cast<QgsVectorLayer *>( layer );

  if ( mCurrentLayer )
  {
    connect( mCurrentLayer, &QgsVectorLayer::editingStarted, this, &QgsGeometryValidationDock::onLayerEditingStatusChanged );
    connect( mCurrentLayer, &QgsVectorLayer::editingStopped, this, &QgsGeometryValidationDock::onLayerEditingStatusChanged );
    connect( mCurrentLayer, &QgsVectorLayer::destroyed, this, &QgsGeometryValidationDock::onLayerDestroyed );
  }

  onLayerEditingStatusChanged();
}

void QgsGeometryValidationDock::onLayerEditingStatusChanged()
{
  if ( mCurrentLayer && mCurrentLayer->isSpatial() && mCurrentLayer->isEditable() )
  {
    const QList<QgsGeometryCheckFactory *> topologyCheckFactories = QgsAnalysis::geometryCheckRegistry()->geometryCheckFactories( mCurrentLayer, QgsGeometryCheck::LayerCheck, QgsGeometryCheck::Flag::AvailableInValidation );
    const QStringList activeChecks = mCurrentLayer->geometryOptions()->geometryChecks();
    for ( const QgsGeometryCheckFactory *factory : topologyCheckFactories )
    {
      if ( activeChecks.contains( factory->id() ) )
        break;
    }
  }
}

void QgsGeometryValidationDock::onLayerDestroyed( QObject *layer )
{
  if ( layer == mCurrentLayer )
    mCurrentLayer = nullptr;
}

void QgsGeometryValidationDock::showHighlight( const QModelIndex &current )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mMapCanvas->currentLayer() );
  if ( vlayer )
  {
    const QgsGeometry featureGeometry = current.data( QgsGeometryValidationModel::FeatureGeometryRole ).value<QgsGeometry>();
    const QgsGeometry errorGeometry = current.data( QgsGeometryValidationModel::ErrorGeometryRole ).value<QgsGeometry>();
    const QgsPointXY locationGeometry = current.data( QgsGeometryValidationModel::ErrorLocationGeometryRole ).value<QgsPointXY>();

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
    connect( errorAnimation, &QPropertyAnimation::finished, errorAnimation, &QPropertyAnimation::deleteLater );
    connect( errorAnimation, &QPropertyAnimation::valueChanged, this, [this]
    {
      mErrorRubberband->update();
    } );

    errorAnimation->setStartValue( QColor( 255, 238, 88, 255 ) );
    errorAnimation->setEndValue( QColor( 255, 238, 88, 0 ) );

    errorAnimation->setDuration( 2000 );
    errorAnimation->start();

    mErrorLocationRubberband->setToGeometry( QgsGeometry( std::make_unique<QgsPoint>( locationGeometry ) ) );
  }
}
