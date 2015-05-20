/***************************************************************************
     qgsexpressioncontext.cpp
     ------------------------
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

#include "qgsexpressioncontext.h"

#include "qgslogger.h"
#include "qgsexpression.h"
#include "qgsfield.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include <QSettings>
#include <QDir>

//internal variable names, shouldn't be overwritten
QStringList QgsExpressionContext::mReservedVariableNames = QStringList()
    << "feature"
    << "fields";


QgsExpressionContext::~QgsExpressionContext()
{
  qDeleteAll( mFunctions );
}

bool QgsExpressionContext::hasFunction( const QString &name ) const
{
  return mFunctions.contains( name );
}

QgsExpression::Function *QgsExpressionContext::function( const QString &name ) const
{
  return mFunctions.contains( name ) ? mFunctions.value( name ) : 0;
}

//stack

QgsExpressionContextStack::~QgsExpressionContextStack()
{
}

void QgsExpressionContextStack::appendContext( QgsExpressionContext *context )
{
  mStack.append( context );
}

bool QgsExpressionContextStack::hasVariable( const QString &name ) const
{
  foreach ( QgsExpressionContext* context, mStack )
  {
    if ( context->hasVariable( name ) )
      return true;
  }
  return false;
}

QVariant QgsExpressionContextStack::variable( const QString &name ) const
{
  QgsExpressionContext* context = activeContextForVariable( name );
  return context ? context->variable( name ) : QVariant();
}

QgsExpressionContext *QgsExpressionContextStack::activeContextForVariable( const QString &name ) const
{
  //iterate through stack backwards, so that higher priority variables take precedence
  QList< QgsExpressionContext* >::const_iterator it = mStack.constEnd();
  while ( it != mStack.constBegin() )
  {
    --it;
    QgsExpressionContextStack* nextStack = dynamic_cast<QgsExpressionContextStack*>(( *it ) );
    if ( nextStack )
    {
      QgsExpressionContext* contextFromStack = nextStack->activeContextForVariable( name );
      if ( contextFromStack )
        return contextFromStack;
    }
    else if (( *it )->hasVariable( name ) )
      return ( *it );
  }
  return 0;
}

QStringList QgsExpressionContextStack::variableNames() const
{
  QStringList names;
  foreach ( QgsExpressionContext* context, mStack )
  {
    names << context->variableNames();
  }
  return names.toSet().toList();
}

bool QgsExpressionContextStack::hasFunction( const QString &name ) const
{
  foreach ( QgsExpressionContext* context, mStack )
  {
    if ( context->hasFunction( name ) )
      return true;
  }
  return false;
}

QgsExpression::Function *QgsExpressionContextStack::function( const QString &name ) const
{
  //iterate through stack backwards, so that higher priority variables take precedence
  QList< QgsExpressionContext* >::const_iterator it = mStack.constEnd();
  while ( it != mStack.constBegin() )
  {
    --it;
    if (( *it )->hasFunction( name ) )
      return ( *it )->function( name );
  }
  return 0;
}

int QgsExpressionContextStack::childCount() const
{
  int count = 0;
  foreach ( QgsExpressionContext* context, mStack )
  {
    QgsExpressionContextStack* nextStack = dynamic_cast<QgsExpressionContextStack*>( context );
    if ( nextStack )
    {
      count += nextStack->childCount();
    }
    else
    {
      count++;
    }
  }

  return count;
}

QgsExpressionContext *QgsExpressionContextStack::getChild( int index )
{
  foreach ( QgsExpressionContext* context, mStack )
  {
    QgsExpressionContextStack* nextStack = dynamic_cast<QgsExpressionContextStack*>( context );
    if ( nextStack )
    {
      int count = nextStack->childCount();
      if ( count < index )
      {
        return nextStack->getChild( index );
      }
      else
      {
        index -= count;
      }
    }
    else if ( index == 0 )
      return context;
    else
      index--;
  }
  return 0;
}


//
// QgsStaticValueExpressionContext
//

void QgsStaticValueExpressionContext::setVariable( const QString &name, const QVariant &value )
{
  if ( mVariables.contains( name ) )
  {
    StaticVariable existing = mVariables.value( name );
    existing.value = value;
    insertVariable( existing );
  }
  else
  {
    insertVariable( QgsStaticValueExpressionContext::StaticVariable( name, value ) );
  }
}

void QgsStaticValueExpressionContext::insertVariable( const QgsStaticValueExpressionContext::StaticVariable &variable )
{
  mVariables.insert( variable.name, variable );
}

void QgsStaticValueExpressionContext::removeVariable( const QString &name )
{
  mVariables.remove( name );
}

bool QgsStaticValueExpressionContext::hasVariable( const QString &name ) const
{
  return mRegisteredNames.contains( name ) || mVariables.contains( name );
}

QVariant QgsStaticValueExpressionContext::variable( const QString &name ) const
{
  return mVariables.value( name ).value;
}

QStringList QgsStaticValueExpressionContext::variableNames() const
{
  QStringList names = mVariables.keys();
  if ( !mRegisteredNames.isEmpty() )
    names.append( mRegisteredNames );
  return names;
}

bool QgsStaticValueExpressionContext::isReadOnly( const QString &name ) const
{
  return mRegisteredNames.contains( name ) || mVariables.value( name ).readOnly;
}



//
// QgsGlobalExpressionContext
//

QgsGlobalExpressionContext::QgsGlobalExpressionContext()
    : QgsStaticValueExpressionContext( QT_TR_NOOP( "QGIS" ) )
{
  //read values from QSettings
  QSettings settings;

  //check if settings contains any variables
  if ( settings.contains( QString( "/variables/values" ) ) )
  {
    QList< QVariant > customVariableVariants = settings.value( QString( "/variables/values" ) ).toList();
    QList< QVariant > customVariableNames = settings.value( QString( "/variables/names" ) ).toList();
    int variableIndex = 0;
    for ( QList< QVariant >::const_iterator it = customVariableVariants.constBegin();
          it != customVariableVariants.constEnd(); ++it )
    {
      if ( variableIndex >= customVariableNames.length() )
      {
        break;
      }

      QVariant value = ( *it );
      QString name = customVariableNames.at( variableIndex ).toString();

      setVariable( name, value );
      variableIndex++;
    }
  }

  //add some extra global variables
  insertVariable( QgsStaticValueExpressionContext::StaticVariable( "qgis_version", QGis::QGIS_VERSION, true ) );
  insertVariable( QgsStaticValueExpressionContext::StaticVariable( "qgis_version_no", QGis::QGIS_VERSION_INT, true ) );
  insertVariable( QgsStaticValueExpressionContext::StaticVariable( "qgis_release_name", QGis::QGIS_RELEASE_NAME, true ) );
}

QgsGlobalExpressionContext::~QgsGlobalExpressionContext()
{
  save();
}

QgsGlobalExpressionContext *QgsGlobalExpressionContext::instance()
{
  static QgsGlobalExpressionContext* sInstance( new QgsGlobalExpressionContext() );
  return sInstance;
}

void QgsGlobalExpressionContext::save()
{
  // save variables to settings
  QSettings settings;
  QList< QVariant > keyVariantList;
  QList< QString > keys = mVariables.keys();
  QList< QVariant > valueList;
  foreach ( QString key, keys )
  {
    if ( isReadOnly( key ) )
    {
      //only save user set (non readonly) variables
      continue;
    }
    keyVariantList << QVariant( key );
    valueList << mVariables.value( key ).value;
  }
  settings.setValue( QString( "/variables/names" ), keyVariantList );
  settings.setValue( QString( "/variables/values" ), valueList );
}

QgsProjectExpressionContext::QgsProjectExpressionContext( QgsProject *project )
    : QgsStaticValueExpressionContext( QT_TR_NOOP( "Project" ) )
    , mProject( project ? project : QgsProject::instance() )
{
  registerVariableNames( QStringList() <<  "project_title" << "project_path" << "project_folder" << "project_filename" );
  load();
}

void QgsProjectExpressionContext::load()
{
  QStringList variableNames = mProject->readListEntry( "Variables", "/variableNames" );
  QStringList variableValues = mProject->readListEntry( "Variables", "/variableValues" );

  int varIndex = 0;
  foreach ( QString variableName, variableNames )
  {
    if ( varIndex >= variableValues.length() )
    {
      break;
    }

    QString varValueString = variableValues.at( varIndex );
    varIndex++;
    QgsStaticValueExpressionContext::setVariable( variableName, varValueString );
  }
}


void QgsProjectExpressionContext::insertVariable( const StaticVariable &value )
{
  QgsStaticValueExpressionContext::insertVariable( value );
  //also write to project

  QStringList variableNames = mVariables.keys();
  QStringList variableValues;
  foreach ( QString variableName, variableNames )
  {
    variableValues << mVariables.value( variableName ).value.toString();
  }
  mProject->writeEntry( "Variables", "/variableNames", variableNames );
  mProject->writeEntry( "Variables", "/variableValues", variableValues );
}

QVariant QgsProjectExpressionContext::variable( const QString &name ) const
{
  if ( name == "project_title" )
    return mProject->title();
  else if ( name == "project_path" )
    return mProject->fileInfo().filePath();
  else if ( name == "project_folder" )
    return mProject->fileInfo().dir().path();
  else if ( name == "project_filename" )
    return mProject->fileInfo().fileName();
  else
    return QgsStaticValueExpressionContext::variable( name );
}

void QgsProjectExpressionContext::clear()
{
  mVariables.clear();
}

