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
#include "qgsfield.h"
#include "qgsfields.h"
#include "qgsgui.h"
#include "qgsiconutils.h"
#include "qgsmemoryproviderutils.h"
#include "qgsvariantutils.h"
#include "qgsvectorlayer.h"

#include <QComboBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QUuid>

#include "moc_qgsnewmemorylayerdialog.cpp"

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

  const Qgis::WkbType geomTypes[] = {
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
    Qgis::WkbType::Triangle,
    Qgis::WkbType::PolyhedralSurface,
    Qgis::WkbType::TIN,
  };

  for ( const auto type : geomTypes )
    mGeometryTypeBox->addItem( QgsIconUtils::iconForWkbType( type ), QgsWkbTypes::translatedDisplayString( type ), static_cast<quint32>( type ) );
  mGeometryTypeBox->setCurrentIndex( -1 );

  mGeometryWithZCheckBox->setEnabled( false );
  mGeometryWithMCheckBox->setEnabled( false );
  mCrsSelector->setEnabled( false );
  mCrsSelector->setShowAccuracyWarnings( true );

  mTypeBox->addItem( QgsFields::iconForFieldType( QMetaType::Type::QString ), QgsVariantUtils::typeToDisplayString( QMetaType::Type::QString ), "string" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QMetaType::Type::Int ), QgsVariantUtils::typeToDisplayString( QMetaType::Type::Int ), "integer" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QMetaType::Type::Double ), QgsVariantUtils::typeToDisplayString( QMetaType::Type::Double ), "double" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QMetaType::Type::Bool ), QgsVariantUtils::typeToDisplayString( QMetaType::Type::Bool ), "bool" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QMetaType::Type::QDate ), QgsVariantUtils::typeToDisplayString( QMetaType::Type::QDate ), "date" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QMetaType::Type::QTime ), QgsVariantUtils::typeToDisplayString( QMetaType::Type::QTime ), "time" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QMetaType::Type::QDateTime ), QgsVariantUtils::typeToDisplayString( QMetaType::Type::QDateTime ), "datetime" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QMetaType::Type::QByteArray ), QgsVariantUtils::typeToDisplayString( QMetaType::Type::QByteArray ), "binary" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QMetaType::Type::QStringList ), QgsVariantUtils::typeToDisplayString( QMetaType::Type::QStringList ), "stringlist" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QMetaType::Type::QVariantList, QMetaType::Type::Int ), QgsVariantUtils::typeToDisplayString( QMetaType::Type::QVariantList, QMetaType::Type::Int ), "integerlist" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QMetaType::Type::QVariantList, QMetaType::Type::Double ), QgsVariantUtils::typeToDisplayString( QMetaType::Type::QVariantList, QMetaType::Type::Double ), "doublelist" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QMetaType::Type::QVariantList, QMetaType::Type::LongLong ), QgsVariantUtils::typeToDisplayString( QMetaType::Type::QVariantList, QMetaType::Type::LongLong ), "integer64list" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QMetaType::Type::QVariantMap ), QgsVariantUtils::typeToDisplayString( QMetaType::Type::QVariantMap ), "map" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QMetaType::Type::User, QMetaType::Type::UnknownType, u"geometry"_s ), tr( "Geometry" ), "geometry" );
  mTypeBox_currentIndexChanged( 0 );

  mWidth->setValidator( new QIntValidator( 1, 255, this ) );
  mPrecision->setValidator( new QIntValidator( 0, 30, this ) );

  mAddAttributeButton->setEnabled( false );
  mRemoveAttributeButton->setEnabled( false );
  mButtonUp->setEnabled( false );
  mButtonDown->setEnabled( false );

  mOkButton = mButtonBox->button( QDialogButtonBox::Ok );
  mOkButton->setEnabled( false );

  connect( mGeometryTypeBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsNewMemoryLayerDialog::geometryTypeChanged );
  connect( mFieldNameEdit, &QLineEdit::textChanged, this, &QgsNewMemoryLayerDialog::fieldNameChanged );
  connect( mTypeBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsNewMemoryLayerDialog::mTypeBox_currentIndexChanged );
  connect( mAttributeView, &QTreeWidget::itemSelectionChanged, this, &QgsNewMemoryLayerDialog::selectionChanged );
  connect( mAddAttributeButton, &QToolButton::clicked, this, &QgsNewMemoryLayerDialog::mAddAttributeButton_clicked );
  connect( mRemoveAttributeButton, &QToolButton::clicked, this, &QgsNewMemoryLayerDialog::mRemoveAttributeButton_clicked );
  connect( mButtonUp, &QToolButton::clicked, this, &QgsNewMemoryLayerDialog::moveFieldsUp );
  connect( mButtonDown, &QToolButton::clicked, this, &QgsNewMemoryLayerDialog::moveFieldsDown );

  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsNewMemoryLayerDialog::showHelp );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsNewMemoryLayerDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QgsNewMemoryLayerDialog::reject );

  mNameLineEdit->selectAll();
  mNameLineEdit->setFocus();
}

