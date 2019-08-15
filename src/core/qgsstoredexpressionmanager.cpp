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

QgsStoredExpressionManager::QgsStoredExpressionManager( QObject *parent ) : QObject( parent )
{

}

void QgsStoredExpressionManager::addStoredExpression( const QString &name, const QString &expression, const QString &tag )
{
  Q_UNUSED( tag );

  QPair<QString, QString> storedExpression( name, expression );

  mStoredExpressions.append( storedExpression );
}

void QgsStoredExpressionManager::addStoredExpressions( QList<QPair<QString, QString> > namedexpressions, const QString &tag )
{
  Q_UNUSED( tag );

  mStoredExpressions.append( namedexpressions );
}

QList<QPair<QString, QString> > QgsStoredExpressionManager::getStoredExpressions( const QString &tag )
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
  QDomElement aStoredExpressions = layer_node.ownerDocument().createElement( QStringLiteral( "storedexpressions" ) );

  for ( const QPair<QString, QString> &storedExpression : mStoredExpressions )
  {
    QDomElement aStoredExpression = layer_node.ownerDocument().createElement( QStringLiteral( "storedexpression" ) );
    aStoredExpression.setAttribute( QStringLiteral( "name" ), storedExpression.first );
    aStoredExpression.setAttribute( QStringLiteral( "expression" ), storedExpression.second );
    aStoredExpressions.appendChild( aStoredExpression );
  }
  layer_node.appendChild( aStoredExpressions );

  return true;
}

bool QgsStoredExpressionManager::readXml( const QDomNode &layer_node )
{
  clearStoredExpressions();

  QDomNode aaNode = layer_node.namedItem( QStringLiteral( "storedexpressions" ) );

  if ( !aaNode.isNull() )
  {
    QDomNodeList aStoredExpressions = aaNode.toElement().elementsByTagName( QStringLiteral( "storedexpression" ) );
    for ( int i = 0; i < aStoredExpressions.size(); ++i )
    {
      QDomElement aStoredExpression = aStoredExpressions.at( i ).toElement();
      addStoredExpression( aStoredExpression.attribute( QStringLiteral( "name" ) ), aStoredExpression.attribute( QStringLiteral( "expression" ) ) );
    }
  }
  return true;
}
