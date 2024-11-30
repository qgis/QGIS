/***************************************************************************
                          qgsgrassvector.h
                             -------------------
    begin                : September, 2015
    copyright            : (C) 2015 by Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGRASSVECTOR_H
#define QGSGRASSVECTOR_H

#include <QDateTime>
#include <QObject>

#include "qgsfields.h"

#include "qgsgrass.h"

class GRASS_LIB_EXPORT QgsGrassVectorLayer : public QObject
{
    Q_OBJECT
  public:
    QgsGrassVectorLayer( QObject *parent = nullptr );

    QgsGrassVectorLayer( const QgsGrassObject &grassObject, int number, struct field_info *fieldInfo, QObject *parent = nullptr );

    QgsGrassObject grassObject() const { return mGrassObject; }

    //! Layer number (field)
    int number() { return mNumber; }

    //! Sets number of elements of given type.
    void setTypeCount( int type, int count ) { mTypeCounts[type] = count; }

    //! Gets number of elements of given type. Types may be combined by bitwise or)
    int typeCount( int type ) const;

    //! Gets all types in the layer (combined by bitwise or)
    int type() const;

    //! Gets all types in the layer as list
    QList<int> types() const;

    QgsFields fields();

    QString error() const { return mError; }

  private:
    QgsGrassObject mGrassObject;
    /* layer number */
    int mNumber = 0;
    /* optional name */
    QString mName;
    QString mDriver;
    QString mDatabase;
    QString mTable;
    /* key column */
    QString mKey;
    /* Numbers of vector types in the layer. Type/count pairs */
    QMap<int, int> mTypeCounts;
    QgsFields mFields;
    // timestamp of dbln file when fields were loaded
    QDateTime mFieldsTimeStamp;
    // Error when loading fields
    QString mError;
};

class GRASS_LIB_EXPORT QgsGrassVector : public QObject
{
    Q_OBJECT
  public:
    QgsGrassVector( const QString &gisdbase, const QString &location, const QString &mapset, const QString &name, QObject *parent = nullptr );

    QgsGrassVector( const QgsGrassObject &grassObject, QObject *parent = nullptr );

    //! Open header and read layers/types
    bool openHead();

    //! Gets list of layers. The layers exist until the vector is deleted or reloaded
    QList<QgsGrassVectorLayer *> layers() const { return mLayers; }

    /**
     * Gets numbers of primitives
     * \returns type/count pairs
    */
    QMap<int, int> typeCounts() const { return mTypeCounts; }

    //! Gets total number of primitives of given type. Types may be combined by bitwise or)
    int typeCount( int type ) const;

    /**
     * Maximum layer number (field).
     * \returns max layer number or 0 if no layer exists
    */
    int maxLayerNumber() const;

    //! Gets number of nodes
    int nodeCount() const { return mNodeCount; }

    //! Returns error message
    QString error() const { return mError; }

  signals:

  public slots:

  private:
    QgsGrassObject mGrassObject;
    QList<QgsGrassVectorLayer *> mLayers;
    QMap<int, int> mTypeCounts;
    int mNodeCount;
    QString mError;
};

#endif // QGSGRASSVECTOR_H