Qgis::WkbType QgsNewMemoryLayerDialog::selectedType() const
{
  Qgis::WkbType geomType = Qgis::WkbType::Unknown;
  geomType = static_cast<Qgis::WkbType>( mGeometryTypeBox->currentData( Qt::UserRole ).toInt() );

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
  const Qgis::WkbType geomType = static_cast<Qgis::WkbType>( mGeometryTypeBox->currentData( Qt::UserRole ).toInt() );

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
  if ( fieldType == "string"_L1 )
  {
    if ( mWidth->text().toInt() < 1 || mWidth->text().toInt() > 255 )
      mWidth->setText( u"255"_s );
    mPrecision->clear();
    mPrecision->setEnabled( false );
    mWidth->setValidator( new QIntValidator( 1, 255, this ) );
    mWidth->setEnabled( true );
  }
  else if ( fieldType == "integer"_L1 )
  {
    if ( mWidth->text().toInt() < 1 || mWidth->text().toInt() > 10 )
      mWidth->setText( u"10"_s );
    mPrecision->clear();
    mPrecision->setEnabled( false );
    mWidth->setValidator( new QIntValidator( 1, 10, this ) );
    mWidth->setEnabled( true );
  }
  else if ( fieldType == "double"_L1 )
  {
    if ( mWidth->text().toInt() < 1 || mWidth->text().toInt() > 30 )
      mWidth->setText( u"30"_s );
    if ( mPrecision->text().toInt() < 1 || mPrecision->text().toInt() > 30 )
      mPrecision->setText( u"6"_s );
    mPrecision->setEnabled( true );
    mWidth->setValidator( new QIntValidator( 1, 20, this ) );
    mWidth->setEnabled( true );
  }
  else if ( fieldType == "bool"_L1 )
  {
    mWidth->clear();
    mWidth->setEnabled( false );
    mPrecision->clear();
    mPrecision->setEnabled( false );
  }
  else if ( fieldType == "date"_L1 )
  {
    mWidth->clear();
    mWidth->setEnabled( false );
    mPrecision->clear();
    mPrecision->setEnabled( false );
  }
  else if ( fieldType == "time"_L1 )
  {
    mWidth->clear();
    mWidth->setEnabled( false );
    mPrecision->clear();
    mPrecision->setEnabled( false );
  }
  else if ( fieldType == "datetime"_L1 )
  {
    mWidth->clear();
    mWidth->setEnabled( false );
    mPrecision->clear();
    mPrecision->setEnabled( false );
  }
  else if ( fieldType == u"binary"_s
            || fieldType == u"stringlist"_s
            || fieldType == u"integerlist"_s
            || fieldType == u"doublelist"_s
            || fieldType == u"integer64list"_s
            || fieldType == u"map"_s
            || fieldType == "geometry"_L1 )
  {
    mWidth->clear();
    mWidth->setEnabled( false );
    mPrecision->clear();
    mPrecision->setEnabled( false );
  }
  else
  {
    QgsDebugError( u"unexpected index"_s );
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
  mAddAttributeButton->setDisabled( name.isEmpty() || !mAttributeView->findItems( name, Qt::MatchExactly ).isEmpty() );
}

void QgsNewMemoryLayerDialog::selectionChanged()
{
  mRemoveAttributeButton->setDisabled( mAttributeView->selectedItems().isEmpty() );
  mButtonUp->setDisabled( mAttributeView->selectedItems().isEmpty() );
  mButtonDown->setDisabled( mAttributeView->selectedItems().isEmpty() );
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
    QMetaType::Type fieldType = QMetaType::Type::UnknownType;
    QMetaType::Type fieldSubType = QMetaType::Type::UnknownType;
    if ( typeName == "string"_L1 )
      fieldType = QMetaType::Type::QString;
    else if ( typeName == "integer"_L1 )
      fieldType = QMetaType::Type::Int;
    else if ( typeName == "double"_L1 )
      fieldType = QMetaType::Type::Double;
    else if ( typeName == "bool"_L1 )
      fieldType = QMetaType::Type::Bool;
    else if ( typeName == "date"_L1 )
      fieldType = QMetaType::Type::QDate;
    else if ( typeName == "time"_L1 )
      fieldType = QMetaType::Type::QTime;
    else if ( typeName == "datetime"_L1 )
      fieldType = QMetaType::Type::QDateTime;
    else if ( typeName == "binary"_L1 )
      fieldType = QMetaType::Type::QByteArray;
    else if ( typeName == "stringlist"_L1 )
    {
      fieldType = QMetaType::Type::QStringList;
      fieldSubType = QMetaType::Type::QString;
    }
    else if ( typeName == "integerlist"_L1 )
    {
      fieldType = QMetaType::Type::QVariantList;
      fieldSubType = QMetaType::Type::Int;
    }
    else if ( typeName == "doublelist"_L1 )
    {
      fieldType = QMetaType::Type::QVariantList;
      fieldSubType = QMetaType::Type::Double;
    }
    else if ( typeName == "integer64list"_L1 )
    {
      fieldType = QMetaType::Type::QVariantList;
      fieldSubType = QMetaType::Type::LongLong;
    }
    else if ( typeName == "map"_L1 )
      fieldType = QMetaType::Type::QVariantMap;
    else if ( typeName == "geometry"_L1 )
      fieldType = QMetaType::Type::User;

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
      if ( QMessageBox::question( this, tr( "New Temporary Scratch Layer" ), tr( "The field “%1” has not been added to the fields list. Are you sure you want to proceed and discard this field?" ).arg( currentFieldName ), QMessageBox::Ok | QMessageBox::Cancel ) != QMessageBox::Ok )
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
  QgsHelp::openHelp( u"managing_data_source/create_layers.html#creating-a-new-temporary-scratch-layer"_s );
}

void QgsNewMemoryLayerDialog::moveFieldsUp()
{
  int currentRow = mAttributeView->currentIndex().row();
  if ( currentRow == 0 )
    return;

  mAttributeView->insertTopLevelItem( currentRow - 1, mAttributeView->takeTopLevelItem( currentRow ) );
  mAttributeView->setCurrentIndex( mAttributeView->model()->index( currentRow - 1, 0 ) );
}

void QgsNewMemoryLayerDialog::moveFieldsDown()
{
  int currentRow = mAttributeView->currentIndex().row();
  if ( currentRow == mAttributeView->topLevelItemCount() - 1 )
    return;

  mAttributeView->insertTopLevelItem( currentRow + 1, mAttributeView->takeTopLevelItem( currentRow ) );
  mAttributeView->setCurrentIndex( mAttributeView->model()->index( currentRow + 1, 0 ) );
}
