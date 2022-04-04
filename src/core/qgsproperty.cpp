/***************************************************************************
     qgsproperty.cpp
     ---------------
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

#include "qgsproperty.h"
#include "qgsproperty_p.h"

#include "qgslogger.h"
#include "qgsexpression.h"
#include "qgsfeature.h"
#include "qgssymbollayerutils.h"
#include "qgscolorramp.h"
#include "qgsexpressionnodeimpl.h"

#include <QRegularExpression>

QgsPropertyDefinition::QgsPropertyDefinition( const QString &name, const QString &description, QgsPropertyDefinition::StandardPropertyTemplate type, const QString &origin, const QString &comment )
  : mName( name )
  , mDescription( description )
  , mStandardType( type )
  , mOrigin( origin )
  , mComment( comment )
{
  switch ( mStandardType )
  {
    case Boolean:
      mTypes = DataTypeBoolean;
      mHelpText = QObject::tr( "bool [<b>1</b>=True|<b>0</b>=False]" );
      break;

    case Integer:
      mTypes = DataTypeNumeric;
      mHelpText = QObject::tr( "int [≤ 0 ≥]" );
      break;

    case IntegerPositive:
      mTypes = DataTypeNumeric;
      mHelpText = QObject::tr( "int [≥ 0]" );
      break;

    case IntegerPositiveGreaterZero:
      mTypes = DataTypeNumeric;
      mHelpText = QObject::tr( "int [≥ 1]" );
      break;

    case Double:
      mTypes = DataTypeNumeric;
      mHelpText = QObject::tr( "double [≤ 0.0 ≥]" );
      break;

    case DoublePositive:
      mTypes = DataTypeNumeric;
      mHelpText = QObject::tr( "double [≥ 0.0]" );
      break;

    case Double0To1:
      mTypes = DataTypeNumeric;
      mHelpText = QObject::tr( "double [0.0-1.0]" );
      break;

    case Rotation:
      mTypes = DataTypeNumeric;
      mHelpText = QObject::tr( "double [0.0-360.0]" );
      break;

    case String:
      mTypes = DataTypeString;
      mHelpText = QObject::tr( "string of variable length" );
      break;

    case Opacity:
      mTypes = DataTypeNumeric;
      mHelpText = QObject::tr( "int [0-100]" );
      break;

    case RenderUnits:
      mTypes = DataTypeString;
      mHelpText = trString() + QStringLiteral( "[<b>MM</b>|<b>MapUnit</b>|<b>Pixel</b>|<b>Point</b>]" );
      break;

    case ColorWithAlpha:
      mTypes = DataTypeString;
      mHelpText = QObject::tr( "string [<b>r,g,b,a</b>] as int 0-255 or #<b>AARRGGBB</b> as hex or <b>color</b> as color's name" );
      break;

    case ColorNoAlpha:
      mTypes = DataTypeString;
      mHelpText = QObject::tr( "string [<b>r,g,b</b>] as int 0-255 or #<b>RRGGBB</b> as hex or <b>color</b> as color's name" );
      break;

    case PenJoinStyle:
      mTypes = DataTypeString;
      mHelpText = trString() + QStringLiteral( "[<b>bevel</b>|<b>miter</b>|<b>round</b>]" );
      break;

    case BlendMode:
      mTypes = DataTypeString;
      mHelpText = trString() + QStringLiteral( "[<b>Normal</b>|<b>Lighten</b>|<b>Screen</b>|<b>Dodge</b>|<br>"
                  "<b>Addition</b>|<b>Darken</b>|<b>Multiply</b>|<b>Burn</b>|<b>Overlay</b>|<br>"
                  "<b>SoftLight</b>|<b>HardLight</b>|<b>Difference</b>|<b>Subtract</b>]" );
      break;

    case Point:
      mTypes = DataTypeString;
      mHelpText = QObject::tr( "double coord [<b>X,Y</b>]" );
      break;

    case Size:
      mTypes = DataTypeNumeric;
      mHelpText = QObject::tr( "double [≥ 0.0]" );
      break;

    case Size2D:
      mTypes = DataTypeString;
      mHelpText = QObject::tr( "string of doubles '<b>width,height</b>' or array of doubles <b>[width, height]</b>" );
      break;

    case LineStyle:
      mTypes = DataTypeString;
      mHelpText = trString() + QStringLiteral( "[<b>no</b>|<b>solid</b>|<b>dash</b>|<b>dot</b>|<b>dash dot</b>|<b>dash dot dot</b>]" );
      break;

    case StrokeWidth:
      mTypes = DataTypeNumeric;
      mHelpText = QObject::tr( "double [≥ 0.0]" );
      break;

    case FillStyle:
      mTypes = DataTypeString;
      mHelpText = trString() + QStringLiteral( "[<b>solid</b>|<b>horizontal</b>|<b>vertical</b>|<b>cross</b>|<b>b_diagonal</b>|<b>f_diagonal"
                  "</b>|<b>diagonal_x</b>|<b>dense1</b>|<b>dense2</b>|<b>dense3</b>|<b>dense4</b>|<b>dense5"
                  "</b>|<b>dense6</b>|<b>dense7</b>|<b>no]" );
      break;

    case CapStyle:
      mTypes = DataTypeString;
      mHelpText = trString() + QStringLiteral( "[<b>square</b>|<b>flat</b>|<b>round</b>]" );
      break;

    case HorizontalAnchor:
      mTypes = DataTypeString;
      mHelpText = trString() + QStringLiteral( "[<b>left</b>|<b>center</b>|<b>right</b>]" );
      break;

    case VerticalAnchor:
      mTypes = DataTypeString;
      mHelpText = trString() + QStringLiteral( "[<b>top</b>|<b>center</b>|<b>bottom</b>]" );
      break;

    case SvgPath:
      mTypes = DataTypeString;
      mHelpText = trString() + QStringLiteral( "[<b>filepath</b>] as<br>"
                  "<b>''</b>=empty|absolute|search-paths-relative|<br>"
                  "project-relative|URL" );
      break;

    case Offset:
      mTypes = DataTypeString;
      mHelpText = QObject::tr( "string of doubles '<b>x,y</b>' or array of doubles <b>[x, y]</b>" );
      break;

    case DateTime:
      mTypes = DataTypeString;
      mHelpText = QObject::tr( "DateTime or string representation of a DateTime" );
      break;

    case Custom:
      mTypes = DataTypeString;
  }
}

QgsPropertyDefinition::QgsPropertyDefinition( const QString &name, DataType dataType, const QString &description, const QString &helpText, const QString &origin, const QString &comment )
  : mName( name )
  , mDescription( description )
  , mTypes( dataType )
  , mHelpText( helpText )
  , mOrigin( origin )
  , mComment( comment )
{}

bool QgsPropertyDefinition::supportsAssistant() const
{
  return mTypes == DataTypeNumeric || mStandardType == Size || mStandardType == StrokeWidth || mStandardType == ColorNoAlpha || mStandardType == ColorWithAlpha
         || mStandardType == Rotation;
}

QString QgsPropertyDefinition::trString()
{
  // just something to reduce translation redundancy
  return QObject::tr( "string " );
}

//
// QgsProperty
//

QVariantMap QgsProperty::propertyMapToVariantMap( const QMap<QString, QgsProperty> &propertyMap )
{
  QVariantMap variantMap;
  QMap<QString, QgsProperty>::const_iterator it = propertyMap.constBegin();
  for ( ; it != propertyMap.constEnd(); ++it )
    variantMap.insert( it.key(), it.value().toVariant() );
  return variantMap;
}

QMap<QString, QgsProperty> QgsProperty::variantMapToPropertyMap( const QVariantMap &variantMap )
{
  QMap<QString, QgsProperty> propertyMap;
  QVariantMap::const_iterator it = variantMap.constBegin();
  for ( ; it != variantMap.constEnd(); ++it )
  {
    QgsProperty property;
    if ( property.loadVariant( it.value() ) )
      propertyMap.insert( it.key(), property );
  }
  return propertyMap;
}

QgsProperty::QgsProperty()
{
  d = new QgsPropertyPrivate();
}

QgsProperty::~QgsProperty() = default;

QgsProperty QgsProperty::fromExpression( const QString &expression, bool isActive )
{
  QgsProperty p;
  p.setExpressionString( expression );
  p.setActive( isActive );
  return p;
}

QgsProperty QgsProperty::fromField( const QString &fieldName, bool isActive )
{
  QgsProperty p;
  p.setField( fieldName );
  p.setActive( isActive );
  return p;
}

QgsProperty QgsProperty::fromValue( const QVariant &value, bool isActive )
{
  QgsProperty p;
  p.setStaticValue( value );
  p.setActive( isActive );
  return p;
}

QgsProperty::QgsProperty( const QgsProperty &other ) //NOLINT
  : d( other.d )
{}

QgsProperty &QgsProperty::operator=( const QgsProperty &other )  //NOLINT
{
  d = other.d;
  return *this;
}

bool QgsProperty::operator==( const QgsProperty &other ) const
{
  return d->active == other.d->active
         && d->type == other.d->type
         && ( d->type != StaticProperty || d->staticValue == other.d->staticValue )
         && ( d->type != FieldBasedProperty || d->fieldName == other.d->fieldName )
         && ( d->type != ExpressionBasedProperty || d->expressionString == other.d->expressionString )
         && ( ( !d->transformer && !other.d->transformer ) || ( d->transformer && other.d->transformer && d->transformer->toExpression( QString() ) == other.d->transformer->toExpression( QString() ) ) );
}

bool QgsProperty::operator!=( const QgsProperty &other ) const
{
  return ( !( ( *this ) == other ) );
}

QgsProperty::Type QgsProperty::propertyType() const
{
  return static_cast< Type >( d->type );
}

bool QgsProperty::isActive() const
{
  return d->type != InvalidProperty && d->active;
}

bool QgsProperty::isStaticValueInContext( const QgsExpressionContext &context, QVariant &staticValue ) const
{
  staticValue = QVariant();
  switch ( d->type )
  {
    case InvalidProperty:
      return true;

    case StaticProperty:
      staticValue = d->staticValue;
      return true;

    case FieldBasedProperty:
      return false;

    case ExpressionBasedProperty:
    {
      QgsExpression exp = d->expression;
      if ( exp.prepare( &context ) && exp.rootNode() )
      {
        if ( exp.rootNode()->hasCachedStaticValue() )
        {
          staticValue = exp.rootNode()->cachedStaticValue();
          return true;
        }
      }
      return false;
    }
  }
  return false;
}

void QgsProperty::setActive( bool active )
{
  d.detach();
  d->active = active;
}

void QgsProperty::setStaticValue( const QVariant &value )
{
  d.detach();
  d->type = StaticProperty;
  d->staticValue = value;
}

QVariant QgsProperty::staticValue() const
{
  if ( d->type != StaticProperty )
    return QVariant();

  return d->staticValue;
}

void QgsProperty::setField( const QString &field )
{
  d.detach();
  d->type = FieldBasedProperty;
  d->fieldName = field;
  d->cachedFieldIdx = -1;
}

QString QgsProperty::field() const
{
  if ( d->type != FieldBasedProperty )
    return QString();

  return d->fieldName;
}

QgsProperty::operator bool() const
{
  return d->type != InvalidProperty;
}

void QgsProperty::setExpressionString( const QString &expression )
{
  d.detach();
  d->expressionString = expression;
  d->expression = QgsExpression( expression );
  d->expressionPrepared = false;
  d->expressionIsInvalid = false;

  if ( d->expressionString.isEmpty() )
  {
    d->active = false;
    d->type = InvalidProperty;
  }
  else
  {
    d->type = ExpressionBasedProperty;
  }
}

QString QgsProperty::expressionString() const
{
  if ( d->type != ExpressionBasedProperty )
    return QString();

  return d->expressionString;
}


QString QgsProperty::asExpression() const
{
  QString exp;
  switch ( d->type )
  {
    case StaticProperty:
      exp = QgsExpression::quotedValue( d->staticValue );
      break;

    case FieldBasedProperty:
      exp = QgsExpression::quotedColumnRef( d->fieldName );
      break;

    case ExpressionBasedProperty:
      exp = d->expressionString;
      break;

    case InvalidProperty:
      exp = QString();
      break;
  }
  return d->transformer ? d->transformer->toExpression( exp ) : exp;
}

bool QgsProperty::prepare( const QgsExpressionContext &context ) const
{
  if ( !d->active )
    return true;

  switch ( d->type )
  {
    case StaticProperty:
      return true;

    case FieldBasedProperty:
    {
      d.detach();
      // cache field index to avoid subsequent lookups
      const QgsFields f = context.fields();
      d->cachedFieldIdx = f.lookupField( d->fieldName );
      return true;
    }

    case ExpressionBasedProperty:
    {
      d.detach();
      if ( !d->expression.prepare( &context ) )
      {
        d->expressionReferencedCols.clear();
        d->expressionPrepared = false;
        d->expressionIsInvalid = true;
        return false;
      }

      d->expressionPrepared = true;
      d->expressionIsInvalid = false;
      d->expressionReferencedCols = d->expression.referencedColumns();
      return true;
    }

    case InvalidProperty:
      return true;

  }

  return false;
}

QSet<QString> QgsProperty::referencedFields( const QgsExpressionContext &context, bool ignoreContext ) const
{
  if ( !d->active )
    return QSet<QString>();

  switch ( d->type )
  {
    case StaticProperty:
    case InvalidProperty:
      return QSet<QString>();

    case FieldBasedProperty:
    {
      QSet< QString > fields;
      if ( !d->fieldName.isEmpty() )
        fields.insert( d->fieldName );
      return fields;
    }

    case ExpressionBasedProperty:
    {
      if ( ignoreContext )
      {
        return d->expression.referencedColumns();
      }

      if ( d->expressionIsInvalid )
        return QSet< QString >();

      d.detach();
      if ( !d->expressionPrepared && !prepare( context ) )
      {
        d->expressionIsInvalid = true;
        return QSet< QString >();
      }

      return d->expressionReferencedCols;
    }

  }
  return QSet<QString>();
}

bool QgsProperty::isProjectColor() const
{
  const QRegularExpression rx( QStringLiteral( "^project_color\\('.*'\\)$" ) );
  return d->type == QgsProperty::ExpressionBasedProperty && !d->expressionString.isEmpty()
         && rx.match( d->expressionString ).hasMatch();
}

QVariant QgsProperty::propertyValue( const QgsExpressionContext &context, const QVariant &defaultValue, bool *ok ) const
{
  if ( ok )
    *ok = false;

  if ( !d->active )
    return defaultValue;

  switch ( d->type )
  {
    case StaticProperty:
    {
      if ( ok )
        *ok = true;
      return d->staticValue;
    }

    case FieldBasedProperty:
    {
      const QgsFeature f = context.feature();
      if ( !f.isValid() )
        return defaultValue;

      //shortcut the field lookup
      if ( d->cachedFieldIdx >= 0 )
      {
        if ( ok )
          *ok = true;
        return f.attribute( d->cachedFieldIdx );
      }
      prepare( context );
      if ( d->cachedFieldIdx < 0 )
        return defaultValue;

      if ( ok )
        *ok = true;
      return f.attribute( d->cachedFieldIdx );
    }

    case ExpressionBasedProperty:
    {
      if ( d->expressionIsInvalid )
        return defaultValue;

      if ( !d->expressionPrepared && !prepare( context ) )
        return defaultValue;

      QVariant result = d->expression.evaluate( &context );
      if ( !result.isNull() )
      {
        if ( ok )
          *ok = true;
        return result;
      }
      else
      {
        return defaultValue;
      }
    }

    case InvalidProperty:
      return defaultValue;

  }

  return QVariant();
}


QVariant QgsProperty::value( const QgsExpressionContext &context, const QVariant &defaultValue, bool *ok ) const
{
  if ( ok )
    *ok = false;

  bool valOk = false;
  QVariant val = propertyValue( context, defaultValue, &valOk );
  if ( !d->transformer && !valOk ) // if transformer present, let it handle null values
    return defaultValue;

  if ( d->transformer )
  {
    if ( !valOk )
      val = QVariant();
    val = d->transformer->transform( context, val );
  }

  if ( ok )
    *ok = true;

  return val;
}

QDateTime QgsProperty::valueAsDateTime( const QgsExpressionContext &context, const QDateTime &defaultDateTime, bool *ok ) const
{
  bool valOk = false;
  const QVariant val = value( context, defaultDateTime, &valOk );

  if ( !valOk || val.isNull() )
  {
    if ( ok )
      *ok = false;
    return defaultDateTime;
  }

  QDateTime dateTime;
  if ( val.type() == QVariant::DateTime )
  {
    dateTime = val.value<QDateTime>();
  }
  else
  {
    dateTime = val.toDateTime();
  }

  if ( !dateTime.isValid() )
    return defaultDateTime;
  else
  {
    if ( ok )
      *ok = true;
    return dateTime;
  }
}

QString QgsProperty::valueAsString( const QgsExpressionContext &context, const QString &defaultString, bool *ok ) const
{
  bool valOk = false;
  const QVariant val = value( context, defaultString, &valOk );

  if ( !valOk || val.isNull() )
  {
    if ( ok )
      *ok = false;
    return defaultString;
  }
  else
  {
    if ( ok )
      *ok = true;
    return val.toString();
  }
}

QColor QgsProperty::valueAsColor( const QgsExpressionContext &context, const QColor &defaultColor, bool *ok ) const
{
  if ( ok )
    *ok = false;

  bool valOk = false;
  const QVariant val = value( context, defaultColor, &valOk );

  if ( !valOk || val.isNull() )
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
  {
    if ( ok )
      *ok = true;
    return color;
  }
}

double QgsProperty::valueAsDouble( const QgsExpressionContext &context, double defaultValue, bool *ok ) const
{
  if ( ok )
    *ok = false;

  bool valOk = false;
  const QVariant val = value( context, defaultValue, &valOk );

  if ( !valOk || val.isNull() )
    return defaultValue;

  bool convertOk = false;
  const double dbl = val.toDouble( &convertOk );
  if ( !convertOk )
    return defaultValue;
  else
  {
    if ( ok )
      *ok = true;
    return dbl;
  }
}

int QgsProperty::valueAsInt( const QgsExpressionContext &context, int defaultValue, bool *ok ) const
{
  if ( ok )
    *ok = false;

  bool valOk = false;
  const QVariant val = value( context, defaultValue, &valOk );

  if ( !valOk || val.isNull() )
    return defaultValue;

  bool convertOk = false;
  const int integer = val.toInt( &convertOk );
  if ( !convertOk )
  {
    //one more option to try
    const double dbl = val.toDouble( &convertOk );
    if ( convertOk )
    {
      if ( ok )
        *ok = true;
      return std::round( dbl );
    }
    else
    {
      return defaultValue;
    }
  }
  else
  {
    if ( ok )
      *ok = true;
    return integer;
  }
}

bool QgsProperty::valueAsBool( const QgsExpressionContext &context, bool defaultValue, bool *ok ) const
{
  if ( ok )
    *ok = false;

  bool valOk = false;
  const QVariant val = value( context, defaultValue, &valOk );

  if ( !valOk || val.isNull() )
    return defaultValue;

  if ( ok )
    *ok = true;
  return val.toBool();
}

QVariant QgsProperty::toVariant() const
{
  QVariantMap propertyMap;

  propertyMap.insert( QStringLiteral( "active" ), d->active );
  propertyMap.insert( QStringLiteral( "type" ), d->type );

  switch ( d->type )
  {
    case StaticProperty:
      // propertyMap.insert( QStringLiteral( "valType" ), d->staticValue.typeName() );
      propertyMap.insert( QStringLiteral( "val" ), d->staticValue.toString() );
      break;

    case FieldBasedProperty:
      propertyMap.insert( QStringLiteral( "field" ), d->fieldName );
      break;

    case ExpressionBasedProperty:
      propertyMap.insert( QStringLiteral( "expression" ), d->expressionString );
      break;

    case InvalidProperty:
      break;
  }

  if ( d->transformer )
  {
    QVariantMap transformer;
    transformer.insert( QStringLiteral( "t" ), d->transformer->transformerType() );
    transformer.insert( QStringLiteral( "d" ), d->transformer->toVariant() );

    propertyMap.insert( QStringLiteral( "transformer" ), transformer );
  }

  return propertyMap;
}

bool QgsProperty::loadVariant( const QVariant &property )
{
  const QVariantMap propertyMap = property.toMap();

  d.detach();
  d->active = propertyMap.value( QStringLiteral( "active" ) ).toBool();
  d->type = static_cast< Type >( propertyMap.value( QStringLiteral( "type" ), InvalidProperty ).toInt() );

  switch ( d->type )
  {
    case StaticProperty:
      d->staticValue = propertyMap.value( QStringLiteral( "val" ) );
      // d->staticValue.convert( QVariant::nameToType( propertyElem.attribute( "valType", "QString" ).toLocal8Bit().constData() ) );
      break;

    case FieldBasedProperty:
      d->fieldName = propertyMap.value( QStringLiteral( "field" ) ).toString();
      if ( d->fieldName.isEmpty() )
        d->active = false;
      break;

    case ExpressionBasedProperty:
      d->expressionString = propertyMap.value( QStringLiteral( "expression" ) ).toString();
      if ( d->expressionString.isEmpty() )
        d->active = false;

      d->expression = QgsExpression( d->expressionString );
      d->expressionPrepared = false;
      d->expressionIsInvalid = false;
      d->expressionReferencedCols.clear();
      break;

    case InvalidProperty:
      break;

  }

  //restore transformer if present
  delete d->transformer;
  d->transformer = nullptr;


  const QVariant transform = propertyMap.value( QStringLiteral( "transformer" ) );

  if ( transform.isValid() )
  {
    const QVariantMap transformerMap = transform.toMap();

    const QgsPropertyTransformer::Type type = static_cast< QgsPropertyTransformer::Type >( transformerMap.value( QStringLiteral( "t" ), QgsPropertyTransformer::GenericNumericTransformer ).toInt() );
    std::unique_ptr< QgsPropertyTransformer > transformer( QgsPropertyTransformer::create( type ) );

    if ( transformer )
    {
      if ( transformer->loadVariant( transformerMap.value( QStringLiteral( "d" ) ) ) )
        d->transformer = transformer.release();
    }
  }

  return true;
}


void QgsProperty::setTransformer( QgsPropertyTransformer *transformer )
{
  d.detach();
  d->transformer = transformer;
}

const QgsPropertyTransformer *QgsProperty::transformer() const
{
  return d->transformer;
}

bool QgsProperty::convertToTransformer()
{
  if ( d->type != ExpressionBasedProperty )
    return false;

  if ( d->transformer )
    return false; // already a transformer

  QString baseExpression;
  QString fieldName;
  std::unique_ptr< QgsPropertyTransformer > transformer( QgsPropertyTransformer::fromExpression( d->expressionString, baseExpression, fieldName ) );
  if ( !transformer )
    return false;

  d.detach();
  d->transformer = transformer.release();
  if ( !fieldName.isEmpty() )
    setField( fieldName );
  else
    setExpressionString( baseExpression );
  return true;
}



