/***************************************************************************
                          gstable.h  -  description
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

#ifndef QGSTABLE_H
#define QGSTABLE_H

#include "qgsdatasource.h"

/*! \class QgsTable
 * \brief Class to represent an attribute table related
 * to a map layer of any type
 */

class QgsTable : public QgsDataSource  {
public: 
    //! Constructor
	QgsTable();
	//! Destructor
	~QgsTable();
};

#endif
