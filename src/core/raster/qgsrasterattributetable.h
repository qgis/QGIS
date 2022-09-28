/***************************************************************************
  qgsrasterattributetable.h - QgsRasterAttributeTable

 ---------------------
 begin                : 3.12.2021
 copyright            : (C) 2021 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRASTERATTRIBUTETABLE_H
#define QGSRASTERATTRIBUTETABLE_H

#include "qgsfields.h"
#include "qgsfeature.h"
#include "qgis_core.h"
#include "gdal.h"
#include "qgis_sip.h"
#include "qgis.h"

#include <QObject>

/**
 * \ingroup core
 * \brief The QgsRasterAttributeTable class represents a raster attribute table (RAT).
 *
 * This class is modeled after the GDAL RAT implementation, it adds some convenience
 * methods to handle data from QGIS and to import/export a RAT from/to a DBF VAT file.
 *
 * \since QGIS 3.30
 */
class CORE_EXPORT QgsRasterAttributeTable
{

  public:


    /**
     * \brief The Field struct represents a RAT field, including its name, usage and type.
     */
    struct Field
    {

      /**
       * Creates a new Field with \a name, \a type and \a usage.
       */
      Field( const QString &name, const Qgis::RasterAttributeTableFieldUsage &usage, const QVariant::Type type ): name( name ), usage( usage ), type( type ) {}
      QString name;
      Qgis::RasterAttributeTableFieldUsage usage;
      QVariant::Type type;
    };

    /**
     * Returns the RAT type.
     */
    Qgis::RasterAttributeTableType type() const;

    /**
     * Sets the RAT \a type
     */
    void setType( const Qgis::RasterAttributeTableType type );

    /**
     * Returns TRUE if the RAT has RGB information.
     */
    bool hasColor();

    /**
     * Returns the RAT fields.
     */
    QList<QgsRasterAttributeTable::Field> fields() const;

    /**
     * Returns the RAT fields as QgsFields.
     */
    QgsFields qgisFields() const;

    /**
     * Returns the RAT rows as a list of QgsFeature.
     */
    QgsFeatureList qgisFeatures( ) const;

    /**
     * Returns TRUE if the RAT was modified from its last reading from the storage.
     */
    bool isDirty() const;

    /**
     * Sets the RAT dirty state to \a isDirty;
     */
    void setIsDirty( bool isDirty );

    /**
     * Returns TRUE if the RAT is valid.
     */
    bool isValid() const;

    /**
     * Inserts a new \a field at \a position and returns TRUE on success.
     */
    bool insertField( const QgsRasterAttributeTable::Field &field, int position = 0 );

    /**
     * Creates a new field from \a name, \a usage and \a type and inserts it at \a position, returns TRUE on success.
     */
    bool insertField( const QString &name, Qgis::RasterAttributeTableFieldUsage usage, QVariant::Type type, int position = 0 );

    /**
     * Creates a new field from \a name, \a usage and \a type and appends it to the fields, returns TRUE on success.
     */
    bool appendField( const QString &name, Qgis::RasterAttributeTableFieldUsage usage, QVariant::Type type );

    /**
     * Appends a new \a field and returns TRUE on success.
     */
    bool appendField( const QgsRasterAttributeTable::Field &field );

    /**
     * Removes the field with \a name, returns TRUE on success.
     */
    bool removeField( const QString &name );

    /**
     * Inserts a row of \a data in the RAT at \a position, returns TRUE on success.
     */
    bool insertRow( const QVariantList data, int position = 0 );

    /**
     * Appends a row of \a data to the RAT, returns TRUE on success.
     */
    bool appendRow( const QVariantList data );

    /**
     * Writes the RAT to a DBF file specified by \a path, optionally reporting any error in \a errorMessage, returns TRUE on success.
     */
    bool writeToFile( const QString &path, QString *errorMessage SIP_OUT = nullptr );

    /**
     * Reads the RAT from a DBF file specified by \a path, optionally reporting any error in \a errorMessage, returns TRUE on success.
     */
    bool readFromFile( const QString &path, QString *errorMessage SIP_OUT = nullptr );

    /**
     * Returns the RAT rows.
     */
    const QList<QList<QVariant>> data() const;

    /**
     * Try to determine the field usage from its \a name and \a type.
     */
    static Qgis::RasterAttributeTableFieldUsage guessFieldUsage( const QString &name, const QVariant::Type type );

  private:

    Qgis::RasterAttributeTableType mType = Qgis::RasterAttributeTableType::Athematic;
    QList<Field> mFields;
    QList<QVariantList> mData;
    bool mIsDirty;

};

#endif // QGSRASTERATTRIBUTETABLE_H
