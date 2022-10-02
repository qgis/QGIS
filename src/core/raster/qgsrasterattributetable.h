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
#include <QLinearGradient>

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
     * \brief The Ramp struct represents the min and max colors of a RAT row.
     */
    struct CORE_EXPORT Ramp
    {
      QColor min;
      QColor max;
    };

    /**
     * \brief The Field struct represents a RAT field, including its name, usage and type.
     */
    struct CORE_EXPORT Field
    {

      /**
       * Creates a new Field with \a name, \a type and \a usage.
       */
      Field( const QString &name, const Qgis::RasterAttributeTableFieldUsage &usage, const QVariant::Type type ): name( name ), usage( usage ), type( type ) {}

      /**
       * Returns TRUE if the field carries a color component (Red, Green, Blue and optionally Alpha) information.
       */
      bool isColor( ) const;

      /**
       * Returns TRUE if the field carries a color ramp component information (RedMin/RedMax, GreenMin/GreenMax, BlueMin/BlueMax and optionally AlphaMin/AlphaMax) information.
       */
      bool isRamp( ) const;

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
     * Returns TRUE if the RAT has color RGBA information.
     * \see color()
     * \see setColor()
     * \see hasRamp()
     * \see setRamp()
     * \see ramp()
     */
    bool hasColor() const;

    /**
     * Sets the color for the row at \a rowIndex to \a color.
     * \a returns TRUE on sucess.
     * \see hasColor()
     * \see setColor()
     * \see hasRamp()
     * \see setRamp()
     * \see ramp()
     */
    bool setColor( const int row, const QColor &color );

    /**
     * Returns TRUE if the RAT has ramp RGBA information.
     * \see setRamp()
     * \see ramp()
     * \see hasColor()
     * \see setColor()
     */
    bool hasRamp() const;

    /**
     * Sets the color ramp for the row at \a rowIndex to \a colorMin and \a colorMax.
     * \a returns TRUE on sucess.
     * \see hasRamp()
     * \see ramp()
     * \see hasColor()
     * \see setColor()
     */
    bool setRamp( const int row, const QColor &colorMin, const QColor &colorMax );

    /**
     * Returns the list of field usages.
     */
    QList<Qgis::RasterAttributeTableFieldUsage> usages( ) const;

    /**
     * Returns the color of the rat \a row or an invalid color if row does not exist or if there is no color definition.
     * \see hasColor()
     * \see setColor()
     * \see hasRamp()
     * \see setRamp()
     * \see ramp()
     */
    QColor color( int row ) const;

    /**
     * Returns the color of the rat \a row or an invalid color if row does not exist or if there is no color ramp definition.
     * \see hasRamp()
     * \see setRamp()
     * \see hasColor()
     * \see setColor()
     * \see color()
     */
    Ramp ramp( int row ) const;

    /**
     * Returns the RAT fields.
     * \see qgisFields()
     */
    QList<QgsRasterAttributeTable::Field> fields() const;

    /**
     * Returns the RAT fields as QgsFields.
     * \see fields()
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
     * Returns TRUE if the RAT is valid, optionally reporting validity checks results in \a errorMessage.
     */
    bool isValid( QString *errorMessage SIP_OUT = nullptr ) const;

    /**
     * Inserts a new \a field at \a position, optionally reporting any error in \a errorMessage, returns TRUE on success.
     * \note Out of range position is automatically clamped to a valid value.
     */
    bool insertField( const QgsRasterAttributeTable::Field &field, int position = 0, QString *errorMessage SIP_OUT = nullptr );

    /**
     * Creates a new field from \a name, \a usage and \a type and inserts it at \a position, optionally reporting any error in \a errorMessage, returns TRUE on success.
     */
    bool insertField( const QString &name, const Qgis::RasterAttributeTableFieldUsage usage, const QVariant::Type type, int position = 0, QString *errorMessage SIP_OUT = nullptr );

    /**
     * Creates a new field from \a name, \a usage and \a type and appends it to the fields, returns TRUE on success.
     */
    bool appendField( const QString &name, const Qgis::RasterAttributeTableFieldUsage usage, const QVariant::Type type, QString *errorMessage SIP_OUT = nullptr );

    /**
     * Appends a new \a field, optionally reporting any error in \a errorMessage, returns TRUE on success.
     */
    bool appendField( const QgsRasterAttributeTable::Field &field, QString *errorMessage SIP_OUT = nullptr );

    /**
     * Removes the field with \a name, optionally reporting any error in \a errorMessage, returns TRUE on success.
     */
    bool removeField( const QString &name, QString *errorMessage SIP_OUT = nullptr );

    /**
     * Inserts a row of \a rowData in the RAT at \a position, optionally reporting any error in \a errorMessage, returns TRUE on success.
     * \note Out of range position is automatically clamped to a valid value.
     */
    bool insertRow( const QVariantList &rowData, int position = 0, QString *errorMessage SIP_OUT = nullptr );

    /**
     * Removes the row in the RAT at \a position, optionally reporting any error in \a errorMessage, returns TRUE on success.
     * \note position must be a valid position.
     */
    bool removeRow( int position = 0, QString *errorMessage SIP_OUT = nullptr );

    /**
     * Appends a row of \a data to the RAT, optionally reporting any error in \a errorMessage, returns TRUE on success.
     */
    bool appendRow( const QVariantList &data, QString *errorMessage SIP_OUT = nullptr );

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
     * Returns a field by name or a default constructed field with empty name if the field is not found.
     * \param name of the field
     * \param ok if specified, will be set to TRUE if the field was found.
     * \returns the matching field or a default constructed one.
     */
    const QgsRasterAttributeTable::Field fieldByName( const QString name, bool *ok SIP_OUT = nullptr ) const;

    /**
     * Returns the list of fields matching \a fieldUsage.
     */
    const QList<QgsRasterAttributeTable::Field> fieldsByUsage( const Qgis::RasterAttributeTableFieldUsage fieldUsage ) const;

    /**
     * Sets the \a value for \a row and \a column.
     * \returns TRUE on success.
     */
    bool setValue( const int row, const int column, const QVariant &value );

    /**
     * Returns the \a value for \a row and \a column.
     */
    QVariant value( const int row, const int column );

    /**
     * Try to determine the field usage from its \a name and \a type.
     */
    static Qgis::RasterAttributeTableFieldUsage guessFieldUsage( const QString &name, const QVariant::Type type );

    /**
     * Returns the translated human readable name of \a fieldUsage.
     */
    static QString usageName( const Qgis::RasterAttributeTableFieldUsage fieldusage );

  private:

    Qgis::RasterAttributeTableType mType = Qgis::RasterAttributeTableType::Athematic;
    QList<Field> mFields;
    QList<QVariantList> mData;
    bool mIsDirty;

};

#endif // QGSRASTERATTRIBUTETABLE_H
