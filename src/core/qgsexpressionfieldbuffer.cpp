/***************************************************************************
                          qgsexpressionfieldbuffer.cpp
                          ---------------------------
    begin                : May 27, 2014
    copyright            : (C) 2014 by Matthias Kuhn
    email                : matthias at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsexpressionfieldbuffer.h"

#include "qgsvectorlayer.h"


void QgsExpressionFieldBuffer::addExpression( const QString &exp, const QgsField &fld )
{
  mExpressions << ExpressionField( exp, fld );
}

void QgsExpressionFieldBuffer::removeExpression( int index )
{
  mExpressions.removeAt( index );
}

void QgsExpressionFieldBuffer::renameExpression( int index, const QString &name )
{
  mExpressions[index].field.setName( name );
}

void QgsExpressionFieldBuffer::updateExpression( int index, const QString &exp )
{
  mExpressions[index].cachedExpression = QgsExpression( exp );
}

void QgsExpressionFieldBuffer::writeXml( QDomNode &layerNode, QDomDocument &document ) const
{
  QDomElement expressionFieldsElem = document.createElement( QStringLiteral( "expressionfields" ) );
  layerNode.appendChild( expressionFieldsElem );

  Q_FOREACH ( const ExpressionField &fld, mExpressions )
  {
    QDomElement fldElem = document.createElement( QStringLiteral( "field" ) );

    fldElem.setAttribute( QStringLiteral( "expression" ), fld.cachedExpression.expression() );
    fldElem.setAttribute( QStringLiteral( "name" ), fld.field.name() );
    fldElem.setAttribute( QStringLiteral( "precision" ), fld.field.precision() );
    fldElem.setAttribute( QStringLiteral( "comment" ), fld.field.comment() );
    fldElem.setAttribute( QStringLiteral( "length" ), fld.field.length() );
    fldElem.setAttribute( QStringLiteral( "type" ), fld.field.type() );
    fldElem.setAttribute( QStringLiteral( "subType" ), fld.field.subType() );
    fldElem.setAttribute( QStringLiteral( "typeName" ), fld.field.typeName() );

    expressionFieldsElem.appendChild( fldElem );
  }
}

void QgsExpressionFieldBuffer::readXml( const QDomNode &layerNode )
{
  mExpressions.clear();

  const QDomElement expressionFieldsElem = layerNode.firstChildElement( QStringLiteral( "expressionfields" ) );

  if ( !expressionFieldsElem.isNull() )
  {
    QDomNodeList fields = expressionFieldsElem.elementsByTagName( QStringLiteral( "field" ) );

    for ( int i = 0; i < fields.size(); ++i )
    {
      QDomElement field = fields.at( i ).toElement();
      QString exp = field.attribute( QStringLiteral( "expression" ) );
      QString name = field.attribute( QStringLiteral( "name" ) );
      QString comment = field.attribute( QStringLiteral( "comment" ) );
      int precision = field.attribute( QStringLiteral( "precision" ) ).toInt();
      int length = field.attribute( QStringLiteral( "length" ) ).toInt();
      QVariant::Type type = static_cast< QVariant::Type >( field.attribute( QStringLiteral( "type" ) ).toInt() );
      QVariant::Type subType = static_cast< QVariant::Type >( field.attribute( QStringLiteral( "subType" ), 0 ).toInt() );
      QString typeName = field.attribute( QStringLiteral( "typeName" ) );

      mExpressions.append( ExpressionField( exp, QgsField( name, type, typeName, length, precision, comment, subType ) ) );
    }
  }
}

void QgsExpressionFieldBuffer::updateFields( QgsFields &flds )
{
  int index = 0;
  Q_FOREACH ( const ExpressionField &fld, mExpressions )
  {
    flds.appendExpressionField( fld.field, index );
    ++index;
  }
}
