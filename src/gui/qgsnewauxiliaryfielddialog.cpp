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

#include <QMessageBox>

QgsNewAuxiliaryFieldDialog::QgsNewAuxiliaryFieldDialog( const QgsPropertyDefinition &def, QgsVectorLayer *layer, bool nameOnly, QWidget *parent )
  : QDialog( parent )
  , mLayer( layer )
  , mNameOnly( nameOnly )
  , mPropertyDefinition( def )
{
  setupUi( this );

  mType->addItem( tr( "String" ) );
  mType->addItem( tr( "Real" ) );
  mType->addItem( tr( "Integer" ) );

  switch ( def.dataType() )
  {
    case QgsPropertyDefinition::DataTypeString:
      mType->setCurrentIndex( mType->findText( tr( "String" ) ) );
      break;
    case QgsPropertyDefinition::DataTypeNumeric:
      mType->setCurrentIndex( mType->findText( tr( "Real" ) ) );
      break;
    case QgsPropertyDefinition::DataTypeBoolean:
      mType->setCurrentIndex( mType->findText( tr( "Integer" ) ) );
      break;
  }

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
    if ( mType->currentText().compare( tr( "String" ) ) == 0 )
    {
      def.setDataType( QgsPropertyDefinition::DataTypeString );
    }
    else if ( mType->currentText().compare( tr( "Real" ) ) == 0 )
    {
      def.setDataType( QgsPropertyDefinition::DataTypeNumeric );
    }
    else
    {
      def.setDataType( QgsPropertyDefinition::DataTypeBoolean );
    }

    def.setOrigin( "user" );
    def.setName( "custom" );
  }

  QString fieldName = QgsAuxiliaryLayer::nameFromProperty( def, true );
  const int idx = mLayer->fields().lookupField( fieldName );
  if ( idx >= 0 )
  {
    const QString title = tr( "Invalid name" );
    const QString msg = tr( "Auxiliary field '%1' already exists" ).arg( fieldName );
    QMessageBox::critical( this, title, msg, QMessageBox::Ok );
  }
  else if ( def.comment().isEmpty() )
  {
    const QString title = tr( "Invalid name" );
    const QString msg = tr( "Name is a mandatory parameter" );
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
