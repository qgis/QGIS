/***************************************************************************
                          qgsstoredexpressionmanager.cpp

 These classes store and control the management and execution of actions
 associated with a particular Qgis layer. Actions are defined to be
 external programs that are run with user-specified inputs that can
 depend on the contents of layer attributes.

                             -------------------
    begin                : August 2019
    copyright            : (C) 2019 David Signer
    email                : david at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstoredexpressionmanager.h"

#include <QDomElement>

QUuid QgsStoredExpressionManager::addStoredExpression( const QString &name, const QString &expression, const QString &tag )
{
  Q_UNUSED( tag );

  QgsStoredExpression storedExpression; //( name, expression );

  storedExpression.name = name;
  storedExpression.expression = expression;

  mStoredExpressions.append( storedExpression );

  return storedExpression.id;
}

void QgsStoredExpressionManager::removeStoredExpression( const QUuid &id, const QString &tag )
{
  Q_UNUSED( tag );

  int i = 0;
  for ( const QgsStoredExpression &storedExpression : mStoredExpressions )
  {
    if ( storedExpression.id == id )
    {
      mStoredExpressions.removeAt( i );
      break;
    }
    ++i;
  }
}

void QgsStoredExpressionManager::addStoredExpressions( QList< QgsStoredExpression > storedExpressions, const QString &tag )
{
  Q_UNUSED( tag );

  mStoredExpressions.append( storedExpressions );
}

QList< QgsStoredExpression > QgsStoredExpressionManager::storedExpressions( const QString &tag )
{
  Q_UNUSED( tag );

  return mStoredExpressions;
}

void QgsStoredExpressionManager::clearStoredExpressions()
{
  mStoredExpressions.clear();
}

bool QgsStoredExpressionManager::writeXml( QDomNode &layer_node ) const
{
  QString elementName;

  switch ( mMode )
  {
    case FilterExpression:
    {
      elementName = QStringLiteral( "storedfilterexpression" );
      break;
    }
  }

  QDomElement aStoredExpressions = layer_node.ownerDocument().createElement( QStringLiteral( "%1s" ).arg( elementName ) );

  for ( const QgsStoredExpression &storedExpression : mStoredExpressions )
  {
    QDomElement aStoredExpression = layer_node.ownerDocument().createElement( elementName );
    aStoredExpression.setAttribute( QStringLiteral( "name" ), storedExpression.name );
    aStoredExpression.setAttribute( QStringLiteral( "expression" ), storedExpression.expression );
    aStoredExpressions.appendChild( aStoredExpression );
  }
  layer_node.appendChild( aStoredExpressions );

  return true;
}

bool QgsStoredExpressionManager::readXml( const QDomNode &layer_node )
{
  clearStoredExpressions();

  QString elementName;
  switch ( mMode )
  {
    case FilterExpression:
    {
      elementName = QStringLiteral( "storedfilterexpression" );
      break;
    }
  }

  QDomNode aaNode = layer_node.namedItem( QStringLiteral( "%1s" ).arg( elementName ) );

  if ( !aaNode.isNull() )
  {
    QDomNodeList aStoredExpressions = aaNode.toElement().elementsByTagName( elementName );
    for ( int i = 0; i < aStoredExpressions.size(); ++i )
    {
      QDomElement aStoredExpression = aStoredExpressions.at( i ).toElement();
      addStoredExpression( aStoredExpression.attribute( QStringLiteral( "name" ) ), aStoredExpression.attribute( QStringLiteral( "expression" ) ) );
    }
  }
  return true;
}
