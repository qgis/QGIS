/***************************************************************************
                          gsdatabaselayer.h  -  description
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

#ifndef QGSDATABASELAYER_H
#define QGSDATABASELAYER_H
class QString;
#include "qgsmaplayer.h"

/**
  *@author Gary E.Sherman
  */

class QgsDatabaseLayer : public QgsMapLayer  {
public: 
	QgsDatabaseLayer(const char *conninfo=0, QString table=QString::null);
	~QgsDatabaseLayer();
 private:
	void calculateExtent();
	QString type; // maps to one of the OGIS Simple geometry types
	QString database;
	QString tableName;
	QString geometryColumn;
	
	
};

#endif
