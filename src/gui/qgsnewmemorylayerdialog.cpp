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

#include <QPushButton>
#include <QComboBox>
#include <QLibrary>
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

  QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Windows/NewMemoryLayer/geometry" ) ).toByteArray() );

  mPointRadioButton->setChecked( true );
  mNameLineEdit->setText( tr( "New scratch layer" ) );

  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsNewMemoryLayerDialog::showHelp );
}

QgsNewMemoryLayerDialog::~QgsNewMemoryLayerDialog()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/NewMemoryLayer/geometry" ), saveGeometry() );
}

QgsWkbTypes::Type QgsNewMemoryLayerDialog::selectedType() const
{
  QgsWkbTypes::Type wkbType = QgsWkbTypes::Unknown;
  if ( !buttonGroupGeometry->isChecked() )
  {
    wkbType = QgsWkbTypes::NoGeometry;
  }
  else if ( mPointRadioButton->isChecked() )
  {
    wkbType = QgsWkbTypes::Point;
  }
  else if ( mLineRadioButton->isChecked() )
  {
    wkbType = QgsWkbTypes::LineString;
  }
  else if ( mPolygonRadioButton->isChecked() )
  {
    wkbType = QgsWkbTypes::Polygon;
  }
  else if ( mMultiPointRadioButton->isChecked() )
  {
    wkbType = QgsWkbTypes::MultiPoint;
  }
  else if ( mMultiLineRadioButton->isChecked() )
  {
    wkbType = QgsWkbTypes::MultiLineString;
  }
  else if ( mMultiPolygonRadioButton->isChecked() )
  {
    wkbType = QgsWkbTypes::MultiPolygon;
  }

  if ( mGeometryWithZCheckBox->isChecked() && wkbType != QgsWkbTypes::Unknown && wkbType != QgsWkbTypes::NoGeometry )
    wkbType = QgsWkbTypes::zmType( wkbType, true, true );

  return wkbType;
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
