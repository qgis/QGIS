/***************************************************************************
                          qgsgrassvectormaplayer.h
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

#ifndef QGSGRASSVECTORMAPLAYER_H
#define QGSGRASSVECTORMAPLAYER_H

#include <QDateTime>
#include <QMap>
#include <QObject>
#include <QPair>

#include "qgsfield.h"

class QgsGrassVectorMap;

class GRASS_LIB_EXPORT QgsGrassVectorMapLayer : public QObject
{
    Q_OBJECT
  public:
    QgsGrassVectorMapLayer( QgsGrassVectorMap *map, int field );

    int field() const { return mField; }
    bool isValid() const { return mValid; }
    QgsGrassVectorMap *map() { return mMap; }
    QgsFields & fields() { return mFields; }
    QMap<int, QList<QVariant>> & attributes() { return mAttributes; }
    bool hasTable() { return mHasTable; }
    int keyColumn() { return mKeyColumn; }
    QList<QPair<double, double>> minMax() { return mMinMax; }
    int userCount() { return mUsers; }
    void addUser();
    void removeUser();

    /** Load attributes from the map. Old sources are released. */
    void load();

    /** Clear all cached data */
    void clear();

    /** Decrease number of users and clear if no more users */
    void close();

  private:

    int mField;
    bool mValid;
    QgsGrassVectorMap *mMap;
    struct field_info *mFieldInfo;
    bool mHasTable;
    // index of key column
    int mKeyColumn;
    QgsFields mFields;
    // Map of attributes with cat as key
    QMap<int, QList<QVariant>> mAttributes;
    // minimum and maximum values of attributes
    QList<QPair<double, double>> mMinMax;
    // timestamp when attributes were loaded
    QDateTime mLastLoaded;
    // number of instances using this layer
    int mUsers;
};

#endif // QGSGRASSVECTORMAPLAYER_H
