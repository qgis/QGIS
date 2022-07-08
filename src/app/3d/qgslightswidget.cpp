/***************************************************************************
  qgslightswidget.cpp
  --------------------------------------
  Date                 : November 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslightswidget.h"

#include "qgs3dmapsettings.h"
#include "qgsapplication.h"
#include "qgssettings.h"

#include <QMessageBox>
#include <QMenu>

QgsLightsWidget::QgsLightsWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  spinPositionX->setClearValue( 0.0 );
  spinPositionY->setClearValue( 1000.0 );
  spinPositionZ->setClearValue( 0.0 );
  spinIntensity->setClearValue( 1.0 );
  spinA0->setClearValue( 0.0 );
  spinA1->setClearValue( 0.0 );
  spinA2->setClearValue( 0.0 );
  spinDirectionalIntensity->setClearValue( 1.0 );

  mLightsModel = new QgsLightsModel( this );
  mLightsListView->setModel( mLightsModel );

  connect( mLightsListView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsLightsWidget::selectedLightChanged );

  btnAddLight->setIcon( QIcon( QgsApplication::iconPath( "symbologyAdd.svg" ) ) );
  btnRemoveLight->setIcon( QIcon( QgsApplication::iconPath( "symbologyRemove.svg" ) ) );

  dialAzimuth->setMaximum( 359 );

  QMenu *addLightMenu = new QMenu( this );
  QAction *addPointLight = new QAction( tr( "Point Light" ), addLightMenu );
  connect( addPointLight, &QAction::triggered, this, &QgsLightsWidget::onAddLight );
  addLightMenu->addAction( addPointLight );

  QAction *addDirectionalLight = new QAction( tr( "Directional Light" ), addLightMenu );
  connect( addDirectionalLight, &QAction::triggered, this, &QgsLightsWidget::onAddDirectionalLight );
  addLightMenu->addAction( addDirectionalLight );

  btnAddLight->setMenu( addLightMenu );

  connect( btnRemoveLight, &QToolButton::clicked, this, &QgsLightsWidget::onRemoveLight );

  connect( spinPositionX, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsLightsWidget::updateCurrentLightParameters );
  connect( spinPositionY, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsLightsWidget::updateCurrentLightParameters );
  connect( spinPositionZ, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsLightsWidget::updateCurrentLightParameters );
  connect( spinIntensity, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsLightsWidget::updateCurrentLightParameters );
  connect( btnColor, &QgsColorButton::colorChanged, this, &QgsLightsWidget::updateCurrentLightParameters );
  connect( spinA0, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsLightsWidget::updateCurrentLightParameters );
  connect( spinA1, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsLightsWidget::updateCurrentLightParameters );
  connect( spinA2, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsLightsWidget::updateCurrentLightParameters );

  connect( spinDirectionalIntensity, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsLightsWidget::updateCurrentDirectionalLightParameters );
  connect( btnDirectionalColor, &QgsColorButton::colorChanged, this, &QgsLightsWidget::updateCurrentDirectionalLightParameters );

  connect( dialAzimuth, &QSlider::valueChanged, this, [this]( int value ) {spinBoxAzimuth->setValue( ( value + 180 ) % 360 );} );
  connect( sliderAltitude, &QSlider::valueChanged, spinBoxAltitude, &QgsDoubleSpinBox::setValue );
  connect( spinBoxAzimuth, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsLightsWidget::onDirectionChange );
  connect( spinBoxAltitude, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsLightsWidget::onDirectionChange );

  mLightsListView->selectionModel()->select( mLightsModel->index( 0, 0 ), QItemSelectionModel::ClearAndSelect );
  selectedLightChanged( mLightsListView->selectionModel()->selection(), QItemSelection() );
}

void QgsLightsWidget::setLights( const QList<QgsLightSource *> sources )
{
  QList< QgsPointLightSettings > pointLights;
  QList< QgsDirectionalLightSettings > directionalLights;
  for ( const QgsLightSource *source : sources )
  {
    switch ( source->type() )
    {
      case Qgis::LightSourceType::Point:
        pointLights.append( *qgis::down_cast< const QgsPointLightSettings *>( source ) );
        break;
      case Qgis::LightSourceType::Directional:
        directionalLights.append( *qgis::down_cast< const QgsDirectionalLightSettings *>( source ) );
        break;
    }
  }

  mLightsModel->setPointLights( pointLights );
  mLightsModel->setDirectionalLights( directionalLights );
  mLightsListView->selectionModel()->select( mLightsModel->index( 0, 0 ), QItemSelectionModel::ClearAndSelect );
  selectedLightChanged( mLightsListView->selectionModel()->selection(), QItemSelection() );
}

QList<QgsLightSource *> QgsLightsWidget::lightSources()
{
  QList<QgsLightSource *> res;
  const QList<QgsPointLightSettings> pointLights = mLightsModel->pointLights();
  const QList<QgsDirectionalLightSettings> directionalLights = mLightsModel->directionalLights();
  for ( const QgsPointLightSettings &light : pointLights )
  {
    res.append( light.clone() );
  }
  for ( const QgsDirectionalLightSettings &light : directionalLights )
  {
    res.append( light.clone() );
  }
  return res;
}

int QgsLightsWidget::directionalLightCount() const
{
  return mLightsModel->directionalLights().count();
}

int QgsLightsWidget::lightSourceCount() const
{
  return mLightsModel->rowCount( QModelIndex() );
}

void QgsLightsWidget::selectedLightChanged( const QItemSelection &selected, const QItemSelection & )
{
  if ( selected.empty() )
  {
    mStackedWidget->setCurrentIndex( 0 );
    return;
  }

  const QgsLightsModel::LightType lightType = static_cast< QgsLightsModel::LightType >( mLightsModel->data( selected.indexes().at( 0 ), QgsLightsModel::LightTypeRole ).toInt() );
  const int listIndex = mLightsModel->data( selected.indexes().at( 0 ), QgsLightsModel::LightListIndex ).toInt();

  switch ( lightType )
  {
    case QgsLightsModel::Point:
      mStackedWidget->setCurrentIndex( 1 );
      showSettingsForPointLight( mLightsModel->pointLights().at( listIndex ) );
      break;

    case QgsLightsModel::Directional:
      mStackedWidget->setCurrentIndex( 2 );
      showSettingsForDirectionalLight( mLightsModel->directionalLights().at( listIndex ) );
      break;
  }
}

void QgsLightsWidget::showSettingsForPointLight( const QgsPointLightSettings &light )
{
  whileBlocking( spinPositionX )->setValue( light.position().x() );
  whileBlocking( spinPositionY )->setValue( light.position().y() );
  whileBlocking( spinPositionZ )->setValue( light.position().z() );
  whileBlocking( btnColor )->setColor( light.color() );
  whileBlocking( spinIntensity )->setValue( light.intensity() );
  whileBlocking( spinA0 )->setValue( light.constantAttenuation() );
  whileBlocking( spinA1 )->setValue( light.linearAttenuation() );
  whileBlocking( spinA2 )->setValue( light.quadraticAttenuation() );
}

void QgsLightsWidget::showSettingsForDirectionalLight( const QgsDirectionalLightSettings &light )
{
  mDirectionX = light.direction().x();
  mDirectionY = light.direction().y();
  mDirectionZ = light.direction().z();
  whileBlocking( btnDirectionalColor )->setColor( light.color() );
  whileBlocking( spinDirectionalIntensity )->setValue( light.intensity() );
  setAzimuthAltitude();
}


void QgsLightsWidget::updateCurrentLightParameters()
{
  const int listIndex = mLightsModel->data( mLightsListView->selectionModel()->selection().indexes().at( 0 ), QgsLightsModel::LightListIndex ).toInt();

  QgsPointLightSettings light;
  light.setPosition( QgsVector3D( spinPositionX->value(), spinPositionY->value(), spinPositionZ->value() ) );
  light.setColor( btnColor->color() );
  light.setIntensity( spinIntensity->value() );
  light.setConstantAttenuation( spinA0->value() );
  light.setLinearAttenuation( spinA1->value() );
  light.setQuadraticAttenuation( spinA2->value() );

  mLightsModel->setPointLightSettings( listIndex, light );
}

void QgsLightsWidget::updateCurrentDirectionalLightParameters()
{
  labelX->setText( QString::number( mDirectionX, 'f', 2 ) );
  labelY->setText( QString::number( mDirectionY, 'f', 2 ) );
  labelZ->setText( QString::number( mDirectionZ, 'f', 2 ) );

  const int listIndex = mLightsModel->data( mLightsListView->selectionModel()->selection().indexes().at( 0 ), QgsLightsModel::LightListIndex ).toInt();

  QgsDirectionalLightSettings light;
  light.setDirection( QgsVector3D( mDirectionX, mDirectionY, mDirectionZ ) );
  light.setColor( btnDirectionalColor->color() );
  light.setIntensity( spinDirectionalIntensity->value() );

  mLightsModel->setDirectionalLightSettings( listIndex, light );
}

void QgsLightsWidget::onAddLight()
{
  if ( mLightsModel->pointLights().size() >= 8 )
  {
    QMessageBox::warning( this, tr( "Add Light" ), tr( "It is not possible to add more than 8 lights to the scene." ) );
    return;
  }

  const QModelIndex newIndex = mLightsModel->addPointLight( QgsPointLightSettings() );
  mLightsListView->selectionModel()->select( newIndex, QItemSelectionModel::ClearAndSelect );
  emit lightsAdded();
}

void QgsLightsWidget::onAddDirectionalLight()
{
  if ( mLightsModel->directionalLights().size() >= 4 )
  {
    QMessageBox::warning( this, tr( "Add Directional Light" ), tr( "It is not possible to add more than 4 directional lights to the scene." ) );
    return;
  }

  const QModelIndex newIndex = mLightsModel->addDirectionalLight( QgsDirectionalLightSettings() );
  mLightsListView->selectionModel()->select( newIndex, QItemSelectionModel::ClearAndSelect );
  emit lightsAdded();
  emit directionalLightsCountChanged( mLightsModel->directionalLights().size() );
}

void QgsLightsWidget::onRemoveLight()
{
  const QItemSelection selected = mLightsListView->selectionModel()->selection();
  if ( selected.empty() )
  {
    return;
  }

  const int directionalCount = mLightsModel->directionalLights().size();
  const int pointCount = mLightsModel->pointLights().size();

  mLightsModel->removeRows( selected.indexes().at( 0 ).row(), 1 );

  if ( mLightsModel->directionalLights().size() != directionalCount )
    emit directionalLightsCountChanged( mLightsModel->directionalLights().size() );

  if ( mLightsModel->rowCount( QModelIndex() ) != directionalCount + pointCount )
    emit lightsRemoved();
}

void QgsLightsWidget::setAzimuthAltitude()
{
  double azimuthAngle;
  double altitudeAngle;

  const double horizontalVectorMagnitude = sqrt( mDirectionX * mDirectionX + mDirectionZ * mDirectionZ );

  if ( horizontalVectorMagnitude == 0 )
    azimuthAngle = 0;
  else
  {
    azimuthAngle = ( asin( -mDirectionX / horizontalVectorMagnitude ) ) / M_PI * 180;
    if ( mDirectionZ < 0 )
      azimuthAngle = 180 - azimuthAngle;
    azimuthAngle = std::fmod( azimuthAngle + 360.0, 360.0 );
  }

  whileBlocking( dialAzimuth )->setValue( int( azimuthAngle + 180 ) % 360 );
  whileBlocking( spinBoxAzimuth )->setValue( azimuthAngle );

  if ( horizontalVectorMagnitude == 0 )
    altitudeAngle = 90;
  else
    altitudeAngle = -atan( mDirectionY / horizontalVectorMagnitude ) / M_PI * 180;

  whileBlocking( spinBoxAltitude )->setValue( altitudeAngle );
  whileBlocking( sliderAltitude )->setValue( altitudeAngle );

  updateCurrentDirectionalLightParameters();
}

void QgsLightsWidget::onDirectionChange()
{
  const double altitudeValue = spinBoxAltitude->value();
  const double azimuthValue = spinBoxAzimuth->value();

  const double horizontalVectorMagnitude = cos( altitudeValue / 180 * M_PI );
  mDirectionX = -horizontalVectorMagnitude * sin( azimuthValue / 180 * M_PI );
  mDirectionZ = horizontalVectorMagnitude * cos( azimuthValue / 180 * M_PI );
  mDirectionY = -sin( altitudeValue / 180 * M_PI );

  whileBlocking( sliderAltitude )->setValue( altitudeValue );
  updateCurrentDirectionalLightParameters();
}



//
// QgsLightsModel
//
QgsLightsModel::QgsLightsModel( QObject *parent )
  : QAbstractListModel( parent )
{

}

int QgsLightsModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return mPointLights.size() + mDirectionalLights.size();
}

QVariant QgsLightsModel::data( const QModelIndex &index, int role ) const
{
  if ( index.row() < 0 || index.row() >= rowCount( QModelIndex() ) )
    return QVariant();

  const LightType lightType = index.row() < mPointLights.size() ? Point : Directional;
  const int lightListRow = lightType == Point ? index.row() : index.row() - mPointLights.size();

  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
    case Qt::EditRole:
      switch ( lightType )
      {
        case Point:
          return tr( "Point light %1" ).arg( lightListRow + 1 );

        case Directional:
          return tr( "Directional light %1" ).arg( lightListRow + 1 );
      }
      break;

    case LightTypeRole:
      return lightType;

    case LightListIndex:
      return lightListRow;

    case Qt::DecorationRole:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mActionHighlightFeature.svg" ) );

    default:
      break;
  }
  return QVariant();
}

bool QgsLightsModel::removeRows( int row, int count, const QModelIndex &parent )
{
  beginRemoveRows( parent, row, row + count - 1 );
  for ( int i = row + count - 1; i >= row; --i )
  {
    const LightType lightType = i < mPointLights.size() ? Point : Directional;
    const int lightListRow = lightType == Point ? i : i - mPointLights.size();

    switch ( lightType )
    {
      case Point:
        mPointLights.removeAt( lightListRow );
        break;

      case Directional:
        mDirectionalLights.removeAt( lightListRow );
        break;
    }
  }
  endRemoveRows();
  return true;
}

void QgsLightsModel::setPointLights( const QList<QgsPointLightSettings> &lights )
{
  beginRemoveRows( QModelIndex(), 0, mPointLights.size() - 1 );
  mPointLights.clear();
  endRemoveRows();

  beginInsertRows( QModelIndex(), 0, lights.size() - 1 );
  mPointLights = lights;
  endInsertRows();
}

void QgsLightsModel::setDirectionalLights( const QList<QgsDirectionalLightSettings> &lights )
{
  beginRemoveRows( QModelIndex(), mPointLights.size(), mPointLights.size() + mDirectionalLights.size() - 1 );
  mDirectionalLights.clear();
  endRemoveRows();

  beginInsertRows( QModelIndex(), mPointLights.size(), mPointLights.size() + lights.size() - 1 );
  mDirectionalLights = lights;
  endInsertRows();
}

QList<QgsPointLightSettings> QgsLightsModel::pointLights() const
{
  return mPointLights;
}

QList<QgsDirectionalLightSettings> QgsLightsModel::directionalLights() const
{
  return mDirectionalLights;
}

void QgsLightsModel::setPointLightSettings( int index, const QgsPointLightSettings &light )
{
  mPointLights[ index ] = light;
}

void QgsLightsModel::setDirectionalLightSettings( int index, const QgsDirectionalLightSettings &light )
{
  mDirectionalLights[ index ] = light;
}

QModelIndex QgsLightsModel::addPointLight( const QgsPointLightSettings &light )
{
  beginInsertRows( QModelIndex(), mPointLights.size(), mPointLights.size() );
  mPointLights.append( light );
  endInsertRows();

  return index( mPointLights.size() - 1 );
}

QModelIndex QgsLightsModel::addDirectionalLight( const QgsDirectionalLightSettings &light )
{
  beginInsertRows( QModelIndex(), mPointLights.size() + mDirectionalLights.size(), mPointLights.size() + mDirectionalLights.size() );
  mDirectionalLights.append( light );
  endInsertRows();

  return index( mPointLights.size() + mDirectionalLights.size() - 1 );
}
