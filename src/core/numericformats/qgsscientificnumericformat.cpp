/***************************************************************************
                             qgsscientificnumericformat.cpp
                             ----------------------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsscientificnumericformat.h"
#include "qgis.h"


QgsScientificNumericFormat::QgsScientificNumericFormat()
{
  mUseScientific = true;
}

QString QgsScientificNumericFormat::id() const
{
  return QStringLiteral( "scientific" );
}

QString QgsScientificNumericFormat::formatDouble( double value, const QgsNumericFormatContext &context ) const
{
  return QgsBasicNumericFormat::formatDouble( value, context );
}

QgsNumericFormat *QgsScientificNumericFormat::clone() const
{
  return create( configuration() );
}

QgsNumericFormat *QgsScientificNumericFormat::create( const QVariantMap &configuration ) const
{
  std::unique_ptr< QgsScientificNumericFormat > res = qgis::make_unique< QgsScientificNumericFormat >();
  res->setConfiguration( configuration );
  return res.release();
}

QVariantMap QgsScientificNumericFormat::configuration() const
{
  QVariantMap res = QgsBasicNumericFormat::configuration();
  return res;
}
