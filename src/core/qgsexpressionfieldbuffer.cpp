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

QgsExpressionFieldBuffer::QgsExpressionFieldBuffer()
{
}

void QgsExpressionFieldBuffer::addExpression( const QString& exp, const QgsField& fld )
{
  mExpressions << ExpressionField( exp, fld );
}

void QgsExpressionFieldBuffer::removeExpression( int index )
{
  mExpressions.removeAt( index );
}

void QgsExpressionFieldBuffer::updateExpression( int index, const QString& exp )
{
  mExpressions[index].expression = exp;
}

void QgsExpressionFieldBuffer::writeXml( QDomNode& layerNode, QDomDocument& document ) const
{
  QDomElement expressionFieldsElem = document.createElement( "expressionfields" );
  layerNode.appendChild( expressionFieldsElem );

  Q_FOREACH ( const ExpressionField& fld, mExpressions )
  {
    QDomElement fldElem = document.createElement( "field" );

    fldElem.setAttribute( "expression", fld.expression );
    fldElem.setAttribute( "name", fld.field.name() );
    fldElem.setAttribute( "precision", fld.field.precision() );
    fldElem.setAttribute( "comment", fld.field.comment() );
    fldElem.setAttribute( "length", fld.field.length() );
    fldElem.setAttribute( "type", fld.field.type() );
    fldElem.setAttribute( "typeName", fld.field.typeName() );

    expressionFieldsElem.appendChild( fldElem );
  }
}

void QgsExpressionFieldBuffer::readXml( const QDomNode& layerNode )
{
  mExpressions.clear();

  const QDomElement expressionFieldsElem = layerNode.firstChildElement( "expressionfields" );

  if ( !expressionFieldsElem.isNull() )
  {
    QDomNodeList fields = expressionFieldsElem.elementsByTagName( "field" );

    for ( unsigned int i = 0; i < fields.length(); ++i )
    {
      QDomElement field = fields.at( i ).toElement();
      QString exp = field.attribute( "expression" );
      QString name = field.attribute( "name" );
      QString comment = field.attribute( "comment" );
      int precision = field.attribute( "precision" ).toInt();
      int length = field.attribute( "length" ).toInt();
      QVariant::Type type = ( QVariant::Type )( field.attribute( "type" ).toInt() );
      QString typeName = field.attribute( "typeName" );

      mExpressions.append( ExpressionField( exp, QgsField( name, type, typeName, length, precision, comment ) ) );
    }
  }
}

void QgsExpressionFieldBuffer::updateFields( QgsFields& flds )
{
  int index = 0;
  Q_FOREACH ( const ExpressionField& fld, mExpressions )
  {
    flds.appendExpressionField( fld.field, index );
    ++index;
  }
}
