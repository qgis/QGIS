/***************************************************************************
                          gsdatabaselayer.h  -  description
                             -------------------
    begin                : Fri Jun 28 2002
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

#ifndef QGSDATABASELAYER_H
#define QGSDATABASELAYER_H
class QString;
class QgsRect;

#include "qgsmaplayer.h"

/*! \class QgsDatabaseLayer
 * \brief A map layer based on data stored in a relational database.
 *
 * At present Qgis supports PostGIS "layers" in PostgresQL. 
 */
class QgsDatabaseLayer:public QgsMapLayer
{
  Q_OBJECT public:
	/*! Constructor
	 * @param conninfo Pointer to the connection information required to
	 * connect to PostgresQL
	 *@param table Name of the table in the database that this layer
	 * represents
	 */
	  QgsDatabaseLayer(const char *conninfo = 0, QString table = QString::null);
	//! Destructor
	 ~QgsDatabaseLayer();
	//! Deprecated draw function
	virtual void draw(QPainter *, QgsRect *, int);
	//! Draws the layer using coordinate transformation
	virtual void draw(QPainter *, QgsRect *, QgsCoordinateTransform * cFx);
	//! Identify the feature found within the search rectangle
	void identify(QgsRect *);
	//! Display the attribute table
	void table();
	//! Get the name of the geometry column
	QString geometryColumnName();
	//! Get the name of the table containing the geometry column
	QString geometryTableName();
	//! Get the name of the database
	QString databaseName();
	//! Get the schema name
	QString schemaName();
  private:
	//! Calculates extent of the layer using SQL and PostGIS functions
	  QgsRect calculateExtent();
	//! Type geometry contained in the layer. This corresponds to one of the OGIS Simple geometry types
	QString type;
	//! WKB type
	int wkbType;
	//! Name of the database containing the layer (table)
	QString database;
	//! Name of the schema containing the layer (table)
	QString schema;
	//! Name of the table containing the features
	QString tableName;
	//! Name of the column in the table that contains the geometry for the features
	QString geometryColumn;
	//! Spatial reference id
	QString srid;
	//OGIS WKB types

	enum ENDIAN
	{
		NDR = 1,
		XDR = 0
	};
	//! Returns the endian type for the client
	int endian();
	/*! Endian type as string
	   * @return XDR or NDR
	 */
	QString endianString();
};

#endif
