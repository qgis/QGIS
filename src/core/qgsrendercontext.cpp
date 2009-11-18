/***************************************************************************
                              qgsrendercontext.cpp
                              --------------------
  begin                : March 16, 2008
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


#include "qgsrendercontext.h"

QgsRenderContext::QgsRenderContext()
  : mPainter( 0 ),
    mCoordTransform( 0 ),
    mDrawEditingInformation( false ),
    mForceVectorOutput( true ),
    mRenderingStopped( false ),
    mScaleFactor( 1.0 ),
    mRasterScaleFactor( 1.0 ),
    mLabelingEngine( NULL )
{

}

QgsRenderContext::~QgsRenderContext()
{
  delete mCoordTransform;
}

void QgsRenderContext::setCoordinateTransform( QgsCoordinateTransform* t )
{
  delete mCoordTransform;
  mCoordTransform = t;
}

