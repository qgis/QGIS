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
    case GenericNumericTransformer:
      transformer = new QgsGenericNumericTransformer();
      break;
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

QgsPropertyTransformer* QgsPropertyTransformer::fromExpression( const QString& expression, QString& baseExpression, QString& fieldName )
{
  baseExpression.clear();
  fieldName.clear();

  if ( QgsPropertyTransformer* sizeScale = QgsSizeScaleTransformer::fromExpression( expression, baseExpression, fieldName ) )
    return sizeScale;
  else
    return nullptr;
}

bool QgsPropertyTransformer::readXml( const QDomElement &transformerElem, const QDomDocument &doc )
{
  Q_UNUSED( doc );
  mMinValue = transformerElem.attribute( "minValue", "0.0" ).toDouble();
  mMaxValue = transformerElem.attribute( "maxValue", "1.0" ).toDouble();
  return true;
}

//
// QgsGenericNumericTransformer
//

QgsGenericNumericTransformer::QgsGenericNumericTransformer( double minValue, double maxValue, double minOutput, double maxOutput, double nullOutput, double exponent )
    : QgsPropertyTransformer( minValue, maxValue )
    , mMinOutput( minOutput )
    , mMaxOutput( maxOutput )
    , mNullOutput( nullOutput )
    , mExponent( exponent )
{}

QgsGenericNumericTransformer *QgsGenericNumericTransformer::clone()
{
  return new QgsGenericNumericTransformer( mMinValue,
         mMaxValue,
         mMinOutput,
         mMaxOutput,
         mNullOutput,
         mExponent );
}

bool QgsGenericNumericTransformer::writeXml( QDomElement &transformerElem, QDomDocument &doc ) const
{
  if ( !QgsPropertyTransformer::writeXml( transformerElem, doc ) )
    return false;

  transformerElem.setAttribute( "minOutput", QString::number( mMinOutput ) );
  transformerElem.setAttribute( "maxOutput", QString::number( mMaxOutput ) );
  transformerElem.setAttribute( "nullOutput", QString::number( mNullOutput ) );
  transformerElem.setAttribute( "exponent", QString::number( mExponent ) );

  return true;
}

bool QgsGenericNumericTransformer::readXml( const QDomElement &transformerElem, const QDomDocument &doc )
{
  if ( !QgsPropertyTransformer::readXml( transformerElem, doc ) )
    return false;

  mMinOutput = transformerElem.attribute( "minOutput", "0.0" ).toDouble();
  mMaxOutput = transformerElem.attribute( "maxOutput", "1.0" ).toDouble();
  mNullOutput = transformerElem.attribute( "nullOutput", "0.0" ).toDouble();
  mExponent = transformerElem.attribute( "exponent", "1.0" ).toDouble();
  return true;
}

double QgsGenericNumericTransformer::value( double input ) const
{
  if ( qgsDoubleNear( mExponent, 1.0 ) )
    return mMinOutput + ( qBound( mMinValue, input, mMaxValue ) - mMinValue ) * ( mMaxOutput - mMinOutput ) / ( mMaxValue - mMinValue );
  else
    return mMinOutput + qPow( qBound( mMinValue, input, mMaxValue ) - mMinValue, mExponent ) * ( mMaxOutput - mMinOutput ) / qPow( mMaxValue - mMinValue, mExponent );
}

QVariant QgsGenericNumericTransformer::transform( const QgsExpressionContext& context, const QVariant& v ) const
{
  Q_UNUSED( context );

  if ( v.isNull() )
    return mNullOutput;

  bool ok;
  double dblValue = v.toDouble( &ok );

  if ( ok )
  {
    //apply scaling to value
    return value( dblValue );
  }
  else
  {
    return v;
  }
}

QString QgsGenericNumericTransformer::toExpression( const QString& baseExpression ) const
{
  QString minValueString = QString::number( mMinValue );
  QString maxValueString = QString::number( mMaxValue );
  QString minOutputString = QString::number( mMinOutput );
  QString maxOutputString = QString::number( mMaxOutput );
  QString nullOutputString = QString::number( mNullOutput );
  QString exponentString = QString::number( mExponent );

  if ( qgsDoubleNear( mExponent, 1.0 ) )
    return QStringLiteral( "coalesce(scale_linear(%1, %2, %3, %4, %5), %6)" ).arg( baseExpression, minValueString, maxValueString, minOutputString, maxOutputString, nullOutputString );
  else
    return QStringLiteral( "coalesce(scale_exp(%1, %2, %3, %4, %5, %6), %7)" ).arg( baseExpression, minValueString, maxValueString, minOutputString, maxOutputString, exponentString, nullOutputString );
}

