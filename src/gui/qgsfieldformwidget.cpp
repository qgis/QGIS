/***************************************************************************
   qgsfieldformwidget.cpp
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

#include "qgis.h"
#include "qgsfieldformwidget.h"
#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"

#include <QMap>

QgsFieldFormWidget::QgsFieldFormWidget( AdvancedFields advancedFields, QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );
  connect( mTypeCmb, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsFieldFormWidget::mTypeCmb_currentIndexChanged );
  connect( mLengthSpinBox, QOverload<int>::of( &QSpinBox::valueChanged ), this, &QgsFieldFormWidget::mLengthSpinBox_valueChanged );
  connect( mPrecisionSpinBox, QOverload<int>::of( &QSpinBox::valueChanged ), this, &QgsFieldFormWidget::changed );
  connect( mFieldNameLineEdit, &QLineEdit::textChanged, this, &QgsFieldFormWidget::changed );
  connect( mFieldNameLineEdit, &QLineEdit::returnPressed, this, &QgsFieldFormWidget::returnPressed );
  connect( mAliasLineEdit, &QLineEdit::textChanged, this, &QgsFieldFormWidget::changed );
  connect( mAliasLineEdit, &QLineEdit::returnPressed, this, &QgsFieldFormWidget::returnPressed );
  connect( mCommentLineEdit, &QLineEdit::textChanged, this, &QgsFieldFormWidget::changed );
  connect( mCommentLineEdit, &QLineEdit::returnPressed, this, &QgsFieldFormWidget::returnPressed );

  if ( advancedFields == static_cast<AdvancedFields>( AdvancedField::Neither ) )
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

QString QgsFieldFormWidget::name() const
{
  return mFieldNameLineEdit->text();
}

void QgsFieldFormWidget::setName( QString name )
{
  return mFieldNameLineEdit->setText( name );
}

QString QgsFieldFormWidget::comment() const
{
  return mCommentLineEdit->text();
}

void QgsFieldFormWidget::setComment( QString comment )
{
  return mCommentLineEdit->setText( comment );
}

void QgsFieldFormWidget::setIsNullable( bool isNullable )
{
  mIsNullableCheckBox->setChecked( isNullable );
}

bool QgsFieldFormWidget::isNullable()
{
  return mIsNullableCheckBox->isChecked();
}

QString QgsFieldFormWidget::alias() const
{
  return mAliasLineEdit->text();
}

void QgsFieldFormWidget::setAlias( QString alias )
{
  return mAliasLineEdit->setText( alias );
}

int QgsFieldFormWidget::length() const
{
  return mLengthSpinBox->value();
}

void QgsFieldFormWidget::setLength( int length )
{
  return mLengthSpinBox->setValue( length );
}

int QgsFieldFormWidget::precision() const
{
  return mPrecisionSpinBox->value();
}

void QgsFieldFormWidget::setPrecision( int precision )
{
  return mPrecisionSpinBox->setValue( precision );
}

QString QgsFieldFormWidget::type() const
{
  QVariant data = mTypeCmb->currentData();
  return data.toMap().value( "type" ).toString();
}

bool QgsFieldFormWidget::setType( const QString typeName )
{
  if ( ! hasType( typeName ) )
    return false;

  mTypeCmb->setCurrentIndex( typeIndex( typeName ) );

  return true;
}

bool QgsFieldFormWidget::addType( const QString typeName, const QString typeDisplay, const QIcon icon, int length, int precision )
{
  return insertType( -1, typeName, typeDisplay, icon, length, precision );
}

bool QgsFieldFormWidget::insertType( const int position, const QString typeName, const QString typeDisplay, const QIcon icon, int length, int precision )
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

bool QgsFieldFormWidget::hasType( const QString typeName )
{
  return typeIndex( typeName ) >= 0;
}

int QgsFieldFormWidget::typeIndex( const QString typeName )
{
  for ( int i = 0; i < mTypeCmb->count(); i++ )
  {
    QVariant data = mTypeCmb->itemData( i );

    if ( data.toMap().value( QStringLiteral( "type" ) ) == typeName )
      return i;
  }

  return -1;
}

bool QgsFieldFormWidget::removeType( const QString typeName )
{
  if ( hasType( typeName ) )
    return false;

  mTypeCmb->removeItem( typeIndex( typeName ) );

  return false;
}

QStringList QgsFieldFormWidget::types() const
{
  QStringList result;

  for ( int i = 0; i < mTypeCmb->count(); i++ )
    result << mTypeCmb->itemData( i ).toMap().value( QStringLiteral( "type" ) ).toString();

  return result;
}

void QgsFieldFormWidget::mLengthSpinBox_valueChanged( const int value )
{
  mPrecisionSpinBox->setMaximum( value );

  emit changed();
}

void QgsFieldFormWidget::mTypeCmb_currentIndexChanged( const int index )
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

bool QgsFieldFormWidget::isValidForm() const
{
  return mFieldNameLineEdit->hasAcceptableInput();
}

QRegExp QgsFieldFormWidget::nameRegExp()
{
  return mRegExpValidator->regExp();
}

void QgsFieldFormWidget::setNameRegExp( QRegExp nameRegExp )
{
  mRegExpValidator->setRegExp( nameRegExp );
}

QgsField *QgsFieldFormWidget::asField() const
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

void QgsFieldFormWidget::setLayer( QgsVectorLayer *vl )
{
  if ( ! vl )
    mChooseFieldToolBox->setCurrentIndex( 0 );

  mChooseFieldToolBox->setItemEnabled( 1, !!vl );
  mExistingLayerFieldCmb->setLayer( vl );
}

QgsVectorLayer *QgsFieldFormWidget::layer() const
{
  return mExistingLayerFieldCmb->layer();
}

void QgsFieldFormWidget::setLayerField( const QString fieldName )
{
  mExistingLayerFieldCmb->setField( fieldName );
}

QString QgsFieldFormWidget::layerField() const
{
  return mExistingLayerFieldCmb->currentField();
}

bool QgsFieldFormWidget::shouldUseExistingField() const
{
  return mChooseFieldToolBox->currentWidget() == mExistingFieldPage;
}

void QgsFieldFormWidget::setShouldUseExistingField( bool shouldUseExistingField )
{
  if ( mExistingLayerFieldCmb->layer() && shouldUseExistingField )
    mChooseFieldToolBox->setCurrentWidget( mExistingFieldPage );
}
