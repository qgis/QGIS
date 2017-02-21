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

QgsPropertyTransformer::QgsPropertyTransformer( const QgsPropertyTransformer& other )
    : mMinValue( other.mMinValue )
    , mMaxValue( other.mMaxValue )
    , mCurveTransform( other.mCurveTransform ? new QgsCurveTransform( *other.mCurveTransform ) : nullptr )
{}

QgsPropertyTransformer& QgsPropertyTransformer::operator=( const QgsPropertyTransformer & other )
{
  mMinValue = other.mMinValue;
  mMaxValue = other.mMaxValue;
  mCurveTransform.reset( other.mCurveTransform ? new QgsCurveTransform( *other.mCurveTransform ) : nullptr );
  return *this;
}

bool QgsPropertyTransformer::writeXml( QDomElement& transformerElem, QDomDocument& doc ) const
{
  Q_UNUSED( doc );
  transformerElem.setAttribute( "minValue", QString::number( mMinValue ) );
  transformerElem.setAttribute( "maxValue", QString::number( mMaxValue ) );

  if ( mCurveTransform )
  {
    QDomElement curveElement = doc.createElement( "curve" );
    mCurveTransform->writeXml( curveElement, doc );
    transformerElem.appendChild( curveElement );
  }
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

double QgsPropertyTransformer::transformNumeric( double input ) const
{
  if ( !mCurveTransform )
    return input;

  if ( qgsDoubleNear( mMaxValue, mMinValue ) )
    return input;

  // convert input into target range
  double scaledInput = ( input - mMinValue ) / ( mMaxValue - mMinValue );

  return mMinValue + ( mMaxValue - mMinValue ) * mCurveTransform->y( scaledInput );
}

bool QgsPropertyTransformer::readXml( const QDomElement &transformerElem, const QDomDocument &doc )
{
  Q_UNUSED( doc );
  mMinValue = transformerElem.attribute( "minValue", "0.0" ).toDouble();
  mMaxValue = transformerElem.attribute( "maxValue", "1.0" ).toDouble();
  mCurveTransform.reset( nullptr );

  QDomNodeList curveNodeList = transformerElem.elementsByTagName( "curve" );
  if ( !curveNodeList.isEmpty() )
  {
    QDomElement curveElem = curveNodeList.at( 0 ).toElement();
    mCurveTransform.reset( new QgsCurveTransform() );
    mCurveTransform->readXml( curveElem, doc );
  }

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

QgsGenericNumericTransformer::QgsGenericNumericTransformer( const QgsGenericNumericTransformer& other )
    : QgsPropertyTransformer( other )
    , mMinOutput( other.mMinOutput )
    , mMaxOutput( other.mMaxOutput )
    , mNullOutput( other.mNullOutput )
    , mExponent( other.mExponent )
{}

QgsGenericNumericTransformer& QgsGenericNumericTransformer::operator=( const QgsGenericNumericTransformer & other )
{
  QgsPropertyTransformer::operator=( other );
  mMinOutput = other.mMinOutput;
  mMaxOutput = other.mMaxOutput;
  mNullOutput = other.mNullOutput;
  mExponent = other.mExponent;
  return *this;
}

QgsGenericNumericTransformer *QgsGenericNumericTransformer::clone()
{
  std::unique_ptr< QgsGenericNumericTransformer > t( new QgsGenericNumericTransformer( mMinValue,
      mMaxValue,
      mMinOutput,
      mMaxOutput,
      mNullOutput,
      mExponent ) );
  if ( mCurveTransform )
    t->setCurveTransform( new QgsCurveTransform( *mCurveTransform ) );
  return t.release();
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
    return value( transformNumeric( dblValue ) );
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

QgsSizeScaleTransformer::QgsSizeScaleTransformer( const QgsSizeScaleTransformer& other )
    : QgsPropertyTransformer( other )
    , mType( other.mType )
    , mMinSize( other.mMinSize )
    , mMaxSize( other.mMaxSize )
    , mNullSize( other.mNullSize )
    , mExponent( other.mExponent )
{}

QgsSizeScaleTransformer& QgsSizeScaleTransformer::operator=( const QgsSizeScaleTransformer & other )
{
  QgsPropertyTransformer::operator=( other );
  mType = other.mType;
  mMinSize = other.mMinSize;
  mMaxSize = other.mMaxSize;
  mNullSize = other.mNullSize;
  mExponent = other.mExponent;
  return *this;
}

QgsSizeScaleTransformer *QgsSizeScaleTransformer::clone()
{
  std::unique_ptr< QgsSizeScaleTransformer > t( new QgsSizeScaleTransformer( mType,
      mMinValue,
      mMaxValue,
      mMinSize,
      mMaxSize,
      mNullSize,
      mExponent ) );
  if ( mCurveTransform )
    t->setCurveTransform( new QgsCurveTransform( *mCurveTransform ) );
  return t.release();
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
    return size( transformNumeric( dblValue ) );
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
  QgsPropertyTransformer::operator=( other );
  mMinValue = other.mMinValue;
  mMaxValue = other.mMaxValue;
  mGradientRamp.reset( other.mGradientRamp ? other.mGradientRamp->clone() : nullptr );
  mNullColor = other.mNullColor;
  mRampName = other.mRampName;
  return *this;
}

QgsColorRampTransformer* QgsColorRampTransformer::clone()
{
  std::unique_ptr< QgsColorRampTransformer > c( new QgsColorRampTransformer( mMinValue, mMaxValue,
      mGradientRamp ? mGradientRamp->clone() : nullptr,
      mNullColor ) );
  c->setRampName( mRampName );
  if ( mCurveTransform )
    c->setCurveTransform( new QgsCurveTransform( *mCurveTransform ) );
  return c.release();
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
    return color( transformNumeric( dblValue ) );
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


//
// QgsCurveTransform
//

bool sortByX( const QgsPoint& a, const QgsPoint& b )
{
  return a.x() < b.x();
}

QgsCurveTransform::QgsCurveTransform()
{
  mControlPoints << QgsPoint( 0, 0 ) << QgsPoint( 1, 1 );
  calcSecondDerivativeArray();
}

QgsCurveTransform::QgsCurveTransform( const QList<QgsPoint>& controlPoints )
    : mControlPoints( controlPoints )
{
  std::sort( mControlPoints.begin(), mControlPoints.end(), sortByX );
  calcSecondDerivativeArray();
}

QgsCurveTransform::~QgsCurveTransform()
{
  delete [] mSecondDerivativeArray;
}

QgsCurveTransform::QgsCurveTransform( const QgsCurveTransform& other )
    : mControlPoints( other.mControlPoints )
{
  if ( other.mSecondDerivativeArray )
  {
    mSecondDerivativeArray = new double[ mControlPoints.count()];
    memcpy( mSecondDerivativeArray, other.mSecondDerivativeArray, sizeof( double ) * mControlPoints.count() );
  }
}

QgsCurveTransform& QgsCurveTransform::operator=( const QgsCurveTransform & other )
{
  mControlPoints = other.mControlPoints;
  if ( other.mSecondDerivativeArray )
  {
    delete [] mSecondDerivativeArray;
    mSecondDerivativeArray = new double[ mControlPoints.count()];
    memcpy( mSecondDerivativeArray, other.mSecondDerivativeArray, sizeof( double ) * mControlPoints.count() );
  }
  return *this;
}

void QgsCurveTransform::setControlPoints( const QList<QgsPoint>& points )
{
  mControlPoints = points;
  std::sort( mControlPoints.begin(), mControlPoints.end(), sortByX );
  calcSecondDerivativeArray();
}

void QgsCurveTransform::addControlPoint( double x, double y )
{
  QgsPoint point( x, y );
  if ( mControlPoints.contains( point ) )
    return;

  mControlPoints << point;
  std::sort( mControlPoints.begin(), mControlPoints.end(), sortByX );
  calcSecondDerivativeArray();
}

void QgsCurveTransform::removeControlPoint( double x, double y )
{
  for ( int i = 0; i < mControlPoints.count(); ++i )
  {
    if ( qgsDoubleNear( mControlPoints.at( i ).x(), x )
         && qgsDoubleNear( mControlPoints.at( i ).y(), y ) )
    {
      mControlPoints.removeAt( i );
      break;
    }
  }
  calcSecondDerivativeArray();
}

// this code is adapted from https://github.com/OpenFibers/Photoshop-Curves
// which in turn was adapted from
// http://www.developpez.net/forums/d331608-3/autres-langages/algorithmes/contribuez/image-interpolation-spline-cubique/#post3513925  //#spellok

double QgsCurveTransform::y( double x ) const
{
  int n = mControlPoints.count();
  if ( n < 2 )
    return x; // invalid
  else if ( n < 3 )
  {
    // linear
    if ( x <= mControlPoints.at( 0 ).x() )
      return mControlPoints.at( 0 ).y();
    else if ( x >=  mControlPoints.at( n - 1 ).x() )
      return mControlPoints.at( 1 ).y();
    else
    {
      double dx = mControlPoints.at( 1 ).x() - mControlPoints.at( 0 ).x();
      double dy = mControlPoints.at( 1 ).y() - mControlPoints.at( 0 ).y();
      return x * ( dy / dx ) + mControlPoints.at( 0 ).y();
    }
  }

  // safety check
  if ( x <= mControlPoints.at( 0 ).x() )
    return mControlPoints.at( 0 ).y();
  if ( x >= mControlPoints.at( n - 1 ).x() )
    return mControlPoints.at( n - 1 ).y();

  // find corresponding segment
  QList<QgsPoint>::const_iterator pointIt = mControlPoints.constBegin();
  QgsPoint currentControlPoint = *pointIt;
  ++pointIt;
  QgsPoint nextControlPoint = *pointIt;

  for ( int i = 0; i < n - 1; ++i )
  {
    if ( x < nextControlPoint.x() )
    {
      // found segment
      double h = nextControlPoint.x() - currentControlPoint.x();
      double t = ( x - currentControlPoint.x() ) / h;

      double a = 1 - t;

      return a*currentControlPoint.y() + t*nextControlPoint.y() + ( h*h / 6 )*(( a*a*a - a )*mSecondDerivativeArray[i] + ( t*t*t - t )*mSecondDerivativeArray[i+1] );
    }

    ++pointIt;
    if ( pointIt == mControlPoints.constEnd() )
      break;

    currentControlPoint = nextControlPoint;
    nextControlPoint = *pointIt;
  }

  //should not happen
  return x;
}

// this code is adapted from https://github.com/OpenFibers/Photoshop-Curves
// which in turn was adapted from
// http://www.developpez.net/forums/d331608-3/autres-langages/algorithmes/contribuez/image-interpolation-spline-cubique/#post3513925  //#spellok

QVector<double> QgsCurveTransform::y( const QVector<double>& x ) const
{
  QVector<double> result;

  int n = mControlPoints.count();
  if ( n < 3 )
  {
    // invalid control points - use simple transform
    Q_FOREACH ( double i, x )
      result << y( i );

    return result;
  }

  // find corresponding segment
  QList<QgsPoint>::const_iterator pointIt = mControlPoints.constBegin();
  QgsPoint currentControlPoint = *pointIt;
  ++pointIt;
  QgsPoint nextControlPoint = *pointIt;

  int xIndex = 0;
  double currentX = x.at( xIndex );
  // safety check
  while ( currentX <= currentControlPoint.x() )
  {
    result << currentControlPoint.y();
    xIndex++;
    currentX = x.at( xIndex );
  }

  for ( int i = 0; i < n - 1; ++i )
  {
    while ( currentX < nextControlPoint.x() )
    {
      // found segment
      double h = nextControlPoint.x() - currentControlPoint.x();

      double t = ( currentX - currentControlPoint.x() ) / h;

      double a = 1 - t;

      result << a*currentControlPoint.y() + t*nextControlPoint.y() + ( h*h / 6 )*(( a*a*a - a )*mSecondDerivativeArray[i] + ( t*t*t - t )*mSecondDerivativeArray[i+1] );
      xIndex++;
      if ( xIndex == x.count() )
        return result;

      currentX = x.at( xIndex );
    }

    ++pointIt;
    if ( pointIt == mControlPoints.constEnd() )
      break;

    currentControlPoint = nextControlPoint;
    nextControlPoint = *pointIt;
  }

  // safety check
  while ( xIndex < x.count() )
  {
    result << nextControlPoint.y();
    xIndex++;
  }

  return result;
}

bool QgsCurveTransform::readXml( const QDomElement& elem, const QDomDocument& )
{
  QString xString = elem.attribute( QStringLiteral( "x" ) );
  QString yString = elem.attribute( QStringLiteral( "y" ) );

  QStringList xVals = xString.split( ',' );
  QStringList yVals = yString.split( ',' );
  if ( xVals.count() != yVals.count() )
    return false;

  QList< QgsPoint > newPoints;
  bool ok = false;
  for ( int i = 0; i < xVals.count(); ++i )
  {
    double x = xVals.at( i ).toDouble( &ok );
    if ( !ok )
      return false;
    double y = yVals.at( i ).toDouble( &ok );
    if ( !ok )
      return false;
    newPoints << QgsPoint( x, y );
  }
  setControlPoints( newPoints );
  return true;
}

bool QgsCurveTransform::writeXml( QDomElement& transformElem, QDomDocument& ) const
{
  QStringList x;
  QStringList y;
  Q_FOREACH ( const QgsPoint& p, mControlPoints )
  {
    x << qgsDoubleToString( p.x() );
    y << qgsDoubleToString( p.y() );
  }

  transformElem.setAttribute( QStringLiteral( "x" ), x.join( ',' ) );
  transformElem.setAttribute( QStringLiteral( "y" ), y.join( ',' ) );

  return true;
}

// this code is adapted from https://github.com/OpenFibers/Photoshop-Curves
// which in turn was adapted from
// http://www.developpez.net/forums/d331608-3/autres-langages/algorithmes/contribuez/image-interpolation-spline-cubique/#post3513925  //#spellok

void QgsCurveTransform::calcSecondDerivativeArray()
{
  int n = mControlPoints.count();
  if ( n < 3 )
    return; // cannot proceed

  delete[] mSecondDerivativeArray;

  double* matrix = new double[ n * 3 ];
  double* result = new double[ n ];
  matrix[0] = 0;
  matrix[1] = 1;
  matrix[2] = 0;
  result[0] = 0;
  QList<QgsPoint>::const_iterator pointIt = mControlPoints.constBegin();
  QgsPoint pointIm1 = *pointIt;
  ++pointIt;
  QgsPoint pointI = *pointIt;
  ++pointIt;
  QgsPoint pointIp1 = *pointIt;

  for ( int i = 1; i < n - 1; ++i )
  {
    matrix[i * 3 + 0 ] = ( pointI.x() - pointIm1.x() ) / 6.0;
    matrix[i * 3 + 1 ] = ( pointIp1.x() - pointIm1.x() ) / 3.0;
    matrix[i * 3 + 2 ] = ( pointIp1.x() - pointI.x() ) / 6.0;
    result[i] = ( pointIp1.y() - pointI.y() ) / ( pointIp1.x() - pointI.x() ) - ( pointI.y() - pointIm1.y() ) / ( pointI.x() - pointIm1.x() );

    // shuffle points
    pointIm1 = pointI;
    pointI = pointIp1;
    ++pointIt;
    if ( pointIt == mControlPoints.constEnd() )
      break;

    pointIp1 = *pointIt;
  }
  matrix[( n-1 )*3 + 0] = 0;
  matrix[( n-1 )*3 + 1] = 1;
  matrix[( n-1 ) * 3 +2] = 0;
  result[n-1] = 0;

  // solving pass1 (up->down)
  for ( int i = 1; i < n; ++i )
  {
    double k = matrix[i * 3 + 0] / matrix[( i-1 ) * 3 + 1];
    matrix[i * 3 + 1] -= k * matrix[( i-1 )*3+2];
    matrix[i * 3 + 0] = 0;
    result[i] -= k * result[i-1];
  }
  // solving pass2 (down->up)
  for ( int i = n - 2; i >= 0; --i )
  {
    double k = matrix[i*3+2] / matrix[( i+1 )*3+1];
    matrix[i*3+1] -= k * matrix[( i+1 )*3+0];
    matrix[i*3+2] = 0;
    result[i] -= k * result[i+1];
  }

  // return second derivative value for each point
  mSecondDerivativeArray = new double[n];
  for ( int i = 0;i < n;++i )
  {
    mSecondDerivativeArray[i] = result[i] / matrix[( i*3 )+1];
  }

  delete[] result;
  delete[] matrix;
}

