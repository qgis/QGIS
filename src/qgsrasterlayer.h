/***************************************************************************
                          gsrasterlayer.h  -  description
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

#ifndef QGSRASTERLAYER_H
#define QGSRASTERLAYER_H

#include "qgsmaplayer.h"

/**
  *@author Gary E.Sherman
  */

class QgsRasterLayer : public QgsMapLayer  {
public: 
	QgsRasterLayer();
	~QgsRasterLayer();
};

#endif
