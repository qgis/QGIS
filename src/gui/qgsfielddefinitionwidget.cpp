/***************************************************************************
   qgsfielddefinitionwidget.cpp
    --------------------------------------
    begin                : September 2020
    copyright            : (C) 2020 by Ivan Ivanov
    email                : ivan@opengis.ch
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsfielddefinitionwidget.h"
#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"

#include <QMap>

QgsFieldDefinitionWidget::QgsFieldDefinitionWidget( AdvancedFields advancedFields, QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );
  connect( mTypeCmb, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsFieldDefinitionWidget::mTypeCmb_currentIndexChanged );
  connect( mLengthSpinBox, qgis::overload<int>::of( &QSpinBox::valueChanged ), this, &QgsFieldDefinitionWidget::mLengthSpinBox_valueChanged );
  connect( mPrecisionSpinBox, qgis::overload<int>::of( &QSpinBox::valueChanged ), this, &QgsFieldDefinitionWidget::changed );
  connect( mFieldNameLineEdit, &QLineEdit::textChanged, this, &QgsFieldDefinitionWidget::changed );
  connect( mFieldNameLineEdit, &QLineEdit::returnPressed, this, &QgsFieldDefinitionWidget::returnPressed );
  connect( mAliasLineEdit, &QLineEdit::textChanged, this, &QgsFieldDefinitionWidget::changed );
  connect( mAliasLineEdit, &QLineEdit::returnPressed, this, &QgsFieldDefinitionWidget::returnPressed );
  connect( mCommentLineEdit, &QLineEdit::textChanged, this, &QgsFieldDefinitionWidget::changed );
  connect( mCommentLineEdit, &QLineEdit::returnPressed, this, &QgsFieldDefinitionWidget::returnPressed );

  if ( advancedFields == AdvancedFields() )
  {
    mAdvancedGroup->setVisible( false );
  }
  else
  {
    mAdvancedGroup->setVisible( true );

    mAliasLabel->setEnabled( advancedFields & AdvancedField::Alias );
    mAliasLineEdit->setEnabled( advancedFields & AdvancedField::Alias );
    mCommentLabel->setEnabled( advancedFields & AdvancedField::Comment );
    mCommentLineEdit->setEnabled( advancedFields & AdvancedField::Comment );
    mIsNullableCheckBox->setEnabled( advancedFields & AdvancedField::IsNullable );
  }

  mFieldNameLineEdit->setValidator( mRegExpValidator );
}

QString QgsFieldDefinitionWidget::name() const
{
  return mFieldNameLineEdit->text();
}

void QgsFieldDefinitionWidget::setName( const QString &name )
{
  return mFieldNameLineEdit->setText( name );
}

QString QgsFieldDefinitionWidget::comment() const
{
  return mCommentLineEdit->text();
}

void QgsFieldDefinitionWidget::setComment( const QString &comment )
{
  return mCommentLineEdit->setText( comment );
}

void QgsFieldDefinitionWidget::setIsNullable( bool isNullable )
{
  mIsNullableCheckBox->setChecked( isNullable );
}

bool QgsFieldDefinitionWidget::isNullable()
{
  return mIsNullableCheckBox->isChecked();
}

QString QgsFieldDefinitionWidget::alias() const
{
  return mAliasLineEdit->text();
}

void QgsFieldDefinitionWidget::setAlias( const QString &alias )
{
  return mAliasLineEdit->setText( alias );
}

int QgsFieldDefinitionWidget::length() const
{
  return mLengthSpinBox->value();
}

void QgsFieldDefinitionWidget::setLength( int length )
{
  return mLengthSpinBox->setValue( length );
}

int QgsFieldDefinitionWidget::precision() const
{
  return mPrecisionSpinBox->value();
}

void QgsFieldDefinitionWidget::setPrecision( int precision )
{
  return mPrecisionSpinBox->setValue( precision );
}

QString QgsFieldDefinitionWidget::type() const
{
  QVariant data = mTypeCmb->currentData();
  return data.toMap().value( "type" ).toString();
}

bool QgsFieldDefinitionWidget::setType( const QString &typeName )
{
  if ( ! hasType( typeName ) )
    return false;

  mTypeCmb->setCurrentIndex( typeIndex( typeName ) );

  return true;
}

bool QgsFieldDefinitionWidget::addType( const QString &typeName, const QString &typeDisplay, const QIcon &icon, int length, int precision )
{
  return insertType( -1, typeName, typeDisplay, icon, length, precision );
}

bool QgsFieldDefinitionWidget::insertType( const int position, const QString &typeName, const QString &typeDisplay, const QIcon &icon, int length, int precision )
{
  if ( typeName.isEmpty() || typeDisplay.isEmpty() )
    return false;

  if ( hasType( typeName ) )
    return false;

  if ( precision > length )
    return false;

  QVariantMap data;
  data.insert( QStringLiteral( "type" ), QVariant( typeName ) );
  data.insert( QStringLiteral( "length" ), QVariant( length ) );
  data.insert( QStringLiteral( "precision" ), QVariant( precision ) );

  if ( position >= 0 )
    mTypeCmb->insertItem( position, icon, typeDisplay, QVariant( data ) );
  else
    mTypeCmb->addItem( icon, typeDisplay, QVariant( data ) );

  if ( mTypeCmb->count() == 1 )
    mTypeCmb->setCurrentIndex( 0 );

  return true;
}

bool QgsFieldDefinitionWidget::hasType( const QString &typeName ) const
{
  return typeIndex( typeName ) >= 0;
}

int QgsFieldDefinitionWidget::typeIndex( const QString &typeName ) const
{
  for ( int i = 0; i < mTypeCmb->count(); i++ )
  {
    QVariant data = mTypeCmb->itemData( i );

    if ( data.toMap().value( QStringLiteral( "type" ) ) == typeName )
      return i;
  }

  return -1;
}

bool QgsFieldDefinitionWidget::removeType( const QString &typeName )
{
  if ( hasType( typeName ) )
    return false;

  mTypeCmb->removeItem( typeIndex( typeName ) );

  return false;
}

QStringList QgsFieldDefinitionWidget::types() const
{
  QStringList result;

  for ( int i = 0; i < mTypeCmb->count(); i++ )
    result << mTypeCmb->itemData( i ).toMap().value( QStringLiteral( "type" ) ).toString();

  return result;
}

void QgsFieldDefinitionWidget::mLengthSpinBox_valueChanged( const int value )
{
  mPrecisionSpinBox->setMaximum( value );

  emit changed();
}

void QgsFieldDefinitionWidget::mTypeCmb_currentIndexChanged( const int index )
{
  Q_UNUSED( index );

  QVariant data = mTypeCmb->currentData();
  int length = data.toMap().value( "length" ).toInt();
  int precision = data.toMap().value( "precision" ).toInt();

  mLengthSpinBox->setEnabled( false );
  mLengthLabel->setEnabled( false );
  mPrecisionSpinBox->setEnabled( false );
  mPrecisionLabel->setEnabled( false );

  if ( length > -1 )
  {
    mLengthSpinBox->setValue( length );
    mLengthSpinBox->setEnabled( true );
    mLengthLabel->setEnabled( true );
  }

  if ( precision > -1 )
  {
    mPrecisionSpinBox->setValue( precision );
    mPrecisionSpinBox->setEnabled( true );
    mPrecisionLabel->setEnabled( true );
  }

  emit changed();
}

bool QgsFieldDefinitionWidget::isValidForm() const
{
  return mFieldNameLineEdit->hasAcceptableInput();
}

QRegExp QgsFieldDefinitionWidget::nameRegExp() const
{
  return mRegExpValidator->regExp();
}

void QgsFieldDefinitionWidget::setNameRegExp( QRegExp &nameRegExp )
{
  mRegExpValidator->setRegExp( nameRegExp );
}

QgsField *QgsFieldDefinitionWidget::asField() const
{
  if ( shouldUseExistingField() )
  {
    QgsField field = mExistingLayerFieldCmb->layer()->fields().field( mExistingLayerFieldCmb->currentField() );
    return new QgsField( field );
  }

  if ( ! isValidForm() )
    return nullptr;

  QgsField *field = new QgsField( name(), QVariant::String, type(), length(), precision(), comment() );
  field->setAlias( alias() );

  return field;
}

void QgsFieldDefinitionWidget::setLayer( QgsVectorLayer *vl )
{
  if ( ! vl )
    mChooseFieldToolBox->setCurrentIndex( 0 );

  mChooseFieldToolBox->setItemEnabled( 1, !!vl );
  mExistingLayerFieldCmb->setLayer( vl );
}

QgsVectorLayer *QgsFieldDefinitionWidget::layer() const
{
  return mExistingLayerFieldCmb->layer();
}

void QgsFieldDefinitionWidget::setLayerField( const QString &fieldName )
{
  mExistingLayerFieldCmb->setField( fieldName );
}

QString QgsFieldDefinitionWidget::layerField() const
{
  return mExistingLayerFieldCmb->currentField();
}

bool QgsFieldDefinitionWidget::shouldUseExistingField() const
{
  return mChooseFieldToolBox->currentWidget() == mExistingFieldPage;
}

void QgsFieldDefinitionWidget::setShouldUseExistingField( bool shouldUseExistingField )
{
  if ( mExistingLayerFieldCmb->layer() && shouldUseExistingField )
    mChooseFieldToolBox->setCurrentWidget( mExistingFieldPage );
}
