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

  connect( mFieldComboBox, &QgsFieldComboBox::fieldChanged, this, &QgsWmsDimensionDialog::fieldChanged );
  connect( mEndFieldComboBox, &QgsFieldComboBox::fieldChanged, this, &QgsWmsDimensionDialog::fieldChanged );
  connect( mNameComboBox, &QComboBox::editTextChanged, this, &QgsWmsDimensionDialog::nameChanged );
  connect( mDefaultValueComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsWmsDimensionDialog::defaultValueChanged );

  // Set available names
  for ( const QString &name : mPredefinedNames )
  {
    if ( !alreadyDefinedDimensions.contains( name.toLower() ) )
    {
      mNameComboBox->addItem( name );
    }
  }
  mDefaultValueComboBox->setCurrentIndex( 0 );
  mReferenceValueLabel->setEnabled( false );
  mReferenceValueComboBox->setEnabled( false );
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

  if ( mPredefinedNames.contains( name ) )
  {
    if ( mPredefinedNames.indexOf( name ) == 0 )  // Time
    {
      mFieldComboBox->setFilters( QgsFieldProxyModel::String | QgsFieldProxyModel::DateTime );
      mEndFieldComboBox->setFilters( QgsFieldProxyModel::String | QgsFieldProxyModel::DateTime );
      mUnitsLineEdit->setText( QStringLiteral( "ISO8601" ) );
      mUnitsLabel->setEnabled( false );
      mUnitsLineEdit->setEnabled( false );
      mUnitSymbolLabel->setEnabled( false );
      mUnitSymbolLineEdit->setEnabled( false );
    }
    else if ( mPredefinedNames.indexOf( name ) == 1 ) // Elevation
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
  fieldChanged( mFieldComboBox->currentField() );
}

void QgsWmsDimensionDialog::fieldChanged( const QString &fieldName )
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
  QList<QVariant> values = uniqueValues.toList();
  std::sort( values.begin(), values.end() );

  mReferenceValueComboBox->clear();
  mReferenceValueComboBox->addItem( QString(), QVariant() ); // Empty value
  const auto constValues = values;
  for ( const QVariant &v : constValues )
  {
    mReferenceValueComboBox->addItem( v.toString(), v );
  }
}

void QgsWmsDimensionDialog::defaultValueChanged( int index )
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
