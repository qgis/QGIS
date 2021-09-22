/***************************************************************************
  qgsserverquerystringparameter.cpp - QgsServerQueryStringParameter

 ---------------------
 begin                : 10.7.2019
 copyright            : (C) 2019 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsserverquerystringparameter.h"
#include "qgsserverrequest.h"
#include "qgsserverexception.h"
#include "nlohmann/json.hpp"

QgsServerQueryStringParameter::QgsServerQueryStringParameter( const QString name,
    bool required,
    QgsServerQueryStringParameter::Type type,
    const QString &description,
    const QVariant &defaultValue ):
  mName( name ),
  mRequired( required ),
  mType( type ),
  mDescription( description ),
  mDefaultValue( defaultValue )
{
}

QgsServerQueryStringParameter::~QgsServerQueryStringParameter()
{

}

QVariant QgsServerQueryStringParameter::value( const QgsServerApiContext &context ) const
{

  // 1: check required
  if ( mRequired && !QUrlQuery( context.request()->url() ).hasQueryItem( mName ) )
  {
    throw QgsServerApiBadRequestException( QStringLiteral( "Missing required argument: '%1'" ).arg( mName ) );
  }

  // 2: get value from query string or set it to the default
  QVariant value;
  if ( QUrlQuery( context.request()->url() ).hasQueryItem( mName ) )
  {
    value = QUrlQuery( context.request()->url() ).queryItemValue( mName, QUrl::FullyDecoded );
  }
  else if ( mDefaultValue.isValid() )
  {
    value = mDefaultValue;
  }

  if ( value.isValid() )
  {

    // 3: check type
    const QVariant::Type targetType { static_cast< QVariant::Type  >( mType )};
    // Handle csv list type
    if ( mType == Type::List )
    {
      value = value.toString().split( ',' );
    }
    if ( value.type() != targetType )
    {
      bool ok = false;
      if ( value.canConvert( static_cast<int>( targetType ) ) )
      {
        ok = true;
        switch ( mType )
        {
          case Type::String:
            value = value.toString( );
            break;
          case Type::Boolean:
            value = value.toBool( );
            break;
          case Type::Double:
            value = value.toDouble( &ok );
            break;
          case Type::Integer:
            value = value.toLongLong( &ok );
            break;
          case Type::List:
            // already converted to a string list
            break;
        }
      }

      if ( ! ok )
      {
        throw  QgsServerApiBadRequestException( QStringLiteral( "Argument '%1' could not be converted to %2" ).arg( mName, typeName( mType ) ) );
      }
    }

    // 4: check custom validation
    if ( mCustomValidator && ! mCustomValidator( context, value ) )
    {
      throw  QgsServerApiBadRequestException( QStringLiteral( "Argument '%1' is not valid. %2" ).arg( name(), description() ) );
    }
  }
  return value;
}

void QgsServerQueryStringParameter::setCustomValidator( const customValidator &customValidator )
{
  mCustomValidator = customValidator;
}

json QgsServerQueryStringParameter::data() const
{
  const auto nameString { name().toStdString() };
  auto dataType { typeName( mType ).toLower().toStdString() };
  // Map list to string because it will be serialized
  if ( dataType == "list" )
  {
    dataType = "string";
  }
  else if ( dataType == "double" )
  {
    dataType = "number";
  }
  return
  {
    { "name", nameString },
    { "description", description().toStdString() },
    { "required", mRequired },
    { "in", "query"},
    { "style", "form"},
    { "explode", false },
    { "schema", {{ "type", dataType }}},
    // This is unfortunately not in OAS: { "default", mDefaultValue.toString().toStdString() }
  };
}

QString QgsServerQueryStringParameter::description() const
{
  return mDescription;
}

QString QgsServerQueryStringParameter::typeName( const QgsServerQueryStringParameter::Type type )
{
  static const QMetaEnum metaEnum = QMetaEnum::fromType<Type>();
  return metaEnum.valueToKey( static_cast<int>( type ) );
}

QString QgsServerQueryStringParameter::name() const
{
  return mName;
}

void QgsServerQueryStringParameter::setDescription( const QString &description )
{
  mDescription = description;
}
