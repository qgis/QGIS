/***************************************************************************
                         qgsdiagramfactory.cpp  -  description
                         ---------------------
    begin                : November 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QDomNode>

#include "qgsdiagramfactory.h"
#include "qgsrendercontext.h"

QgsDiagramFactory::QgsDiagramFactory(): mSizeUnit( MM )
{

}

QgsDiagramFactory::~QgsDiagramFactory()
{

}

QgsDiagramFactory::SizeType QgsDiagramFactory::sizeType() const
{
  return QgsDiagramFactory::HEIGHT;
}

double QgsDiagramFactory::diagramSizeScaleFactor( const QgsRenderContext& context ) const
{
  if ( mSizeUnit == MM )
  {
    return context.scaleFactor();
  }
  else if ( mSizeUnit == MapUnits )
  {
    return 1 / context.mapToPixel().mapUnitsPerPixel(); //pixel based devices
  }

  return 1.0;
}

bool QgsDiagramFactory::writeSizeUnits( QDomElement& factoryElem, QDomDocument& doc ) const
{
  if ( factoryElem.isNull() )
  {
    return false;
  }

  if ( mSizeUnit == MM )
  {
    factoryElem.setAttribute( "sizeUnits", "MM" );
  }
  else if ( mSizeUnit == MapUnits )
  {
    factoryElem.setAttribute( "sizeUnits", "MapUnits" );
  }
  return true;
}

bool QgsDiagramFactory::readSizeUnits( const QDomElement& factoryElem )
{
  if ( factoryElem.isNull() )
  {
    return false;
  }

  QString unitString = factoryElem.attribute( "sizeUnits" );
  if ( unitString == "MapUnits" )
  {
    mSizeUnit = MapUnits;
  }
  else
  {
    mSizeUnit = MM;
  }
  return true;
}
