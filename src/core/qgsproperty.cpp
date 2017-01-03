/***************************************************************************
     qgsproperty.cpp
     ---------------
    Date                 : April 2015
    Copyright            : (C) 2015 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsproperty.h"

#include "qgslogger.h"
#include "qgsexpression.h"
#include "qgsfeature.h"
#include "qgssymbollayerutils.h"
#include "qgscolorramp.h"
#include <qmath.h>

//
// QgsAbstractProperty
//

QgsAbstractProperty *QgsAbstractProperty::create( QgsAbstractProperty::Type type )
{
  QgsAbstractProperty* prop = nullptr;
  switch ( type )
  {
    case QgsAbstractProperty::StaticProperty:
      prop = new QgsStaticProperty();
      break;
    case QgsAbstractProperty::FieldBasedProperty:
      prop = new QgsFieldBasedProperty();
      break;
    case QgsAbstractProperty::ExpressionBasedProperty:
      prop = new QgsExpressionBasedProperty();
      break;
  }
  return prop;
}

QgsAbstractProperty::QgsAbstractProperty( bool active )
    : mActive( active )
{

}

QgsAbstractProperty::QgsAbstractProperty( const QgsAbstractProperty& other )
    : mActive( other.mActive )
    , mTransformer( other.mTransformer ? other.mTransformer->clone() : nullptr )
{

}

QgsAbstractProperty &QgsAbstractProperty::operator=( const QgsAbstractProperty & other )
{
  mActive = other.mActive;
  mTransformer.reset( other.mTransformer ? other.mTransformer->clone() : nullptr );
  return *this;
}

QVariant QgsAbstractProperty::value( const QgsExpressionContext& context, const QVariant& defaultValue ) const
{
  QVariant val = propertyValue( context, defaultValue );

  if ( mTransformer )
  {
    val = mTransformer->transform( context, val );
  }

  return val;
}

QColor QgsAbstractProperty::valueAsColor( const QgsExpressionContext &context, const QColor &defaultColor ) const
{
  QVariant val = value( context, defaultColor );

  if ( !val.isValid() )
    return defaultColor;

  QColor color;
  if ( val.type() == QVariant::Color )
  {
    color = val.value<QColor>();
  }
  else
  {
    color = QgsSymbolLayerUtils::decodeColor( val.toString() );
  }

  if ( !color.isValid() )
    return defaultColor;
  else
    return color;
}

double QgsAbstractProperty::valueAsDouble( const QgsExpressionContext &context, double defaultValue ) const
{
  QVariant val = value( context, defaultValue );

  if ( !val.isValid() )
    return defaultValue;

  bool ok = false;
  double dbl = val.toDouble( &ok );
  if ( !ok )
    return defaultValue;
  else
    return dbl;
}

int QgsAbstractProperty::valueAsInt( const QgsExpressionContext &context, int defaultValue ) const
{
  QVariant val = value( context, defaultValue );

  if ( !val.isValid() )
    return defaultValue;

  bool ok = false;
  int integer = val.toInt( &ok );
  if ( !ok )
  {
    //one more option to try
    double dbl = val.toDouble( &ok );
    if ( ok )
    {
      return qRound( dbl );
    }
    else
    {
      return defaultValue;
    }
  }
  else
    return integer;
}

bool QgsAbstractProperty::writeXml( QDomElement &propertyElem, QDomDocument &doc ) const
{
  Q_UNUSED( doc );
  propertyElem.setAttribute( "active", mActive ? "1" : "0" );

  if ( mTransformer )
  {
    QDomElement transformerElem = doc.createElement( "transformer" );
    transformerElem.setAttribute( "t", static_cast< int >( mTransformer->transformerType() ) );
    if ( mTransformer->writeXml( transformerElem, doc ) )
      propertyElem.appendChild( transformerElem );
  }

  return true;
}

bool QgsAbstractProperty::readXml( const QDomElement &propertyElem, const QDomDocument &doc )
{
  mActive = static_cast< bool >( propertyElem.attribute( "active", "1" ).toInt() );

  //restore transformer if present
  mTransformer.reset( nullptr );
  QDomNodeList transformerNodeList = propertyElem.elementsByTagName( "transformer" );
  if ( !transformerNodeList.isEmpty() )
  {
    QDomElement transformerElem = transformerNodeList.at( 0 ).toElement();
    QgsPropertyTransformer::Type type = static_cast< QgsPropertyTransformer::Type >( transformerElem.attribute( "t", "0" ).toInt() );
    QScopedPointer< QgsPropertyTransformer > transformer( QgsPropertyTransformer::create( type ) );
    if ( transformer )
    {
      if ( transformer->readXml( transformerElem, doc ) )
        mTransformer.reset( transformer.take() );
    }
  }

  return true;
}


void QgsAbstractProperty::setTransformer( QgsPropertyTransformer* transformer )
{
  mTransformer.reset( transformer );
}

//
// QgsStaticProperty
//

QgsStaticProperty::QgsStaticProperty( const QVariant& value, bool active )
    : QgsAbstractProperty( active )
    , mValue( value )
{

}

QgsStaticProperty* QgsStaticProperty::clone()
{
  return new QgsStaticProperty( *this );
}

QVariant QgsStaticProperty::propertyValue( const QgsExpressionContext& context, const QVariant& defaultValue ) const
{
  Q_UNUSED( context );
  return mActive ? mValue : defaultValue;
}

bool QgsStaticProperty::writeXml( QDomElement &propertyElem, QDomDocument &doc ) const
{
  if ( !QgsAbstractProperty::writeXml( propertyElem, doc ) )
    return false;

  propertyElem.setAttribute( "type", mValue.typeName() );
  propertyElem.setAttribute( "val", mValue.toString() );
  return true;
}

bool QgsStaticProperty::readXml( const QDomElement &propertyElem, const QDomDocument &doc )
{
  if ( !QgsAbstractProperty::readXml( propertyElem, doc ) )
    return false;

  mValue =  QVariant( propertyElem.attribute( "val", "1" ) );
  mValue.convert( QVariant::nameToType( propertyElem.attribute( "type", "QString" ).toLocal8Bit().constData() ) );
  return true;
}

//
// QgsFieldBasedProperty
//

QgsFieldBasedProperty::QgsFieldBasedProperty( const QString& field, bool isActive )
    : QgsAbstractProperty( isActive )
    , mField( field )
{

}

QgsFieldBasedProperty* QgsFieldBasedProperty::clone()
{
  return new QgsFieldBasedProperty( *this );
}

QVariant QgsFieldBasedProperty::propertyValue( const QgsExpressionContext& context, const QVariant& defaultValue ) const
{
  if ( !mActive )
    return defaultValue;

  QgsFeature f = context.feature();
  if ( !f.isValid() )
    return defaultValue;

  int fieldIdx = f.fieldNameIndex( mField );
  if ( fieldIdx < 0 )
    return defaultValue;

  return f.attribute( fieldIdx );
}

QSet< QString > QgsFieldBasedProperty::referencedFields( const QgsExpressionContext &context ) const
{
  Q_UNUSED( context );
  QSet< QString > fields;
  if ( mActive && !mField.isEmpty() )
    fields.insert( mField );
  return fields;
}

bool QgsFieldBasedProperty::writeXml( QDomElement& propertyElem, QDomDocument& doc ) const
{
  if ( !QgsAbstractProperty::writeXml( propertyElem, doc ) )
    return false;

  propertyElem.setAttribute( "field", mField );
  return true;
}

bool QgsFieldBasedProperty::readXml( const QDomElement& propertyElem, const QDomDocument& doc )
{
  if ( !QgsAbstractProperty::readXml( propertyElem, doc ) )
    return false;

  mField = propertyElem.attribute( "field" );
  if ( mField.isEmpty() )
    mActive = false;

  return true;
}


//
// QgsExpressionBasedProperty
//

QgsExpressionBasedProperty::QgsExpressionBasedProperty( const QString& expression, bool isActive )
    : QgsAbstractProperty( isActive )
    , mExpressionString( expression )
    , mPrepared( false )
    , mExpression( expression )
{

}

QgsExpressionBasedProperty* QgsExpressionBasedProperty::clone()
{
  // make sure we do the clone in a way to take advantage of implicit sharing of QgsExpression
  QgsExpressionBasedProperty* clone = new QgsExpressionBasedProperty();
  clone->mActive = mActive;
  clone->mExpression = mExpression;
  clone->mExpressionString = mExpressionString;
  clone->mPrepared = mPrepared;
  clone->mReferencedCols = mReferencedCols;
  clone->setTransformer( mTransformer ? mTransformer->clone() : nullptr );
  return clone;
}

void QgsExpressionBasedProperty::setExpressionString( const QString& expression )
{
  mExpressionString = expression;
  mExpression = QgsExpression( expression );
  mPrepared = false;
}

QVariant QgsExpressionBasedProperty::propertyValue( const QgsExpressionContext& context, const QVariant& defaultValue ) const
{
  if ( !mActive )
    return defaultValue;

  if ( !mPrepared && !prepare( context ) )
    return defaultValue;

  QVariant result = mExpression.evaluate( &context );
  return result.isValid() ? result : defaultValue;
}

QSet< QString > QgsExpressionBasedProperty::referencedFields( const QgsExpressionContext &context ) const
{
  if ( !mActive )
    return QSet< QString >();

  if ( !mPrepared && !prepare( context ) )
    return QSet< QString >();

  return mReferencedCols;
}

bool QgsExpressionBasedProperty::writeXml( QDomElement& propertyElem, QDomDocument& doc ) const
{
  if ( !QgsAbstractProperty::writeXml( propertyElem, doc ) )
    return false;

  propertyElem.setAttribute( "expression", mExpressionString );
  return true;
}

bool QgsExpressionBasedProperty::readXml( const QDomElement& propertyElem, const QDomDocument& doc )
{
  if ( !QgsAbstractProperty::readXml( propertyElem, doc ) )
    return false;

  mExpressionString = propertyElem.attribute( "expression" );
  if ( mExpressionString.isEmpty() )
    mActive = false;

  mExpression = QgsExpression( mExpressionString );
  mPrepared = false;
  mReferencedCols.clear();
  return true;
}

bool QgsExpressionBasedProperty::prepare( const QgsExpressionContext &context ) const
{
  if ( !mExpression.prepare( &context ) )
  {
    mReferencedCols.clear();
    mPrepared = false;
    return false;
  }

  mPrepared = true;
  mReferencedCols = mExpression.referencedColumns();
  return true;
}

//
// QgsPropertyTransformer
//

QgsPropertyTransformer* QgsPropertyTransformer::create( QgsPropertyTransformer::Type type )
{
  QgsPropertyTransformer* transformer = nullptr;
  switch ( type )
  {
    case SizeScaleTransformer:
      transformer = new QgsSizeScaleTransformer();
      break;
    case ColorRampTransformer:
      transformer = new QgsColorRampTransformer();
      break;
  }
  return transformer;
}

QgsPropertyTransformer::QgsPropertyTransformer( double minValue, double maxValue )
    : mMinValue( minValue )
    , mMaxValue( maxValue )
{}

bool QgsPropertyTransformer::writeXml( QDomElement& transformerElem, QDomDocument& doc ) const
{
  Q_UNUSED( doc );
  transformerElem.setAttribute( "minValue", QString::number( mMinValue ) );
  transformerElem.setAttribute( "maxValue", QString::number( mMaxValue ) );
  return true;
}

bool QgsPropertyTransformer::readXml( const QDomElement &transformerElem, const QDomDocument &doc )
{
  Q_UNUSED( doc );
  mMinValue = transformerElem.attribute( "minValue", "0.0" ).toDouble();
  mMaxValue = transformerElem.attribute( "maxValue", "1.0" ).toDouble();
  return true;
}

//
// QgsSizeScaleProperty
//
QgsSizeScaleTransformer::QgsSizeScaleTransformer( ScaleType type, double minValue, double maxValue, double minSize, double maxSize, double nullSize, double exponent )
    : QgsPropertyTransformer( minValue, maxValue )
    , mType( Linear )
    , mMinSize( minSize )
    , mMaxSize( maxSize )
    , mNullSize( nullSize )
    , mExponent( exponent )
{
  setType( type );
}

QgsSizeScaleTransformer *QgsSizeScaleTransformer::clone()
{
  return new QgsSizeScaleTransformer( mType,
                                      mMinValue,
                                      mMaxValue,
                                      mMinSize,
                                      mMaxSize,
                                      mNullSize,
                                      mExponent );
}

bool QgsSizeScaleTransformer::writeXml( QDomElement &transformerElem, QDomDocument &doc ) const
{
  if ( !QgsPropertyTransformer::writeXml( transformerElem, doc ) )
    return false;

  transformerElem.setAttribute( "scaleType", QString::number( static_cast< int >( mType ) ) );
  transformerElem.setAttribute( "minSize", QString::number( mMinSize ) );
  transformerElem.setAttribute( "maxSize", QString::number( mMaxSize ) );
  transformerElem.setAttribute( "nullSize", QString::number( mNullSize ) );
  transformerElem.setAttribute( "exponent", QString::number( mExponent ) );

  return true;
}

bool QgsSizeScaleTransformer::readXml( const QDomElement &transformerElem, const QDomDocument &doc )
{
  if ( !QgsPropertyTransformer::readXml( transformerElem, doc ) )
    return false;

  mType = static_cast< ScaleType >( transformerElem.attribute( "scaleType", "0" ).toInt() );
  mMinSize = transformerElem.attribute( "minSize", "0.0" ).toDouble();
  mMaxSize = transformerElem.attribute( "maxSize", "1.0" ).toDouble();
  mNullSize = transformerElem.attribute( "nullSize", "0.0" ).toDouble();
  mExponent = transformerElem.attribute( "exponent", "1.0" ).toDouble();
  return true;
}

double QgsSizeScaleTransformer::size( double value ) const
{
  switch ( mType )
  {
    case Linear:
      return mMinSize + ( qBound( mMinValue, value, mMaxValue ) - mMinValue ) * ( mMaxSize - mMinSize ) / ( mMaxValue - mMinValue );

    case Area:
    case Flannery:
    case Exponential:
      return mMinSize + qPow( qBound( mMinValue, value, mMaxValue ) - mMinValue, mExponent ) * ( mMaxSize - mMinSize ) / qPow( mMaxValue - mMinValue, mExponent );

  }
  return 0;
}

void QgsSizeScaleTransformer::setType( QgsSizeScaleTransformer::ScaleType type )
{
  mType = type;
  switch ( mType )
  {
    case Linear:
      mExponent = 1.0;
      break;
    case Area:
      mExponent = 0.5;
      break;
    case Flannery:
      mExponent = 0.57;
      break;
    case Exponential:
      //no change
      break;
  }
}

QVariant QgsSizeScaleTransformer::transform( const QgsExpressionContext& context, const QVariant& value ) const
{
  Q_UNUSED( context );

  if ( value.isNull() )
    return mNullSize;

  bool ok;
  double dblValue = value.toDouble( &ok );

  if ( ok )
  {
    //apply scaling to value
    return size( dblValue );
  }
  else
  {
    return value;
  }
}


//
// QgsColorRampTransformer
//

QgsColorRampTransformer::QgsColorRampTransformer( double minValue, double maxValue,
    QgsColorRamp* ramp,
    const QColor &nullColor )
    : QgsPropertyTransformer( minValue, maxValue )
    , mGradientRamp( ramp )
    , mNullColor( nullColor )
{

}

QgsColorRampTransformer::QgsColorRampTransformer( const QgsColorRampTransformer &other )
    : QgsPropertyTransformer( other )
    , mGradientRamp( other.mGradientRamp ? other.mGradientRamp->clone() : nullptr )
    , mNullColor( other.mNullColor )
{

}

QgsColorRampTransformer &QgsColorRampTransformer::operator=( const QgsColorRampTransformer & other )
{
  mMinValue = other.mMinValue;
  mMaxValue = other.mMaxValue;
  mGradientRamp.reset( other.mGradientRamp ? other.mGradientRamp->clone() : nullptr );
  mNullColor = other.mNullColor;
  return *this;
}

QgsColorRampTransformer* QgsColorRampTransformer::clone()
{
  return new QgsColorRampTransformer( mMinValue, mMaxValue,
                                      mGradientRamp ? mGradientRamp->clone() : nullptr,
                                      mNullColor );
}

bool QgsColorRampTransformer::writeXml( QDomElement &transformerElem, QDomDocument &doc ) const
{
  if ( !QgsPropertyTransformer::writeXml( transformerElem, doc ) )
    return false;

  if ( mGradientRamp )
  {
    QDomElement colorRampElem = QgsSymbolLayerUtils::saveColorRamp( "[source]", mGradientRamp.data(), doc );
    transformerElem.appendChild( colorRampElem );
  }
  transformerElem.setAttribute( "nullColor", QgsSymbolLayerUtils::encodeColor( mNullColor ) );

  return true;
}

bool QgsColorRampTransformer::readXml( const QDomElement &transformerElem, const QDomDocument &doc )
{
  if ( !QgsPropertyTransformer::readXml( transformerElem, doc ) )
    return false;

  mGradientRamp.reset( nullptr );
  QDomElement sourceColorRampElem = transformerElem.firstChildElement( "colorramp" );
  if ( !sourceColorRampElem.isNull() && sourceColorRampElem.attribute( "name" ) == "[source]" )
  {
    setColorRamp( QgsSymbolLayerUtils::loadColorRamp( sourceColorRampElem ) );
  }

  mNullColor = QgsSymbolLayerUtils::decodeColor( transformerElem.attribute( "nullColor", "0,0,0,0" ) );
  return true;
}

QVariant QgsColorRampTransformer::transform( const QgsExpressionContext &context, const QVariant &value ) const
{
  Q_UNUSED( context );

  if ( value.isNull() )
    return mNullColor;

  bool ok;
  double dblValue = value.toDouble( &ok );

  if ( ok )
  {
    //apply scaling to value
    return color( dblValue );
  }
  else
  {
    return value;
  }
}

QColor QgsColorRampTransformer::color( double value ) const
{
  double scaledVal = qBound( 0.0, ( value - mMinValue ) / ( mMaxValue - mMinValue ), 1.0 );

  if ( !mGradientRamp )
    return mNullColor;

  return mGradientRamp->color( scaledVal );
}

QgsColorRamp *QgsColorRampTransformer::colorRamp() const
{
  return mGradientRamp.data();
}

void QgsColorRampTransformer::setColorRamp( QgsColorRamp* ramp )
{
  mGradientRamp.reset( ramp );
}

