/***************************************************************************
                          qgsmaplayer.h  -  description
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

#ifndef QGSMAPLAYER_H
#define QGSMAPLAYER_H

#include <qwidget.h>
#include "qgsdatasource.h"
/**
 *@author Gary E.Sherman
 */

class QgsMapLayer : public QgsDataSource  {

	public: 
    QgsMapLayer(int type=0, QString lyrname=QString::null );
    virtual ~QgsMapLayer();
  /** Read property of int layerType. */
  const int type();
  /** Write property of QString layerName. */
  void setlayerName( const QString& _newVal);
  /** Read property of QString layerName. */
  const QString name();


public: // Public attributes
enum LAYERS {
	VECTOR,
	RASTER,
DATABASE
}  ;

private: // Private attributes
  /** Name of the layer - used for display  */
  QString layerName;
  /** Type of the layer (eg. vector, raster, database  */
  int layerType;
  //! Position in the map stack 
  int zpos;
  //! Tag for embedding additional information
  QString tag;
 
};

#endif
