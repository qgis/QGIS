/***************************************************************************
                          qgsmaplayer.cpp  -  description
                             -------------------
    begin                : Fri Jun 28 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman@mrcc.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaplayer.h"

QgsMapLayer::QgsMapLayer(int type, QString lyrname ) 
  : QgsDataSource(), layerType(type), layerName(lyrname)
{
}
QgsMapLayer::~QgsMapLayer(){
} 
const int QgsMapLayer::type(){
  return layerType;
}
/** Write property of QString layerName. */
void QgsMapLayer::setlayerName( const QString& _newVal){
  layerName = _newVal;
}
/** Read property of QString layerName. */
const QString QgsMapLayer::name(){
  return layerName;
}

