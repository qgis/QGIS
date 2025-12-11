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

#include "qgis_grass_lib.h"
#include "qgsfeature.h"
#include "qgsfields.h"

#include <QDateTime>
#include <QMap>
#include <QObject>
#include <QPair>

extern "C"
{
#include <grass/version.h>
#if defined( _MSC_VER ) && defined( M_PI_4 )
#undef M_PI_4 //avoid redefinition warning
#endif
#include <grass/gprojects.h>
#include <grass/gis.h>
#include <grass/dbmi.h>
#include <grass/vector.h>
}

class QgsGrassVectorMap;

class GRASS_LIB_EXPORT QgsGrassVectorMapLayer : public QObject
{
    Q_OBJECT
  public:
    QgsGrassVectorMapLayer( QgsGrassVectorMap *map, int field );

    [[nodiscard]] int field() const { return mField; }
    [[nodiscard]] bool isValid() const { return mValid; }
    [[nodiscard]] QgsGrassVectorMap *map() const { return mMap; }

    //! Category index index
    int cidxFieldIndex();

    //! Current number of cats in cat index, changing during editing
    int cidxFieldNumCats();

    /**
     * Original fields before editing started + topo field if edited.
     * Does not reflect add/delete column.
     * Original fields must be returned by provider fields()
    */
    [[nodiscard]] const QgsFields &fields() const { return mFields; }

    /**
     * Current fields, as modified during editing, it contains cat field, without topo field.
     * This fields are used by layers which are not editied to reflect current state of editing.
    */
    [[nodiscard]] const QgsFields &tableFields() const { return mTableFields; }

    static QStringList fieldNames( const QgsFields &fields );

    [[nodiscard]] const QMap<int, QList<QVariant>> &attributes() const { return mAttributes; }

    /**
     * Gets attribute for index corresponding to current fields(),
     * if there is no table, returns cat
    */
    QVariant attribute( int cat, int index );

    [[nodiscard]] bool hasTable() const { return mHasTable; }
    [[nodiscard]] int keyColumn() const { return mKeyColumn; }
    [[nodiscard]] QString keyColumnName() const { return mFieldInfo ? mFieldInfo->key : QString(); }
    [[nodiscard]] QList<QPair<double, double>> minMax() const { return mMinMax; }

    [[nodiscard]] int userCount() const { return mUsers; }
    void addUser();
    void removeUser();

    //! Load attributes from the map. Old sources are released.
    void load();

    //! Clear all cached data
    void clear();

    //! Decrease number of users and clear if no more users
    void close();

    void startEdit();
    void closeEdit();

    //------------------------------- Database utils ---------------------------------
    void setMapset();

    /**
     * Execute SQL statement
     * \param sql
    */
    void executeSql( const QString &sql, QString &error );

    /**
     * Update attributes
     *   \param cat
     *   \param index ields  index
     */
    void changeAttributeValue( int cat, const QgsField &field, const QVariant &value, QString &error );

    /**
     * Insert new attributes to the table (it does not check if attributes already exists)
     *   \param cat
     */
    void insertAttributes( int cat, const QgsFeature &feature, QString &error );

    /**
     * Restore previously deleted table record using data from mAttributes, if exists.
     *  If there the cat is not in mAttributes, nothing is inserted (to keep previous state).
     *   \param cat
    */
    void reinsertAttributes( int cat, QString &error );

    /**
     * Update existing record by values from feature.
     *  \param cat
     *  \param nullValues override all values, if false, only non empty values are used for update
     */
    void updateAttributes( int cat, QgsFeature &feature, QString &error, bool nullValues = false );

    /**
     * Delete attributes from the table
     *   \param cat
     */
    void deleteAttribute( int cat, QString &error );

    /**
     * Check if a database row exists
     *   \param cat
     *   \param error set to error if happens
     *   \returns true if cat is orphan
     */
    bool recordExists( int cat, QString &error );

    /**
     * Check if a database row exists and it is orphan (no more lines with that category)
     *   \param cat
     *   \param error set to error if happens
     *   \returns true if cat is orphan
     */
    bool isOrphan( int cat, QString &error );

    /**
     * Create table and link vector to this table
     * \param fields fields to be created without cat (id) field
     */
    void createTable( const QgsFields &fields, QString &error );

    /**
     * Add column to table
     *   \param field
     */
    void addColumn( const QgsField &field, QString &error );

    void deleteColumn( const QgsField &field, QString &error );

    //! Insert records for all existing categories to the table
    void insertCats( QString &error );

    // update fields to real state
    void updateFields();

    // for debug only
    void printCachedAttributes();

  private:
    QString quotedValue( const QVariant &value );
    dbDriver *openDriver( QString &error );
    void addTopoField( QgsFields &fields );
    int mField;
    bool mValid = false;
    QgsGrassVectorMap *mMap = nullptr;
    struct field_info *mFieldInfo = nullptr;
    dbDriver *mDriver = nullptr;

    bool mHasTable = false;
    // index of key column
    int mKeyColumn = -1;

    // table fields, updated if a field is added/deleted, if there is no table, it contains
    // cat field
    QgsFields mTableFields;

    // original fields + topo symbol when editing, does not reflect add/column
    QgsFields mFields;

    // list of fields in mAttributes, these fields may only grow when a field is added,
    // but do not shrink until editing is closed
    QgsFields mAttributeFields;

    // Map of attributes with cat as key
    QMap<int, QList<QVariant>> mAttributes;

    // Map of current original fields() indexes to mAttributes, skipping topo symbol
    //QMap<int, int> mAttributeIndexes;

    // minimum and maximum values of attributes
    QList<QPair<double, double>> mMinMax;
    // timestamp when attributes were loaded
    QDateTime mLastLoaded;
    // number of instances using this layer
    int mUsers = 0;
};

#endif // QGSGRASSVECTORMAPLAYER_H
