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


QgsServerQueryStringParameter::QgsServerQueryStringParameter( const QString name,
    bool required,
    QgsServerQueryStringParameter::Type type,
    const QString &description ):
  mName( name ),
  mRequired( required ),
  mType( type ),
  mDescription( description )
{

}

QgsServerQueryStringParameter::~QgsServerQueryStringParameter()
{

}

QVariant QgsServerQueryStringParameter::value( const QgsServerApiContext &context ) const
{
  // 1: check required
  if ( mRequired && ! context.request()->url().hasQueryItem( mName ) )
  {
    throw  QgsServerApiBadRequestError( QStringLiteral( "Missing required argument: %1" ).arg( mName ) );
  }

  // 2: check type
  QVariant value { context.request()->url().queryItemValue( mName ) };
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
        case Type::Bool:
          value = value.toBool( );
          break;
        case Type::Double:
          value = value.toDouble( &ok );
          break;
        case Type::Int:
          value = value.toLongLong( &ok );
          break;
        case Type::List:
          // already converted to a string list
          break;
      }
    }

    if ( ! ok )
    {
      throw  QgsServerApiBadRequestError( QStringLiteral( "Argument %1 could not be converted to %2" ).arg( mName )
                                          .arg( typeName( mType ) ) );
    }
  }

  // 3: check custom validation
  if ( mCustomValidator && ! mCustomValidator( context, value ) )
  {
    throw  QgsServerApiBadRequestError( QStringLiteral( "Custom validator failed to validate argument: %1" ).arg( mName ) );
  }
  return value;
}

void QgsServerQueryStringParameter::setCustomValidator( const customValidator &customValidator )
{
  mCustomValidator = customValidator;
}

QString QgsServerQueryStringParameter::description() const
{
  return mDescription;
}

QString QgsServerQueryStringParameter::typeName( const QgsServerQueryStringParameter::Type type )
{
  static QMetaEnum metaEnum = QMetaEnum::fromType<Type>();
  return metaEnum.valueToKey( static_cast<int>( type ) );
}

QString QgsServerQueryStringParameter::name() const
{
  return mName;
}
