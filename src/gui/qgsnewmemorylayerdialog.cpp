/***************************************************************************
                         qgsnewmemorylayerdialog.cpp
                             -------------------
    begin                : September 2014
    copyright            : (C) 2014 by Nyall Dawson, Marco Hugentobler
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

#include "qgsnewmemorylayerdialog.h"
#include "qgsapplication.h"
#include "qgis.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsproviderregistry.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgssettings.h"
#include "qgsmemoryproviderutils.h"
#include "qgsgui.h"

#include <QPushButton>
#include <QComboBox>
#include <QUuid>
#include <QFileDialog>

QgsVectorLayer *QgsNewMemoryLayerDialog::runAndCreateLayer( QWidget *parent, const QgsCoordinateReferenceSystem &defaultCrs )
{
  QgsNewMemoryLayerDialog dialog( parent );
  dialog.setCrs( defaultCrs );
  if ( dialog.exec() == QDialog::Rejected )
  {
    return nullptr;
  }

  QgsWkbTypes::Type geometrytype = dialog.selectedType();
  QString name = dialog.layerName().isEmpty() ? tr( "New scratch layer" ) : dialog.layerName();
  QgsVectorLayer *newLayer = QgsMemoryProviderUtils::createMemoryLayer( name, QgsFields(), geometrytype, dialog.crs() );
  return newLayer;
}

QgsNewMemoryLayerDialog::QgsNewMemoryLayerDialog( QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  mGeometryTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconTableLayer.svg" ) ), tr( "No geometry" ), QgsWkbTypes::NoGeometry );
  mGeometryTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconPointLayer.svg" ) ), tr( "Point" ), QgsWkbTypes::Point );
  mGeometryTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconLineLayer.svg" ) ), tr( "LineString / CompoundCurve" ), QgsWkbTypes::LineString );
  mGeometryTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconPolygonLayer.svg" ) ), tr( "Polygon / CurvePolygon" ), QgsWkbTypes::Polygon );
  mGeometryTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconPointLayer.svg" ) ), tr( "MultiPoint" ), QgsWkbTypes::MultiPoint );
  mGeometryTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconLineLayer.svg" ) ), tr( "MultiLineString / MultiCurve" ), QgsWkbTypes::MultiLineString );
  mGeometryTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconPolygonLayer.svg" ) ), tr( "MultiPolygon / MultiSurface" ), QgsWkbTypes::MultiPolygon );

  mGeometryWithZCheckBox->setEnabled( false );
  mGeometryWithMCheckBox->setEnabled( false );

  mNameLineEdit->setText( tr( "New scratch layer" ) );

  connect( mGeometryTypeBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsNewMemoryLayerDialog::geometryTypeChanged );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsNewMemoryLayerDialog::showHelp );
  geometryTypeChanged( mGeometryTypeBox->currentIndex() );
}

QgsWkbTypes::Type QgsNewMemoryLayerDialog::selectedType() const
{
  QgsWkbTypes::Type geomType = QgsWkbTypes::Unknown;
  geomType = static_cast<QgsWkbTypes::Type>
             ( mGeometryTypeBox->currentData( Qt::UserRole ).toInt() );

  if ( geomType != QgsWkbTypes::Unknown && geomType != QgsWkbTypes::NoGeometry )
  {
    if ( mGeometryWithZCheckBox->isChecked() )
      geomType = QgsWkbTypes::addZ( geomType );
    if ( mGeometryWithMCheckBox->isChecked() )
      geomType = QgsWkbTypes::addM( geomType );
  }

  return geomType;
}

void QgsNewMemoryLayerDialog::geometryTypeChanged( int )
{
  QgsWkbTypes::Type geomType = static_cast<QgsWkbTypes::Type>
                               ( mGeometryTypeBox->currentData( Qt::UserRole ).toInt() );

  bool isSpatial = geomType != QgsWkbTypes::NoGeometry;
  mGeometryWithZCheckBox->setEnabled( isSpatial );
  mGeometryWithMCheckBox->setEnabled( isSpatial );
  mCrsSelector->setEnabled( isSpatial );
}

void QgsNewMemoryLayerDialog::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  mCrsSelector->setCrs( crs );
}

QgsCoordinateReferenceSystem QgsNewMemoryLayerDialog::crs() const
{
  return mCrsSelector->crs();
}

QString QgsNewMemoryLayerDialog::layerName() const
{
  return mNameLineEdit->text();
}

void QgsNewMemoryLayerDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/create_layers.html#creating-a-new-temporary-scratch-layer" ) );
}