QgsGenericNumericTransformer* QgsGenericNumericTransformer::fromExpression( const QString& expression, QString& baseExpression, QString& fieldName )
{
  bool ok = false;

  double nullValue = 0.0;
  double exponent = 1.0;

  baseExpression.clear();
  fieldName.clear();

  QgsExpression e( expression );

  if ( !e.rootNode() )
    return nullptr;

  const QgsExpression::NodeFunction * f = dynamic_cast<const QgsExpression::NodeFunction*>( e.rootNode() );
  if ( !f )
    return nullptr;

  QList<QgsExpression::Node*> args = f->args()->list();

  // the scale function may be enclosed in a coalesce(expr, 0) to avoid NULL value
  // to be drawn with the default size
  if ( "coalesce" == QgsExpression::Functions()[f->fnIndex()]->name() )
  {
    f = dynamic_cast<const QgsExpression::NodeFunction*>( args[0] );
    if ( !f )
      return nullptr;
    nullValue = QgsExpression( args[1]->dump() ).evaluate().toDouble( &ok );
    if ( ! ok )
      return nullptr;
    args = f->args()->list();
  }

  if ( "scale_linear" == QgsExpression::Functions()[f->fnIndex()]->name() )
  {
    exponent = 1.0;
  }
  else if ( "scale_exp" == QgsExpression::Functions()[f->fnIndex()]->name() )
  {
    exponent = QgsExpression( args[5]->dump() ).evaluate().toDouble( &ok );
  }
  else
  {
    return nullptr;
  }

  bool expOk = true;
  double minValue = QgsExpression( args[1]->dump() ).evaluate().toDouble( &ok );
  expOk &= ok;
  double maxValue = QgsExpression( args[2]->dump() ).evaluate().toDouble( &ok );
  expOk &= ok;
  double minOutput = QgsExpression( args[3]->dump() ).evaluate().toDouble( &ok );
  expOk &= ok;
  double maxOutput = QgsExpression( args[4]->dump() ).evaluate().toDouble( &ok );
  expOk &= ok;

  if ( !expOk )
  {
    return nullptr;
  }

  if ( args[0]->nodeType() == QgsExpression::ntColumnRef )
  {
    fieldName = static_cast< QgsExpression::NodeColumnRef* >( args[0] )->name();
  }
  else
  {
    baseExpression = args[0]->dump();
  }
  return new QgsGenericNumericTransformer( minValue, maxValue, minOutput, maxOutput, nullValue, exponent );
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

QgsSizeScaleTransformer* QgsSizeScaleTransformer::fromExpression( const QString& expression, QString& baseExpression, QString& fieldName )
{
  bool ok = false;

  ScaleType type = Linear;
  double nullSize = 0.0;
  double exponent = 1.0;

  baseExpression.clear();
  fieldName.clear();

  QgsExpression e( expression );

  if ( !e.rootNode() )
    return nullptr;

  const QgsExpression::NodeFunction * f = dynamic_cast<const QgsExpression::NodeFunction*>( e.rootNode() );
  if ( !f )
    return nullptr;

  QList<QgsExpression::Node*> args = f->args()->list();

  // the scale function may be enclosed in a coalesce(expr, 0) to avoid NULL value
  // to be drawn with the default size
  if ( "coalesce" == QgsExpression::Functions()[f->fnIndex()]->name() )
  {
    f = dynamic_cast<const QgsExpression::NodeFunction*>( args[0] );
    if ( !f )
      return nullptr;
    nullSize = QgsExpression( args[1]->dump() ).evaluate().toDouble( &ok );
    if ( ! ok )
      return nullptr;
    args = f->args()->list();
  }

  if ( "scale_linear" == QgsExpression::Functions()[f->fnIndex()]->name() )
  {
    type = Linear;
  }
  else if ( "scale_exp" == QgsExpression::Functions()[f->fnIndex()]->name() )
  {
    exponent = QgsExpression( args[5]->dump() ).evaluate().toDouble( &ok );
    if ( ! ok )
      return nullptr;
    if ( qgsDoubleNear( exponent, 0.57, 0.001 ) )
      type = Flannery;
    else if ( qgsDoubleNear( exponent, 0.5, 0.001 ) )
      type = Area;
    else
      type = Exponential;
  }
  else
  {
    return nullptr;
  }

  bool expOk = true;
  double minValue = QgsExpression( args[1]->dump() ).evaluate().toDouble( &ok );
  expOk &= ok;
  double maxValue = QgsExpression( args[2]->dump() ).evaluate().toDouble( &ok );
  expOk &= ok;
  double minSize = QgsExpression( args[3]->dump() ).evaluate().toDouble( &ok );
  expOk &= ok;
  double maxSize = QgsExpression( args[4]->dump() ).evaluate().toDouble( &ok );
  expOk &= ok;

  if ( !expOk )
  {
    return nullptr;
  }

  if ( args[0]->nodeType() == QgsExpression::ntColumnRef )
  {
    fieldName = static_cast< QgsExpression::NodeColumnRef* >( args[0] )->name();
  }
  else
  {
    baseExpression = args[0]->dump();
  }
  return new QgsSizeScaleTransformer( type, minValue, maxValue, minSize, maxSize, nullSize, exponent );
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
