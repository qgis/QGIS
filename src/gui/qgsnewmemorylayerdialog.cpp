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
#include "qgsfield.h"
#include "qgsfields.h"
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
  QgsFields fields = dialog.fields();
  QString name = dialog.layerName().isEmpty() ? tr( "New scratch layer" ) : dialog.layerName();
  QgsVectorLayer *newLayer = QgsMemoryProviderUtils::createMemoryLayer( name, fields, geometrytype, dialog.crs() );
  return newLayer;
}

QgsNewMemoryLayerDialog::QgsNewMemoryLayerDialog( QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  mNameLineEdit->setText( tr( "New scratch layer" ) );

  mGeometryTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconTableLayer.svg" ) ), tr( "No Geometry" ), QgsWkbTypes::NoGeometry );
  mGeometryTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconPointLayer.svg" ) ), tr( "Point" ), QgsWkbTypes::Point );
  mGeometryTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconLineLayer.svg" ) ), tr( "LineString" ), QgsWkbTypes::LineString );
  mGeometryTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconLineLayer.svg" ) ), tr( "CompoundCurve" ), QgsWkbTypes::CompoundCurve );
  mGeometryTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconPolygonLayer.svg" ) ), tr( "Polygon" ), QgsWkbTypes::Polygon );
  mGeometryTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconPolygonLayer.svg" ) ), tr( "CurvePolygon" ), QgsWkbTypes::CurvePolygon );
  mGeometryTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconPointLayer.svg" ) ), tr( "MultiPoint" ), QgsWkbTypes::MultiPoint );
  mGeometryTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconLineLayer.svg" ) ), tr( "MultiLineString" ), QgsWkbTypes::MultiLineString );
  mGeometryTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconLineLayer.svg" ) ), tr( "MultiCurve" ), QgsWkbTypes::MultiCurve );
  mGeometryTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconPolygonLayer.svg" ) ), tr( "MultiPolygon" ), QgsWkbTypes::MultiPolygon );
  mGeometryTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconPolygonLayer.svg" ) ), tr( "MultiSurface" ), QgsWkbTypes::MultiSurface );
  mGeometryTypeBox->setCurrentIndex( -1 );

  mGeometryWithZCheckBox->setEnabled( false );
  mGeometryWithMCheckBox->setEnabled( false );
  mCrsSelector->setEnabled( false );

  mTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldText.svg" ) ), tr( "Text" ), "string" );
  mTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldInteger.svg" ) ), tr( "Whole Number" ), "integer" );
  mTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldFloat.svg" ) ), tr( "Decimal Number" ), "double" );
  mTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldBool.svg" ) ), tr( "Boolean" ), "bool" );
  mTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldDate.svg" ) ), tr( "Date" ), "date" );
  mTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldTime.svg" ) ), tr( "Time" ), "time" );
  mTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldDateTime.svg" ) ), tr( "Date and Time" ), "datetime" );
  mTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldBinary.svg" ) ), tr( "Binary (BLOB)" ), "binary" );
  mTypeBox_currentIndexChanged( 1 );

  mWidth->setValidator( new QIntValidator( 1, 255, this ) );
  mPrecision->setValidator( new QIntValidator( 0, 30, this ) );

  mAddAttributeButton->setEnabled( false );
  mRemoveAttributeButton->setEnabled( false );

  mOkButton = mButtonBox->button( QDialogButtonBox::Ok );
  mOkButton->setEnabled( false );

  connect( mGeometryTypeBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsNewMemoryLayerDialog::geometryTypeChanged );
  connect( mFieldNameEdit, &QLineEdit::textChanged, this, &QgsNewMemoryLayerDialog::fieldNameChanged );
  connect( mTypeBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsNewMemoryLayerDialog::mTypeBox_currentIndexChanged );
  connect( mAttributeView, &QTreeWidget::itemSelectionChanged, this, &QgsNewMemoryLayerDialog::selectionChanged );
  connect( mAddAttributeButton, &QToolButton::clicked, this, &QgsNewMemoryLayerDialog::mAddAttributeButton_clicked );
  connect( mRemoveAttributeButton, &QToolButton::clicked, this, &QgsNewMemoryLayerDialog::mRemoveAttributeButton_clicked );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsNewMemoryLayerDialog::showHelp );
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

  bool ok = ( !mNameLineEdit->text().isEmpty() && mGeometryTypeBox->currentIndex() != -1 );
  mOkButton->setEnabled( ok );
}

