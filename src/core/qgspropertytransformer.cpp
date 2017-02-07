/***************************************************************************
     qgspropertytransformer.cpp
     --------------------------
    Date                 : January 2017
    Copyright            : (C) 2017 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspropertytransformer.h"

#include "qgslogger.h"
#include "qgsexpression.h"
#include "qgsfeature.h"
#include "qgssymbollayerutils.h"
#include "qgscolorramp.h"
#include <qmath.h>


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

QString QgsSizeScaleTransformer::toExpression( const QString& baseExpression ) const
{
  QString minValueString = QString::number( mMinValue );
  QString maxValueString = QString::number( mMaxValue );
  QString minSizeString = QString::number( mMinSize );
  QString maxSizeString = QString::number( mMaxSize );
  QString nullSizeString = QString::number( mNullSize );
  QString exponentString = QString::number( mExponent );

  switch ( mType )
  {
    case Linear:
      return QStringLiteral( "coalesce(scale_linear(%1, %2, %3, %4, %5), %6)" ).arg( baseExpression, minValueString, maxValueString, minSizeString, maxSizeString, nullSizeString );

    case Area:
    case Flannery:
    case Exponential:
      return QStringLiteral( "coalesce(scale_exp(%1, %2, %3, %4, %5, %6), %7)" ).arg( baseExpression, minValueString, maxValueString, minSizeString, maxSizeString, exponentString, nullSizeString );

  }
  return QString();
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
    , mRampName( other.mRampName )
{

}

QgsColorRampTransformer &QgsColorRampTransformer::operator=( const QgsColorRampTransformer & other )
{
  mMinValue = other.mMinValue;
  mMaxValue = other.mMaxValue;
  mGradientRamp.reset( other.mGradientRamp ? other.mGradientRamp->clone() : nullptr );
  mNullColor = other.mNullColor;
  mRampName = other.mRampName;
  return *this;
}

QgsColorRampTransformer* QgsColorRampTransformer::clone()
{
  QgsColorRampTransformer* c = new QgsColorRampTransformer( mMinValue, mMaxValue,
      mGradientRamp ? mGradientRamp->clone() : nullptr,
      mNullColor );
  c->setRampName( mRampName );
  return c;
}

bool QgsColorRampTransformer::writeXml( QDomElement &transformerElem, QDomDocument &doc ) const
{
  if ( !QgsPropertyTransformer::writeXml( transformerElem, doc ) )
    return false;

  if ( mGradientRamp )
  {
    QDomElement colorRampElem = QgsSymbolLayerUtils::saveColorRamp( "[source]", mGradientRamp.get(), doc );
    transformerElem.appendChild( colorRampElem );
  }
  transformerElem.setAttribute( "nullColor", QgsSymbolLayerUtils::encodeColor( mNullColor ) );
  transformerElem.setAttribute( "rampName", mRampName );

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
  mRampName = transformerElem.attribute( "rampName", QString() );
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

QString QgsColorRampTransformer::toExpression( const QString& baseExpression ) const
{
  if ( !mGradientRamp )
    return QgsExpression::quotedValue( mNullColor.name() );

  QString minValueString = QString::number( mMinValue );
  QString maxValueString = QString::number( mMaxValue );
  QString nullColorString = mNullColor.name();

  return QStringLiteral( "coalesce(ramp_color('%1',scale_linear(%2, %3, %4, 0, 1), '%5')" ).arg( !mRampName.isEmpty() ? mRampName : QStringLiteral( "custom ramp" ),
         baseExpression, minValueString, maxValueString, nullColorString );
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
  return mGradientRamp.get();
}

void QgsColorRampTransformer::setColorRamp( QgsColorRamp* ramp )
{
  mGradientRamp.reset( ramp );
}

