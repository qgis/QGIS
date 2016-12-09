/***************************************************************************
                         qgsprocessingparameterregistry.cpp
                         ----------------------------------
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

#include "qgsprocessingparameterregistry.h"
#include "qgsprocessingparameters.h"
#include <QRegularExpression>


QgsProcessingParameterRegistry::QgsProcessingParameterRegistry()
{
  addParameterType( new QgsProcessingParameterMetadata( QStringLiteral( "boolean" ), QgsProcessingParameterBoolean::createFromScriptCode, nullptr ) );
}

QgsProcessingParameterRegistry::~QgsProcessingParameterRegistry()
{
  qDeleteAll( mMetadata );
}

bool QgsProcessingParameterRegistry::parseScriptCodeParameterOptions( const QString& code, bool& isOptional, QString& name, QString& type, QString& definition )
{
  QRegularExpression re( "(.*?)=\\s*(.*)" );
  QRegularExpressionMatch m = re.match( code );
  if ( !m.hasMatch() )
    return false;

  name = m.captured( 1 );
  QString tokens = m.captured( 2 );
  if ( tokens.toLower().startsWith( QStringLiteral( "optional" ) ) )
  {
    isOptional = true;
    tokens.remove( 0, 8 ); // length "optional" = 8
  }
  else
  {
    isOptional = false;
  }

  QRegularExpression re2( "(.*?)\\s*(.*)" );
  m = re2.match( tokens );
  if ( !m.hasMatch() )
  {
    type = tokens;
    definition.clear();
  }
  else
  {
    type = m.captured( 1 );
    definition = m.captured( 2 );
  }
  return true;
}

QgsProcessingParameterAbstractMetadata*QgsProcessingParameterRegistry::parameterMetadata( const QString& type ) const
{
  return mMetadata.value( type );
}

bool QgsProcessingParameterRegistry::addParameterType( QgsProcessingParameterAbstractMetadata* metadata )
{
  if ( !metadata || mMetadata.contains( metadata->type() ) )
    return false;

  mMetadata[metadata->type()] = metadata;
  return true;
}

QgsProcessingParameter* QgsProcessingParameterRegistry::createFromScriptCode( const QString& code ) const
{
  bool isOptional = false;
  QString name;
  QString definition;
  QString type;
  if ( !parseScriptCodeParameterOptions( code, isOptional, name, type, definition ) )
    return nullptr;

  if ( !mMetadata.contains( type ) )
    return nullptr;

  QString description = createDescription( name );

  return mMetadata[type]->createParameterFromScriptCode( name, description, isOptional, definition );
}

QString QgsProcessingParameterRegistry::createDescription( const QString& name )
{
  QString desc = name;
  return desc.replace( '_', ' ' );
}

