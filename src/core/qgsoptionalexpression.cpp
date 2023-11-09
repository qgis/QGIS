/***************************************************************************
  qgsoptionalexpression - QgsOptionalExpression

 ---------------------
 begin                : 14.9.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsoptionalexpression.h"

QgsOptionalExpression::QgsOptionalExpression( const QgsExpression &expression )
  : QgsOptional<QgsExpression>( expression )
{

}

QgsOptionalExpression::QgsOptionalExpression( const QgsExpression &expression, bool enabled )
  : QgsOptional<QgsExpression>( expression, enabled )
{

}

void QgsOptionalExpression::writeXml( QDomElement &element ) const
{
  const QDomText exp = element.ownerDocument().createTextNode( data().expression() );
  element.setAttribute( QStringLiteral( "enabled" ), enabled() );
  element.appendChild( exp );
}

void QgsOptionalExpression::readXml( const QDomElement &element )
{
  setEnabled( element.attribute( QStringLiteral( "enabled" ) ).toInt() );
  setData( element.text() );
}
