/***************************************************************************
    qgsvertexentry.cpp  - entry for vertex of vertextool
    ---------------------
    begin                : April 2009
    copyright            : (C) 2009 by Richard Kostecky
    email                : csf dot kostej at mail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "vertextool/qgsvertexentry.h"

#include "qgsguiutils.h"
#include "qgsmaplayer.h"
#include "qgsmapcanvas.h"

QgsVertexEntry::QgsVertexEntry( const QgsPoint &p, QgsVertexId vertexId )
  : mSelected( false )
  , mPoint( p )
  , mVertexId( vertexId )
{
}

QgsVertexEntry::~QgsVertexEntry()
{
}


void QgsVertexEntry::setSelected( bool selected )
{
  mSelected = selected;
}
