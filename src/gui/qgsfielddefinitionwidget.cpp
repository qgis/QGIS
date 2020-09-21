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

QVariant::Type QgsFieldDefinitionWidget::type() const
{
  QVariant data = mTypeCmb->currentData();
  return static_cast<QVariant::Type>( data.toMap().value( "type" ).toInt() );
}

bool QgsFieldDefinitionWidget::setType( const QVariant::Type &typeName )
{
  if ( ! hasType( typeName ) )
    return false;

  mTypeCmb->setCurrentIndex( typeIndex( typeName ) );

  return true;
}

bool QgsFieldDefinitionWidget::addTypes( const QList<QVariant::Type> &types )
{
  for ( const QVariant::Type &type : types )
  {
    if ( ! insertType( -1, type ) )
    {
      return false;
    }
  }

  return true;
}

bool QgsFieldDefinitionWidget::insertType( const int position, const QVariant::Type &type )
{
  int length = -1;
  int precision = -1;
  QString typeName;
  QIcon icon;

  switch ( type )
  {
    case QVariant::Bool:
      typeName = QStringLiteral( "boolean" );
      icon = QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldBool.svg" ) );
      break;
    case QVariant::Int:
      typeName = QStringLiteral( "int" );
      icon = QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldInteger.svg" ) );
      break;
    case QVariant::LongLong:
      typeName = QStringLiteral( "long long" );
      icon = QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldInteger.svg" ) );
      break;
    case QVariant::Double:
      typeName = QStringLiteral( "double" );
      icon = QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldFloat.svg" ) );
      break;
    case QVariant::Char:
    case QVariant::String:
      typeName = QStringLiteral( "string" );
      icon = QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldText.svg" ) );
      length = 255;
      break;
    case QVariant::BitArray:
    case QVariant::ByteArray:
      typeName = QStringLiteral( "binary" );
      icon = QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldBinary.svg" ) );
      length = 255;
      break;
    case QVariant::DateTime:
      typeName = QStringLiteral( "datetime" );
      icon = QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldDateTime.svg" ) );
      break;
    case QVariant::Time:
      typeName = QStringLiteral( "time" );
      icon = QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldTime.svg" ) );
      break;
    case QVariant::Date:
      typeName = QStringLiteral( "date" );
      icon = QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldDate.svg" ) );
      break;
    case QVariant::Uuid:
      typeName = QStringLiteral( "uuid" );
      icon = QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldText.svg" ) );
      break;
    default:
      return false;
  }

  if ( typeName.isEmpty() || typeName.isEmpty() )
    return false;

  if ( hasType( type ) )
    return false;

  if ( precision > length )
    return false;

  QVariantMap data;
  data.insert( QStringLiteral( "type" ), QVariant( type ) );
  data.insert( QStringLiteral( "typeName" ), QVariant( typeName ) );
  data.insert( QStringLiteral( "length" ), QVariant( length ) );
  data.insert( QStringLiteral( "precision" ), QVariant( precision ) );

  if ( position >= 0 )
    mTypeCmb->insertItem( position, icon, typeName, QVariant( data ) );
  else
    mTypeCmb->addItem( icon, typeName, QVariant( data ) );

  if ( mTypeCmb->count() == 1 )
    mTypeCmb->setCurrentIndex( 0 );

  return true;
}

bool QgsFieldDefinitionWidget::hasType( const QVariant::Type &type ) const
{
  return typeIndex( type ) >= 0;
}

int QgsFieldDefinitionWidget::typeIndex( const QVariant::Type &type ) const
{
  for ( int i = 0; i < mTypeCmb->count(); i++ )
  {
    QVariant data = mTypeCmb->itemData( i );

    if ( static_cast<QVariant::Type>( data.toMap().value( QStringLiteral( "type" ) ).toInt() ) == type )
      return i;
  }

  return -1;
}

bool QgsFieldDefinitionWidget::removeType( const QVariant::Type &type )
{
  if ( hasType( type ) )
    return false;

  mTypeCmb->removeItem( typeIndex( type ) );

  return false;
}

QList<QVariant::Type> QgsFieldDefinitionWidget::types() const
{
  QList<QVariant::Type> result;

  for ( int i = 0; i < mTypeCmb->count(); i++ )
    result << static_cast<QVariant::Type>( mTypeCmb->itemData( i ).toMap().value( QStringLiteral( "type" ) ).toInt() );

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

  QgsField *field = new QgsField( name(), QVariant::String, "", length(), precision(), comment() );
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
