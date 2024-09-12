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
#include "qgis.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectorlayer.h"
#include "qgsfield.h"
#include "qgsfields.h"
#include "qgsmemoryproviderutils.h"
#include "qgsgui.h"
#include "qgsiconutils.h"
#include "qgsvariantutils.h"

#include <QPushButton>
#include <QComboBox>
#include <QUuid>
#include <QFileDialog>
#include <QMessageBox>

QgsVectorLayer *QgsNewMemoryLayerDialog::runAndCreateLayer( QWidget *parent, const QgsCoordinateReferenceSystem &defaultCrs )
{
  QgsNewMemoryLayerDialog dialog( parent );
  dialog.setCrs( defaultCrs );
  if ( dialog.exec() == QDialog::Rejected )
  {
    return nullptr;
  }

  const Qgis::WkbType geometrytype = dialog.selectedType();
  const QgsFields fields = dialog.fields();
  const QString name = dialog.layerName().isEmpty() ? tr( "New scratch layer" ) : dialog.layerName();
  QgsVectorLayer *newLayer = QgsMemoryProviderUtils::createMemoryLayer( name, fields, geometrytype, dialog.crs() );
  return newLayer;
}

QgsNewMemoryLayerDialog::QgsNewMemoryLayerDialog( QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  mNameLineEdit->setText( tr( "New scratch layer" ) );

  const Qgis::WkbType geomTypes[] =
  {
    Qgis::WkbType::NoGeometry,
    Qgis::WkbType::Point,
    Qgis::WkbType::LineString,
    Qgis::WkbType::CompoundCurve,
    Qgis::WkbType::Polygon,
    Qgis::WkbType::CurvePolygon,
    Qgis::WkbType::MultiPoint,
    Qgis::WkbType::MultiLineString,
    Qgis::WkbType::MultiCurve,
    Qgis::WkbType::MultiPolygon,
    Qgis::WkbType::MultiSurface,
  };

  for ( const auto type : geomTypes )
    mGeometryTypeBox->addItem( QgsIconUtils::iconForWkbType( type ), QgsWkbTypes::translatedDisplayString( type ), static_cast< quint32>( type ) );
  mGeometryTypeBox->setCurrentIndex( -1 );

  mGeometryWithZCheckBox->setEnabled( false );
  mGeometryWithMCheckBox->setEnabled( false );
  mCrsSelector->setEnabled( false );
  mCrsSelector->setShowAccuracyWarnings( true );

  mTypeBox->addItem( QgsFields::iconForFieldType( QVariant::String ), QgsVariantUtils::typeToDisplayString( QVariant::String ), "string" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QVariant::Int ), QgsVariantUtils::typeToDisplayString( QVariant::Int ), "integer" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QVariant::Double ), QgsVariantUtils::typeToDisplayString( QVariant::Double ), "double" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QVariant::Bool ), QgsVariantUtils::typeToDisplayString( QVariant::Bool ), "bool" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QVariant::Date ), QgsVariantUtils::typeToDisplayString( QVariant::Date ), "date" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QVariant::Time ), QgsVariantUtils::typeToDisplayString( QVariant::Time ), "time" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QVariant::DateTime ), QgsVariantUtils::typeToDisplayString( QVariant::DateTime ), "datetime" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QVariant::ByteArray ), QgsVariantUtils::typeToDisplayString( QVariant::ByteArray ), "binary" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QVariant::StringList ), QgsVariantUtils::typeToDisplayString( QVariant::StringList ), "stringlist" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QVariant::List, QVariant::Int ), QgsVariantUtils::typeToDisplayString( QVariant::List, QVariant::Int ), "integerlist" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QVariant::List, QVariant::Double ), QgsVariantUtils::typeToDisplayString( QVariant::List, QVariant::Double ), "doublelist" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QVariant::List, QVariant::LongLong ), QgsVariantUtils::typeToDisplayString( QVariant::List, QVariant::LongLong ), "integer64list" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QVariant::Map ), QgsVariantUtils::typeToDisplayString( QVariant::Map ), "map" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QVariant::UserType, QVariant::Invalid, QStringLiteral( "geometry" ) ), tr( "Geometry" ), "geometry" );
  mTypeBox_currentIndexChanged( 0 );

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
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsNewMemoryLayerDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QgsNewMemoryLayerDialog::reject );

  mNameLineEdit->selectAll();
  mNameLineEdit->setFocus();
}

Qgis::WkbType QgsNewMemoryLayerDialog::selectedType() const
{
  Qgis::WkbType geomType = Qgis::WkbType::Unknown;
  geomType = static_cast<Qgis::WkbType>
             ( mGeometryTypeBox->currentData( Qt::UserRole ).toInt() );

  if ( geomType != Qgis::WkbType::Unknown && geomType != Qgis::WkbType::NoGeometry )
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
  const Qgis::WkbType geomType = static_cast<Qgis::WkbType>
                                 ( mGeometryTypeBox->currentData( Qt::UserRole ).toInt() );

  const bool isSpatial = geomType != Qgis::WkbType::NoGeometry;
  mGeometryWithZCheckBox->setEnabled( isSpatial );
  mGeometryWithMCheckBox->setEnabled( isSpatial );
  mCrsSelector->setEnabled( isSpatial );

  const bool ok = ( !mNameLineEdit->text().isEmpty() && mGeometryTypeBox->currentIndex() != -1 );
  mOkButton->setEnabled( ok );
}

