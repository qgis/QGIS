/***************************************************************************
                          qgsdlgbuffer.h
                          Buffer dialog - Subclasses qgsdlgbufferbase
 Part of the Geoprocessing plugin for PostgreSQL/PostGIS layers
                             -------------------
    begin                : Jan 21, 2004
    copyright            : (C) 2004 by Gary E.Sherman
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
 /*  $Id$ */
#ifndef QGSDLGPGBUFFER_H
#define QGSDLGPGBUFFER_H
#include "qgsdlgpgbufferbase.h"
class QgsDlgPgBuffer:public QgsDlgPgBufferBase
{
  Q_OBJECT public:
  //! Constructor
    QgsDlgPgBuffer(QWidget * parent = 0, const char *name = 0);
    //! Destructor
    QgsDlgPgBuffer::~QgsDlgPgBuffer();
    //! Set the information label in the dialog
    void setBufferLabel(QString & lbl);
    //! Get the buffer distance
    QString bufferDistance();
    //! Get the name of the buffered layer to be created
    QString bufferLayerName();
    //! Get the name of the column to be used as the object id (unique key)
    QString objectIdColumn();
    //! Get the name to use for the geometry column in the buffered layer
    QString geometryColumn();
    //! Get the srid to use when creating the buffered layer
    QString srid();
    //! Get the schema name for the new layer
    QString schema();
    //! Flag to indicate if the layer should be added to the map
    bool addLayerToMap();
    //! Add a field name to the list of fields available for use as the object id
    void addFieldItem(QString field);
    //! Add a schema name to the list of available schemas
    void addSchema(QString schema);
    //! Set the srid value on the dialog
    void setSrid(QString srid);
    //! Set the bufferd layer name on the dialog
    void setBufferLayerName(QString name);
    //! Set the geometry column on the dialog
    void setGeometryColumn(QString name);
};
#endif // QGSDLGPGBUFFER_H
