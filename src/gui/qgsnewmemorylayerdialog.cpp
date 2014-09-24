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
#include "qgsgenericprojectionselector.h"
#include "qgsproviderregistry.h"
#include "qgsvectordataprovider.h"

#include <QPushButton>
#include <QComboBox>
#include <QLibrary>
#include <QSettings>
#include <QFileDialog>

QgsVectorLayer *QgsNewMemoryLayerDialog::runAndCreateLayer( QWidget *parent )
{
  QgsNewMemoryLayerDialog dialog( parent );
  if ( dialog.exec() == QDialog::Rejected )
  {
    return 0;
  }

  QGis::WkbType geometrytype = dialog.selectedType();
  QString crsId = dialog.selectedCrsId();

  QString geomType;
  switch ( geometrytype )
  {
    case QGis::WKBPoint:
      geomType = "point";
      break;
    case QGis::WKBLineString:
      geomType = "linestring";
      break;
    case QGis::WKBPolygon:
      geomType = "polygon";
      break;
    case QGis::WKBMultiPoint:
      geomType = "multipoint";
      break;
    case QGis::WKBMultiLineString:
      geomType = "multilinestring";
      break;
    case QGis::WKBMultiPolygon:
      geomType = "multipolygon";
      break;
    default:
      geomType = "point";
  }

  QString layerProperties = geomType + QString( "?crs=%1" ).arg( crsId );
  QString name = dialog.layerName().isEmpty() ? tr( "New scratch layer" ) : dialog.layerName();
  QgsVectorLayer* newLayer = new QgsVectorLayer( layerProperties, name, QString( "memory" ) );
  return newLayer;
}

QgsNewMemoryLayerDialog::QgsNewMemoryLayerDialog( QWidget *parent, Qt::WindowFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/NewMemoryLayer/geometry" ).toByteArray() );

  mPointRadioButton->setChecked( true );

  QgsCoordinateReferenceSystem srs;
  srs.createFromOgcWmsCrs( settings.value( "/Projections/layerDefaultCrs", GEO_EPSG_CRS_AUTHID ).toString() );
  srs.validate();
  mCrsId = srs.authid();
  mSpatialRefSysEdit->setText( srs.authid() + " - " + srs.description() );

  mNameLineEdit->setText( tr( "New scratch layer" ) );
}

QgsNewMemoryLayerDialog::~QgsNewMemoryLayerDialog()
{
  QSettings settings;
  settings.setValue( "/Windows/NewMemoryLayer/geometry", saveGeometry() );
}

QGis::WkbType QgsNewMemoryLayerDialog::selectedType() const
{
  if ( mPointRadioButton->isChecked() )
  {
    return QGis::WKBPoint;
  }
  else if ( mLineRadioButton->isChecked() )
  {
    return QGis::WKBLineString;
  }
  else if ( mPolygonRadioButton->isChecked() )
  {
    return QGis::WKBPolygon;
  }
  else if ( mMultiPointRadioButton->isChecked() )
  {
    return QGis::WKBMultiPoint;
  }
  else if ( mMultiLineRadioButton->isChecked() )
  {
    return QGis::WKBMultiLineString;
  }
  else if ( mMultiPolygonRadioButton->isChecked() )
  {
    return QGis::WKBMultiPolygon;
  }
  return QGis::WKBUnknown;
}

QString QgsNewMemoryLayerDialog::layerName() const
{
  return mNameLineEdit->text();
}

QString QgsNewMemoryLayerDialog::selectedCrsId() const
{
  return mCrsId;
}

void QgsNewMemoryLayerDialog::on_mChangeSrsButton_clicked()
{
  QgsGenericProjectionSelector *selector = new QgsGenericProjectionSelector( this );
  selector->setMessage();
  selector->setSelectedAuthId( mCrsId );
  if ( selector->exec() )
  {
    mCrsId = selector->selectedAuthId();
    mSpatialRefSysEdit->setText( mCrsId );
  }
  else
  {
    QApplication::restoreOverrideCursor();
  }
  delete selector;
}