void QgsNewMemoryLayerDialog::mTypeBox_currentIndexChanged( int )
{
  const QString fieldType = mTypeBox->currentData().toString();
  if ( fieldType == QLatin1String( "string" ) )
  {
    if ( mWidth->text().toInt() < 1 || mWidth->text().toInt() > 255 )
      mWidth->setText( QStringLiteral( "255" ) );
    mPrecision->clear();
    mPrecision->setEnabled( false );
    mWidth->setValidator( new QIntValidator( 1, 255, this ) );
    mWidth->setEnabled( true );
  }
  else if ( fieldType == QLatin1String( "integer" ) )
  {
    if ( mWidth->text().toInt() < 1 || mWidth->text().toInt() > 10 )
      mWidth->setText( QStringLiteral( "10" ) );
    mPrecision->clear();
    mPrecision->setEnabled( false );
    mWidth->setValidator( new QIntValidator( 1, 10, this ) );
    mWidth->setEnabled( true );
  }
  else if ( fieldType == QLatin1String( "double" ) )
  {
    if ( mWidth->text().toInt() < 1 || mWidth->text().toInt() > 30 )
      mWidth->setText( QStringLiteral( "30" ) );
    if ( mPrecision->text().toInt() < 1 || mPrecision->text().toInt() > 30 )
      mPrecision->setText( QStringLiteral( "6" ) );
    mPrecision->setEnabled( true );
    mWidth->setValidator( new QIntValidator( 1, 20, this ) );
    mWidth->setEnabled( true );
  }
  else if ( fieldType == QLatin1String( "bool" ) )
  {
    mWidth->clear();
    mWidth->setEnabled( false );
    mPrecision->clear();
    mPrecision->setEnabled( false );
  }
  else if ( fieldType == QLatin1String( "date" ) )
  {
    mWidth->clear();
    mWidth->setEnabled( false );
    mPrecision->clear();
    mPrecision->setEnabled( false );
  }
  else if ( fieldType == QLatin1String( "time" ) )
  {
    mWidth->clear();
    mWidth->setEnabled( false );
    mPrecision->clear();
    mPrecision->setEnabled( false );
  }
  else if ( fieldType == QLatin1String( "datetime" ) )
  {
    mWidth->clear();
    mWidth->setEnabled( false );
    mPrecision->clear();
    mPrecision->setEnabled( false );
  }
  else if ( fieldType == QStringLiteral( "binary" )
            || fieldType == QStringLiteral( "stringlist" )
            || fieldType == QStringLiteral( "integerlist" )
            || fieldType == QStringLiteral( "doublelist" )
            || fieldType == QStringLiteral( "integer64list" )
            || fieldType == QStringLiteral( "map" )
            || fieldType == QLatin1String( "geometry" ) )
  {
    mWidth->clear();
    mWidth->setEnabled( false );
    mPrecision->clear();
    mPrecision->setEnabled( false );
  }
  else
  {
    QgsDebugError( QStringLiteral( "unexpected index" ) );
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
    const QString name( ( *it )->text( 0 ) );
    const QString typeName( ( *it )->text( 1 ) );
    const int width = ( *it )->text( 2 ).toInt();
    const int precision = ( *it )->text( 3 ).toInt();
    QVariant::Type fieldType = QVariant::Invalid;
    QVariant::Type fieldSubType = QVariant::Invalid;
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
    else if ( typeName == QLatin1String( "binary" ) )
      fieldType = QVariant::ByteArray;
    else if ( typeName == QLatin1String( "stringlist" ) )
    {
      fieldType = QVariant::StringList;
      fieldSubType = QVariant::String;
    }
    else if ( typeName == QLatin1String( "integerlist" ) )
    {
      fieldType = QVariant::List;
      fieldSubType = QVariant::Int;
    }
    else if ( typeName == QLatin1String( "doublelist" ) )
    {
      fieldType = QVariant::List;
      fieldSubType = QVariant::Double;
    }
    else if ( typeName == QLatin1String( "integer64list" ) )
    {
      fieldType = QVariant::List;
      fieldSubType = QVariant::LongLong;
    }
    else if ( typeName == QLatin1String( "map" ) )
      fieldType = QVariant::Map;
    else if ( typeName == QLatin1String( "geometry" ) )
      fieldType = QVariant::UserType;

    const QgsField field = QgsField( name, fieldType, typeName, width, precision, QString(), fieldSubType );
    fields.append( field );
    ++it;
  }

  return fields;
}

void QgsNewMemoryLayerDialog::accept()
{
  if ( !mFieldNameEdit->text().trimmed().isEmpty() )
  {
    const QString currentFieldName = mFieldNameEdit->text();
    if ( fields().lookupField( currentFieldName ) == -1 )
    {
      if ( QMessageBox::question( this, tr( "New Temporary Scratch Layer" ),
                                  tr( "The field “%1” has not been added to the fields list. Are you sure you want to proceed and discard this field?" ).arg( currentFieldName ),
                                  QMessageBox::Ok | QMessageBox::Cancel ) != QMessageBox::Ok )
      {
        return;
      }
    }
  }

  QDialog::accept();
}

void QgsNewMemoryLayerDialog::mAddAttributeButton_clicked()
{
  if ( !mFieldNameEdit->text().isEmpty() )
  {
    const QString fieldName = mFieldNameEdit->text();
    const QString fieldType = mTypeBox->currentData( Qt::UserRole ).toString();
    const QString width = mWidth->text();
    const QString precision = mPrecision->text();
    mAttributeView->addTopLevelItem( new QTreeWidgetItem( QStringList() << fieldName << fieldType << width << precision ) );

    mFieldNameEdit->clear();

    if ( !mFieldNameEdit->hasFocus() )
    {
      mFieldNameEdit->setFocus();
    }
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
