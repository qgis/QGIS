/***************************************************************************
  qgsrasterattributetableaddcolumndialog.cpp - QgsRasterAttributeTableAddColumnDialog

 ---------------------
 begin                : 10.10.2022
 copyright            : (C) 2022 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsrasterattributetableaddcolumndialog.h"
#include "qgsrasterattributetable.h"
#include "qgsgui.h"

#include <QPushButton>


QgsRasterAttributeTableAddColumnDialog::QgsRasterAttributeTableAddColumnDialog( QgsRasterAttributeTable *attributeTable, QWidget *parent )
  : QDialog( parent )
  , mAttributeTable( attributeTable )
{
  // Precondition
  Q_ASSERT( mAttributeTable );

  setupUi( this );

  connect( mName, &QLineEdit::textChanged, this, [ = ]( const QString & ) { updateDialog(); } );
  connect( mStandardColumn, &QRadioButton::toggled, this, [ = ]( bool ) { updateDialog(); } );
  connect( mColor, &QRadioButton::toggled, this, [ = ]( bool ) { updateDialog(); } );
  connect( mUsage, qOverload<int>( &QComboBox::currentIndexChanged ), this, [ = ]( int ) { updateDialog(); } );

  mDataType->addItem( QgsFields::iconForFieldType( QVariant::Type::String ), tr( "String" ), static_cast<int>( QVariant::Type::String ) );
  mDataType->addItem( QgsFields::iconForFieldType( QVariant::Type::Int ), tr( "Integer" ), static_cast<int>( QVariant::Type::Int ) );
  mDataType->addItem( QgsFields::iconForFieldType( QVariant::Type::LongLong ), tr( "Long Integer" ), static_cast<int>( QVariant::Type::LongLong ) );
  mDataType->addItem( QgsFields::iconForFieldType( QVariant::Type::Double ), tr( "Double" ), static_cast<int>( QVariant::Type::Double ) );
  mStandardColumn->setChecked( true );

  updateDialog();

  QgsGui::enableAutoGeometryRestore( this );
}

int QgsRasterAttributeTableAddColumnDialog::position() const
{
  if ( mAfter->isChecked() )
  {
    return mColumn->currentIndex() + 1;
  }
  else
  {
    return mColumn->currentIndex();
  }
}

bool QgsRasterAttributeTableAddColumnDialog::isColor() const
{
  return mColor->isChecked();
}

bool QgsRasterAttributeTableAddColumnDialog::isRamp() const
{
  return mRamp->isChecked();
}

QString QgsRasterAttributeTableAddColumnDialog::name() const
{
  return mName->text();
}

Qgis::RasterAttributeTableFieldUsage QgsRasterAttributeTableAddColumnDialog::usage() const
{
  return static_cast<Qgis::RasterAttributeTableFieldUsage>( mUsage->currentData( ).toInt( ) );
}

QVariant::Type QgsRasterAttributeTableAddColumnDialog::type() const
{
  return static_cast<QVariant::Type>( mDataType->currentData( ).toInt( ) );
}

void QgsRasterAttributeTableAddColumnDialog::updateDialog()
{
  mDefinition->setEnabled( mStandardColumn->isChecked() );
  mError->hide();
  mError->clear();

  QList<Qgis::RasterAttributeTableFieldUsage> usages;
  usages = mAttributeTable->usages();
  const bool hasMinMax { usages.contains( Qgis::RasterAttributeTableFieldUsage::MinMax ) };
  const bool hasMinAndMax { usages.contains( Qgis::RasterAttributeTableFieldUsage::Min ) &&usages.contains( Qgis::RasterAttributeTableFieldUsage::Max ) };
  const bool canAddMinMax { !hasMinMax &&mAttributeTable->type() == Qgis::RasterAttributeTableType::Thematic };
  const bool canAddMinAndMax { !hasMinAndMax &&mAttributeTable->type() == Qgis::RasterAttributeTableType::Athematic };

  if ( mAttributeTable->hasColor() || mAttributeTable->hasRamp() )
  {
    mColor->setChecked( false );
    mColor->setEnabled( false );
    mRamp->setChecked( false );
    mRamp->setEnabled( false );
    mStandardColumn->setChecked( true );
  }
  else if ( mAttributeTable->type() == Qgis::RasterAttributeTableType::Thematic )
  {
    mColor->setEnabled( true );
    mRamp->setChecked( false );
    mRamp->setEnabled( false );
  }
  else
  {
    mColor->setEnabled( true );
    mRamp->setEnabled( true );
  }

  bool isValid { true };
  if ( mStandardColumn->isChecked() )
  {
    const QString upperName { mName->text().trimmed().toUpper() };
    if ( upperName.isEmpty() )
    {
      mError->setText( tr( "A field name cannot be blank." ) );
      isValid = false;
    }

    const QList<QgsRasterAttributeTable::Field> fields { mAttributeTable->fields() };
    for ( const QgsRasterAttributeTable::Field &f : std::as_const( fields ) )
    {
      if ( f.name.toUpper() == upperName )
      {
        mError->setText( tr( "A field with this name already exists." ) );
        isValid = false;
        break;
      }
    }
  }

  const QHash<Qgis::RasterAttributeTableFieldUsage, QgsRasterAttributeTable::UsageInformation> usageInfo { QgsRasterAttributeTable::usageInformation() };

  const int currentUsageIndex { mUsage->currentIndex()};
  const QSignalBlocker usageBlocker( mUsage );
  mUsage->clear();


  for ( auto it = usageInfo.cbegin(); it != usageInfo.cend(); ++it )
  {
    // We don't want duplicated columns or columns that are not suitable for color or ramps
    // if they are already there, it could be a single if condition but it is more readable
    // this way
    if ( ! it.value().unique || ! usages.contains( it.key() ) )
    {
      if ( ( it.key() == Qgis::RasterAttributeTableFieldUsage::MinMax && ! canAddMinMax ) ||
           ( it.key() == Qgis::RasterAttributeTableFieldUsage::Min && ! canAddMinAndMax ) ||
           ( it.key() == Qgis::RasterAttributeTableFieldUsage::Max && ! canAddMinAndMax ) ||
           ( it.value().isColor ) ||
           ( it.value().isRamp ) )
      {
        continue;
      }
      mUsage->addItem( QgsRasterAttributeTable::usageName( it.key() ), static_cast<int>( it.key() ) );
    }
  }
  mUsage->setCurrentIndex( std::clamp( currentUsageIndex, 0, static_cast<int>( mUsage->count() - 1 ) ) );

  const QList<QgsRasterAttributeTable::Field> fields { mAttributeTable->fields() };

  int currentIndex { mColumn->currentIndex() };
  if ( mColumn->currentIndex() < 0 )
  {
    currentIndex = fields.count( ) - 1;
  }

  const QSignalBlocker columnBlocker( mColumn );
  mColumn->clear();
  for ( const QgsRasterAttributeTable::Field &field : std::as_const( fields ) )
  {
    mColumn->addItem( field.name );
  }
  mColumn->setCurrentIndex( std::clamp( currentIndex, 0, static_cast<int>( fields.count( ) - 1 ) ) );

  if ( ! isValid )
  {
    mError->show();
  }

  mButtonBox->button( QDialogButtonBox::StandardButton::Ok )->setEnabled( isValid );

}
