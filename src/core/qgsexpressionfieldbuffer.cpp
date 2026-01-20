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
  QDomElement expressionFieldsElem = document.createElement( u"expressionfields"_s );
  layerNode.appendChild( expressionFieldsElem );

  const auto constMExpressions = mExpressions;
  for ( const ExpressionField &fld : constMExpressions )
  {
    QDomElement fldElem = document.createElement( u"field"_s );

    fldElem.setAttribute( u"expression"_s, fld.cachedExpression.expression() );
    fldElem.setAttribute( u"name"_s, fld.field.name() );
    fldElem.setAttribute( u"precision"_s, fld.field.precision() );
    fldElem.setAttribute( u"comment"_s, fld.field.comment() );
    fldElem.setAttribute( u"length"_s, fld.field.length() );
    fldElem.setAttribute( u"type"_s, fld.field.type() );
    fldElem.setAttribute( u"subType"_s, fld.field.subType() );
    fldElem.setAttribute( u"typeName"_s, fld.field.typeName() );

    expressionFieldsElem.appendChild( fldElem );
  }
}

void QgsExpressionFieldBuffer::readXml( const QDomNode &layerNode )
{
  mExpressions.clear();

  const QDomElement expressionFieldsElem = layerNode.firstChildElement( u"expressionfields"_s );

  if ( !expressionFieldsElem.isNull() )
  {
    const QDomNodeList fields = expressionFieldsElem.elementsByTagName( u"field"_s );

    for ( int i = 0; i < fields.size(); ++i )
    {
      const QDomElement field = fields.at( i ).toElement();
      const QString exp = field.attribute( u"expression"_s );
      const QString name = field.attribute( u"name"_s );
      const QString comment = field.attribute( u"comment"_s );
      const int precision = field.attribute( u"precision"_s ).toInt();
      const int length = field.attribute( u"length"_s ).toInt();
      const QMetaType::Type type = static_cast< QMetaType::Type >( field.attribute( u"type"_s ).toInt() );
      const QMetaType::Type subType = static_cast< QMetaType::Type >( field.attribute( u"subType"_s, u"0"_s ).toInt() );
      const QString typeName = field.attribute( u"typeName"_s );

      mExpressions.append( ExpressionField( exp, QgsField( name, type, typeName, length, precision, comment, subType ) ) );
    }
  }
}

void QgsExpressionFieldBuffer::updateFields( QgsFields &flds ) const
{
  int index = 0;
  const auto constMExpressions = mExpressions;
  for ( const ExpressionField &fld : constMExpressions )
  {
    flds.appendExpressionField( fld.field, index );
    ++index;
  }
}
