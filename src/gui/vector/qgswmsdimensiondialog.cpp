/***************************************************************************
                              qgswmsdimensiondialog.cpp
                              ------------------
  begin                : August 20, 2019
  copyright            : (C) 2019 by René-Luc D'Hont
  email                : rldhont at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswmsdimensiondialog.h"
#include "qgsvectorlayer.h"
#include "qgsfieldcombobox.h"

#include <QStandardItemModel>
#include <QPushButton>

QgsWmsDimensionDialog::QgsWmsDimensionDialog( QgsVectorLayer *layer, QStringList alreadyDefinedDimensions, QWidget *parent, Qt::WindowFlags f )
  : QDialog( parent, f )
  , mLayer( layer )
{
  setupUi( this );

  if ( !mLayer )
  {
    return;
  }

  // Set field combobox
  mFieldComboBox->setLayer( mLayer );
  mEndFieldComboBox->setLayer( mLayer );
  mEndFieldComboBox->setAllowEmptyFieldName( true );

  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsWmsDimensionDialog::accept );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsWmsDimensionDialog::reject );
  connect( mFieldComboBox, &QgsFieldComboBox::fieldChanged, this, &QgsWmsDimensionDialog::fieldChanged );
  connect( mEndFieldComboBox, &QgsFieldComboBox::fieldChanged, this, &QgsWmsDimensionDialog::fieldChanged );
  connect( mNameComboBox, &QComboBox::editTextChanged, this, &QgsWmsDimensionDialog::nameChanged );
  connect( mDefaultDisplayComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, &QgsWmsDimensionDialog::defaultDisplayChanged );

  // Set available names
  const QMetaEnum pnMetaEnum( QMetaEnum::fromType<QgsVectorLayerServerProperties::PredefinedWmsDimensionName>() );
  for ( int i = 0; i < pnMetaEnum.keyCount(); i++ )
  {
    QString name( pnMetaEnum.key( i ) );
    if ( !alreadyDefinedDimensions.contains( name.toLower() ) )
    {
      mNameComboBox->addItem( QStringLiteral( "%1%2" ).arg( !name.isEmpty() ? name.at( 0 ) : QString(), name.mid( 1 ).toLower() ), QVariant( pnMetaEnum.value( i ) ) );
    }
  }

  // Set default display combobox
  mDefaultDisplayComboBox->clear();
  QMap<int, QString> defaultDisplayLabels = QgsVectorLayerServerProperties::wmsDimensionDefaultDisplayLabels();
  for ( const int &k : defaultDisplayLabels.keys() )
  {
    mDefaultDisplayComboBox->addItem( defaultDisplayLabels[k], QVariant( k ) );
  }
  // Set default display to All values
  mDefaultDisplayComboBox->setCurrentIndex( mDefaultDisplayComboBox->findData( QVariant( QgsVectorLayerServerProperties::WmsDimensionInfo::AllValues ) ) );

  mReferenceValueLabel->setEnabled( false );
  mReferenceValueComboBox->setEnabled( false );
}

void QgsWmsDimensionDialog::setInfo( const QgsVectorLayerServerProperties::WmsDimensionInfo &info )
{
  const QMetaEnum pnMetaEnum( QMetaEnum::fromType<QgsVectorLayerServerProperties::PredefinedWmsDimensionName>() );
  int predefinedNameValue = pnMetaEnum.keyToValue( info.name.toUpper().toStdString().c_str() );
  if ( predefinedNameValue == -1 )
  {
    mNameComboBox->setEditText( info.name );
  }
  else
  {
    mNameComboBox->setCurrentIndex( mNameComboBox->findData( QVariant( predefinedNameValue ) ) );
  }
  mNameComboBox->setEnabled( false );

  mFieldComboBox->setField( info.fieldName );
  mEndFieldComboBox->setField( info.endFieldName );

  mUnitsLineEdit->setText( info.units );
  mUnitSymbolLineEdit->setText( info.unitSymbol );

  mDefaultDisplayComboBox->setCurrentIndex( mDefaultDisplayComboBox->findData( QVariant( info.defaultDisplayType ) ) );
  if ( info.defaultDisplayType == QgsVectorLayerServerProperties::WmsDimensionInfo::ReferenceValue )
  {
    int referenceValueIndex = mReferenceValueComboBox->findData( info.referenceValue );
    if ( referenceValueIndex == -1 )
    {
      mReferenceValueComboBox->setEditText( info.referenceValue.toString() );
    }
    else
    {
      mReferenceValueComboBox->setCurrentIndex( referenceValueIndex );
    }
  }
  else
  {
    mReferenceValueComboBox->setCurrentIndex( 0 );
  }
}

QgsVectorLayerServerProperties::WmsDimensionInfo QgsWmsDimensionDialog::info() const
{
  // Is the name a predefined value?
  QString name = mNameComboBox->currentText();
  if ( mNameComboBox->findText( name ) != -1 )
  {
    name = name.toLower();
  }

  // Gets the reference value
  QString refText = mReferenceValueComboBox->currentText();
  QVariant refValue;
  if ( mReferenceValueComboBox->findText( refText ) != -1 )
  {
    refValue = mReferenceValueComboBox->currentData();
  }
  return QgsVectorLayerServerProperties::WmsDimensionInfo( name, mFieldComboBox->currentField(),
         mEndFieldComboBox->currentField(),
         mUnitsLineEdit->text(), mUnitSymbolLineEdit->text(),
         mDefaultDisplayComboBox->currentData().toInt(), refValue );
}

void QgsWmsDimensionDialog::nameChanged( const QString &name )
{
  // reset input
  mUnitsLabel->setEnabled( true );
  mUnitsLineEdit->setEnabled( true );
  mUnitsLineEdit->clear();
  mUnitSymbolLabel->setEnabled( true );
  mUnitSymbolLineEdit->setEnabled( true );
  mUnitSymbolLineEdit->clear();

  // Is the name a predefined value?
  if ( mNameComboBox->findText( name ) != -1 )
  {
    int data = mNameComboBox->currentData().toInt();
    if ( data == QgsVectorLayerServerProperties::TIME )
    {
      mFieldComboBox->setFilters( QgsFieldProxyModel::String | QgsFieldProxyModel::DateTime );
      mEndFieldComboBox->setFilters( QgsFieldProxyModel::String | QgsFieldProxyModel::DateTime );
      mUnitsLineEdit->setText( QStringLiteral( "ISO8601" ) );
      mUnitsLabel->setEnabled( false );
      mUnitsLineEdit->setEnabled( false );
      mUnitSymbolLabel->setEnabled( false );
      mUnitSymbolLineEdit->setEnabled( false );
    }
    if ( data == QgsVectorLayerServerProperties::DATE )
    {
      mFieldComboBox->setFilters( QgsFieldProxyModel::String | QgsFieldProxyModel::Date );
      mEndFieldComboBox->setFilters( QgsFieldProxyModel::String | QgsFieldProxyModel::Date );
      mUnitsLineEdit->setText( QStringLiteral( "ISO8601" ) );
      mUnitsLabel->setEnabled( false );
      mUnitsLineEdit->setEnabled( false );
      mUnitSymbolLabel->setEnabled( false );
      mUnitSymbolLineEdit->setEnabled( false );
    }
    else if ( data == QgsVectorLayerServerProperties::ELEVATION )
    {
      mFieldComboBox->setFilters( QgsFieldProxyModel::Numeric );
      mEndFieldComboBox->setFilters( QgsFieldProxyModel::Numeric );
    }
  }
  else
  {
    mFieldComboBox->setFilters( QgsFieldProxyModel::AllTypes );
    mEndFieldComboBox->setFilters( QgsFieldProxyModel::AllTypes );
  }

  fieldChanged();
}

void QgsWmsDimensionDialog::fieldChanged()
{
  QString currentFieldName = mFieldComboBox->currentField();
  int currentFieldIndexOf = mLayer->fields().indexOf( currentFieldName );
  QSet<QVariant> uniqueValues = mLayer->uniqueValues( currentFieldIndexOf );

  QString currentEndFieldName = mEndFieldComboBox->currentField();
  if ( !currentEndFieldName.isEmpty() )
  {
    int currentEndFieldIndexOf = mLayer->fields().indexOf( currentEndFieldName );
    uniqueValues.unite( mLayer->uniqueValues( currentEndFieldIndexOf ) );
  }
  QList<QVariant> values = qgis::setToList( uniqueValues );
  std::sort( values.begin(), values.end() );

  mReferenceValueComboBox->clear();
  mReferenceValueComboBox->addItem( QString(), QVariant() ); // Empty value
  const auto constValues = values;
  for ( const QVariant &v : constValues )
  {
    mReferenceValueComboBox->addItem( v.toString(), v );
  }
}

void QgsWmsDimensionDialog::defaultDisplayChanged( int index )
{
  if ( index == 3 )
  {
    mReferenceValueLabel->setEnabled( true );
    mReferenceValueComboBox->setEnabled( true );
  }
  else
  {
    mReferenceValueLabel->setEnabled( false );
    mReferenceValueComboBox->setEnabled( false );
  }
}
