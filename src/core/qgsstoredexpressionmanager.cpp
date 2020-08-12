/***************************************************************************
                          qgsstoredexpressionmanager.cpp
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
#include "qgis.h"


#include <QDomElement>

QString QgsStoredExpressionManager::addStoredExpression( const QString &name, const QString &expression, const QgsStoredExpression::Category &tag )
{
  QgsStoredExpression storedExpression( name, expression, tag );

  mStoredExpressions.append( storedExpression );

  return storedExpression.id;
}

void QgsStoredExpressionManager::removeStoredExpression( const QString &id )
{
  int i = 0;
  for ( const QgsStoredExpression &storedExpression : qgis::as_const( mStoredExpressions ) )
  {
    if ( storedExpression.id == id )
    {
      mStoredExpressions.removeAt( i );
      //leave the loop after editing source
      break;
    }
    ++i;
  }
}

void QgsStoredExpressionManager::updateStoredExpression( const QString &id, const QString &name, const QString &expression, const QgsStoredExpression::Category &tag )
{
  int i = 0;
  for ( const QgsStoredExpression &storedExpression : qgis::as_const( mStoredExpressions ) )
  {
    if ( storedExpression.id == id )
    {
      QgsStoredExpression newStoredExpression = storedExpression;
      newStoredExpression.name = name;
      newStoredExpression.expression = expression;
      newStoredExpression.tag = tag;
      mStoredExpressions.replace( i, newStoredExpression );
      //leave the loop after editing source
      break;
    }
    ++i;
  }
}

void QgsStoredExpressionManager::addStoredExpressions( const QList< QgsStoredExpression > &storedExpressions )
{
  mStoredExpressions.append( storedExpressions );
}

QList< QgsStoredExpression > QgsStoredExpressionManager::storedExpressions( const QgsStoredExpression::Category &tag )
{
  QList< QgsStoredExpression > storedExpressions;

  for ( const QgsStoredExpression &storedExpression : qgis::as_const( mStoredExpressions ) )
  {
    if ( storedExpression.tag & tag )
    {
      storedExpressions.append( storedExpression );
    }
  }
  return storedExpressions;
}

QgsStoredExpression QgsStoredExpressionManager::storedExpression( const QString &id ) const
{
  for ( const QgsStoredExpression &storedExpression : qgis::as_const( mStoredExpressions ) )
  {
    if ( storedExpression.id == id )
    {
      return storedExpression;
    }
  }
  return QgsStoredExpression();
}

QgsStoredExpression QgsStoredExpressionManager::findStoredExpressionByExpression( const QString &expression, const QgsStoredExpression::Category &tag ) const
{
  for ( const QgsStoredExpression &storedExpression : qgis::as_const( mStoredExpressions ) )
  {
    if ( storedExpression.expression == expression && storedExpression.tag & tag )
    {
      return storedExpression;
    }
  }
  return QgsStoredExpression();
}

void QgsStoredExpressionManager::clearStoredExpressions()
{
  mStoredExpressions.clear();
}

bool QgsStoredExpressionManager::writeXml( QDomNode &layerNode ) const
{
  QDomElement aStoredExpressions = layerNode.ownerDocument().createElement( QStringLiteral( "storedexpressions" ) );

  for ( const QgsStoredExpression &storedExpression : qgis::as_const( mStoredExpressions ) )
  {
    QDomElement aStoredExpression = layerNode.ownerDocument().createElement( QStringLiteral( "storedexpression" ) );
    aStoredExpression.setAttribute( QStringLiteral( "name" ), storedExpression.name );
    aStoredExpression.setAttribute( QStringLiteral( "expression" ), storedExpression.expression );
    aStoredExpression.setAttribute( QStringLiteral( "tag" ), storedExpression.tag );
    aStoredExpressions.appendChild( aStoredExpression );
  }
  layerNode.appendChild( aStoredExpressions );

  return true;
}

bool QgsStoredExpressionManager::readXml( const QDomNode &layerNode )
{
  clearStoredExpressions();

  QDomNode aaNode = layerNode.namedItem( QStringLiteral( "storedexpressions" ) );

  if ( !aaNode.isNull() )
  {
    QDomNodeList aStoredExpressions = aaNode.toElement().elementsByTagName( QStringLiteral( "storedexpression" ) );
    for ( int i = 0; i < aStoredExpressions.size(); ++i )
    {
      QDomElement aStoredExpression = aStoredExpressions.at( i ).toElement();
      addStoredExpression( aStoredExpression.attribute( QStringLiteral( "name" ) ), aStoredExpression.attribute( QStringLiteral( "expression" ) ), QgsStoredExpression::Category( aStoredExpression.attribute( QStringLiteral( "tag" ) ).toInt() ) );
    }
  }
  return true;
}
