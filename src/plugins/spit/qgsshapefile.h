/***************************************************************************
                          qgsshapefile.h  -  description
                             -------------------
    begin                : Fri Dec 19 2003
    copyright            : (C) 2003 by Denis Antipov
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSHAPEFILE_H
#define QGSSHAPEFILE_H

#include <vector>
#include <QString>
#include <QStringList>
#include <QObject>
#include <ogr_api.h>

class QProgressDialog;
class QTextCodec;

class OGRLayer;
class OGRDataSource;
extern "C"
{
#include <libpq-fe.h>
}


class QgsShapeFile : public QObject
{
    Q_OBJECT
  public:

    QgsShapeFile( QString fileName, QString encoding = QString() );
    ~QgsShapeFile();
    int getFeatureCount();
    QString getFeatureClass();
    bool insertLayer( QString dbname, QString schema, QString primary_key, QString geom_col,
                      QString srid, PGconn * conn, QProgressDialog& pro,
                      bool &fin, QString& errorText );

    bool is_valid();
    QString getName();
    QString getTable();
    void setTable( QString new_table );
    void setDefaultTable();
    std::vector <QString> column_names;
    std::vector <QString> column_types;
    void setColumnNames( QStringList );
    bool scanGeometries();


  private:
    QString table_name;
    OGRDataSourceH ogrDataSource;
    OGRLayerH ogrLayer;
    bool import_canceled;
    bool valid;
    //! Flag to indicate the file contains multiple geometry types
    bool isMulti;
    bool hasMoreDimensions;
    int features;
    QString fileName;
    QString geom_type;
    QStringList geometries;
    QTextCodec* codec;

  public slots:
    void cancelImport();
};

#endif
