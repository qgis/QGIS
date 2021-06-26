/***************************************************************************
    Triangulation.cpp
    -----------------
    begin                : September 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/
#include "qgstriangulation.h"
#include "qgsfields.h"

QgsFields QgsTriangulation::triangulationFields()
{
  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "type" ), QVariant::String, QStringLiteral( "String" ) ) );
  return fields;
}
