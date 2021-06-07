/***************************************************************************
  qgsgeocodercontext.cpp
  ---------------
  Date                 : August 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeocodercontext.h"

//
// QgsGeocoderContext
//

QgsGeocoderContext::QgsGeocoderContext( const QgsCoordinateTransformContext &transformContext )
  : mTransformContext( transformContext )
{}
