/***************************************************************************
                         qgsprocessingparameters.cpp
                         --------------------------
    begin                : December 2016
    copyright            : (C) 2016 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingparameters.h"

#include <QRegularExpression>


//
// QgsProcessingParameter
//

QgsProcessingParameter::QgsProcessingParameter( const QString& name, const QString& description, const QVariant& defaultValue, bool optional )
    : mName( name )
    , mDescription( description )
    , mDefault( defaultValue )
    , mFlags( optional ? QgsProcessingParameter::FlagOptional : 0 )
{

}

void QgsProcessingParameter::setName( const QString& name )
{
  mName = name;
}


void QgsProcessingParameter::setDescription( const QString& description )
{
  mDescription = description;
}


bool QgsProcessingParameter::setDefaultValue( const QVariant& value )
{
  if ( !acceptsValue( value ) )
    return false;

  mDefault = parseValue( value );
  return true;
}

void QgsProcessingParameter::setFlags( const Flags& flags )
{
  mFlags = flags;
}

QString QgsProcessingParameter::valueAsCommandLineParameter( const QVariant& value ) const
{
  return value.toString();
}

QString QgsProcessingParameter::asScriptCode() const
{
  QString code = QStringLiteral( "##%1=" ).arg( mName );
  if ( mFlags && FlagOptional )
    code += QStringLiteral( "optional " );
  code += type() + ' ';
  code += mDefault.toString();
  return code;
}

//
// QgsProcessingParameter
//
QgsProcessingParameterBoolean::QgsProcessingParameterBoolean( const QString& name, const QString& description, const QVariant& defaultValue, bool optional )
    : QgsProcessingParameter( name, description, defaultValue, optional )
{}

bool QgsProcessingParameterBoolean::acceptsValue( const QVariant& value ) const
{
  if ( !value.isValid() && !( mFlags & FlagOptional ) )
    return false;

  return true;
}

QVariant QgsProcessingParameterBoolean::parseValue( const QVariant& value ) const
{
  return convertToBool( value );
}

QgsProcessingParameter* QgsProcessingParameterBoolean::createFromScriptCode( const QString& name, const QString& description, bool isOptional, const QString& definition )
{
  QVariant defaultVal = convertToBool( definition );
  return new QgsProcessingParameterBoolean( name, description, defaultVal, isOptional );
}

QVariant QgsProcessingParameterBoolean::convertToBool( const QVariant& value )
{
  if ( value.type() == QVariant::String )
    return value.toString().toLower().trimmed() == QStringLiteral( "true" );
  else
    return value.toBool();
}



