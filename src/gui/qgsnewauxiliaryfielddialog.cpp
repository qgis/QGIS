/***************************************************************************
                         qgsnewauxiliaryfielddialog.cpp  -  description
                             -------------------
    begin                : Sept 05, 2017
    copyright            : (C) 2017 by Paul Blottiere
    email                : paul.blottiere@oslandia.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsnewauxiliaryfielddialog.h"
#include "qgsauxiliarystorage.h"
#include "qgsgui.h"
#include "qgsapplication.h"
#include "qgsvariantutils.h"

#include <QMessageBox>

QgsNewAuxiliaryFieldDialog::QgsNewAuxiliaryFieldDialog( const QgsPropertyDefinition &def, QgsVectorLayer *layer, bool nameOnly, QWidget *parent )
  : QDialog( parent )
  , mLayer( layer )
  , mNameOnly( nameOnly )
  , mPropertyDefinition( def )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  mType->addItem( QgsFields::iconForFieldType( QVariant::String ), QgsVariantUtils::typeToDisplayString( QVariant::String ), QgsPropertyDefinition::DataTypeString );
  mType->addItem( QgsFields::iconForFieldType( QVariant::Double ), QgsVariantUtils::typeToDisplayString( QVariant::Double ), QgsPropertyDefinition::DataTypeNumeric );
  mType->addItem( QgsFields::iconForFieldType( QVariant::Int ), tr( "Integer" ), QgsPropertyDefinition::DataTypeBoolean );

  mType->setCurrentIndex( mType->findData( def.dataType() ) );

  if ( mNameOnly )
    mType->setEnabled( false );
  else
    mType->setEnabled( true );
}

void QgsNewAuxiliaryFieldDialog::accept()
{
  QgsPropertyDefinition def = mPropertyDefinition;
  def.setComment( mName->text() );

  if ( !mNameOnly )
  {
    def.setDataType( static_cast< QgsPropertyDefinition::DataType >( mType->currentData().toInt() ) );

    def.setOrigin( "user" );
    def.setName( "custom" );
  }

  const QString fieldName = QgsAuxiliaryLayer::nameFromProperty( def, true );
  const int idx = mLayer->fields().lookupField( fieldName );
  if ( idx >= 0 )
  {
    const QString title = tr( "New Auxiliary Field" );
    const QString msg = tr( "Invalid name. Auxiliary field '%1' already exists." ).arg( fieldName );
    QMessageBox::critical( this, title, msg, QMessageBox::Ok );
  }
  else if ( def.comment().isEmpty() )
  {
    const QString title = tr( "New Auxiliary Field" );
    const QString msg = tr( "Name is a mandatory parameter." );
    QMessageBox::critical( this, title, msg, QMessageBox::Ok );
  }
  else
  {
    if ( mLayer->auxiliaryLayer()->addAuxiliaryField( def ) )
      mPropertyDefinition = def;
    QDialog::accept();
  }
}

QgsPropertyDefinition QgsNewAuxiliaryFieldDialog::propertyDefinition() const
{
  return mPropertyDefinition;
}