void QgsNewMemoryLayerDialog::mTypeBox_currentIndexChanged( int index )
{
  switch ( index )
  {
    case 0: // Text data
      if ( mWidth->text().toInt() < 1 || mWidth->text().toInt() > 255 )
        mWidth->setText( QStringLiteral( "255" ) );
      mPrecision->clear();
      mPrecision->setEnabled( false );
      mWidth->setValidator( new QIntValidator( 1, 255, this ) );
      break;
    case 1: // Whole number
      if ( mWidth->text().toInt() < 1 || mWidth->text().toInt() > 10 )
        mWidth->setText( QStringLiteral( "10" ) );
      mPrecision->clear();
      mPrecision->setEnabled( false );
      mWidth->setValidator( new QIntValidator( 1, 10, this ) );
      break;
    case 2: // Decimal number
      if ( mWidth->text().toInt() < 1 || mWidth->text().toInt() > 30 )
        mWidth->setText( QStringLiteral( "30" ) );
      if ( mPrecision->text().toInt() < 1 || mPrecision->text().toInt() > 30 )
        mPrecision->setText( QStringLiteral( "6" ) );
      mPrecision->setEnabled( true );
      mWidth->setValidator( new QIntValidator( 1, 20, this ) );
      break;
    case 3: // Boolean
      mWidth->clear();
      mWidth->setEnabled( false );
      mPrecision->clear();
      mPrecision->setEnabled( false );
      break;
    case 4: // Date
      mWidth->clear();
      mWidth->setEnabled( false );
      mPrecision->clear();
      mPrecision->setEnabled( false );
      break;
    case 5: // Time
      mWidth->clear();
      mWidth->setEnabled( false );
      mPrecision->clear();
      mPrecision->setEnabled( false );
      break;
    case 6: // Datetime
      mWidth->clear();
      mWidth->setEnabled( false );
      mPrecision->clear();
      mPrecision->setEnabled( false );
      break;
    case 7: // Binary
      mWidth->clear();
      mWidth->setEnabled( false );
      mPrecision->clear();
      mPrecision->setEnabled( false );
      break;

    default:
      QgsDebugMsg( QStringLiteral( "unexpected index" ) );
      break;
  }
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

void QgsNewMemoryLayerDialog::fieldNameChanged( const QString &name )
{
  mAddAttributeButton->setDisabled( name.isEmpty() || ! mAttributeView->findItems( name, Qt::MatchExactly ).isEmpty() );
}

void QgsNewMemoryLayerDialog::selectionChanged()
{
  mRemoveAttributeButton->setDisabled( mAttributeView->selectedItems().isEmpty() );
}

QgsFields QgsNewMemoryLayerDialog::fields() const
{
  QgsFields fields = QgsFields();

  QTreeWidgetItemIterator it( mAttributeView );
  while ( *it )
  {
    QString name( ( *it )->text( 0 ) );
    QString typeName( ( *it )->text( 1 ) );
    int width = ( *it )->text( 2 ).toInt();
    int precision = ( *it )->text( 3 ).toInt();
    QVariant::Type fieldType = QVariant::Invalid;
    if ( typeName == QLatin1String( "string" ) )
      fieldType = QVariant::String;
    else if ( typeName == QLatin1String( "integer" ) )
      fieldType = QVariant::Int;
    else if ( typeName == QLatin1String( "double" ) )
      fieldType = QVariant::Double;
    else if ( typeName == QLatin1String( "bool" ) )
      fieldType = QVariant::Bool;
    else if ( typeName == QLatin1String( "date" ) )
      fieldType = QVariant::Date;
    else if ( typeName == QLatin1String( "time" ) )
      fieldType = QVariant::Time;
    else if ( typeName == QLatin1String( "datetime" ) )
      fieldType = QVariant::DateTime;

    QgsField field = QgsField( name, fieldType, typeName, width, precision );
    fields.append( field );
    ++it;
  }

  return fields;
}

void QgsNewMemoryLayerDialog::mAddAttributeButton_clicked()
{
  if ( !mFieldNameEdit->text().isEmpty() )
  {
    QString fieldName = mFieldNameEdit->text();
    QString fieldType = mTypeBox->currentData( Qt::UserRole ).toString();
    QString width = mWidth->text();
    QString precision = mPrecision->text();
    mAttributeView->addTopLevelItem( new QTreeWidgetItem( QStringList() << fieldName << fieldType << width << precision ) );

    mFieldNameEdit->clear();
  }
}

void QgsNewMemoryLayerDialog::mRemoveAttributeButton_clicked()
{
  delete mAttributeView->currentItem();
}

void QgsNewMemoryLayerDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/create_layers.html#creating-a-new-temporary-scratch-layer" ) );
}
