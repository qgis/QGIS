/***************************************************************************
                          gsdatasource.h  -  description
                             -------------------
    begin                : Sun Jun 30 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id */

#ifndef QGSDATASOURCE_H
#define QGSDATASOURCE_H
#include <qstring.h>

/**Base class for spatial and tabular data
  *@author Gary E.Sherman
  */

class QgsDataSource {

public: 
	QgsDataSource();
	~QgsDataSource();
 protected:
 //! Path or uri of the datasource
 	QString dataSource;
};

#endif
