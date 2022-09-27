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
     * \brief The FieldUsage enum represents the usage of a RAT field.
     */
    enum class FieldUsage : int
    {
      Generic = GFU_Generic, //!< Field usage Generic
      PixelCount = GFU_PixelCount, //!< Field usage PixelCount
      Name = GFU_Name, //!< Field usage Name
      Min = GFU_Min, //!< Field usage Min
      Max = GFU_Max, //!< Field usage Max
      MinMax = GFU_MinMax, //!< Field usage MinMax
      Red = GFU_Red, //!< Field usage Red
      Green = GFU_Green, //!< Field usage Green
      Blue = GFU_Blue, //!< Field usage Blue
      Alpha = GFU_Alpha, //!< Field usage Alpha
      RedMin = GFU_RedMin, //!< Field usage RedMin
      GreenMin = GFU_GreenMin, //!< Field usage GreenMin
      BlueMin = GFU_BlueMin, //!< Field usage BlueMin
      AlphaMin = GFU_AlphaMin, //!< Field usage AlphaMin
      RedMax = GFU_RedMax, //!< Field usage RedMax
      GreenMax = GFU_GreenMax, //!< Field usage GreenMax
      BlueMax = GFU_BlueMax, //!< Field usage BlueMax
      AlphaMax = GFU_AlphaMax, //!< Field usage AlphaMax
      MaxCount = GFU_MaxCount //!< Field usage MaxCount
    };

    /**
     * \brief The RatType enum represents the type of RAT.
     */
    enum class RatType : int
    {
      Thematic = GRTT_THEMATIC,
      Athematic = GRTT_ATHEMATIC
    };

    /**
     * \brief The Field struct represents a RAT field, including its name, usage and type.
     */
    struct Field
    {

      /**
       * Creates a new Field with \a name, \a type and \a usage.
       */
      Field( const QString &name, const FieldUsage &usage, const QVariant::Type type ): name( name ), usage( usage ), type( type ) {}
      QString name;
      FieldUsage usage;
      QVariant::Type type;
    };

    /**
     * Returns the RAT type.
     */
    const RatType &type() const;

    /**
     * Sets the RAT \a type
     */
    void setType( const RatType &newType );

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
    bool insertField( const QString &name, QgsRasterAttributeTable::FieldUsage usage, QVariant::Type type, int position = 0 );

    /**
     * Creates a new field from \a name, \a usage and \a type and appends it to the fields, returns TRUE on success.
     */
    bool appendField( const QString &name, QgsRasterAttributeTable::FieldUsage usage, QVariant::Type type );

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
    const QList<QList<QVariant>> &data() const;

    /**
     * Try to determine the field usage from its \a name and \a type.
     */
    static FieldUsage guessFieldUsage( const QString &name, const QVariant::Type type );

  private:

    RatType mType;
    QList<Field> mFields;
    QList<QVariantList> mData;
    bool mIsDirty;

};

#endif // QGSRASTERATTRIBUTETABLE_H
