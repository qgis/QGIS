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
#include "qgis_sip.h"
#include "qgis.h"
#include "qgscolorrampimpl.h"

#include <QObject>
#include <QLinearGradient>
#include <QCoreApplication>

class QgsRasterLayer;
class QgsRasterRenderer;
class QgsRasterDataProvider;

/**
 * \ingroup core
 * \brief The QgsRasterAttributeTable class represents a Raster Attribute Table (RAT).
 *
 * This class is modeled after the GDAL Raster Attribute Table implementation, it adds some convenience
 * methods to handle data from QGIS and to import/export a Raster Attribute Table from/to a DBF VAT file.
 *
 * \since QGIS 3.30
 */
class CORE_EXPORT QgsRasterAttributeTable
{

    Q_DECLARE_TR_FUNCTIONS( QgsRasterAttributeTable )

  public:

    /**
     * \ingroup core
     * \brief The UsageInformation class represents information about a field usage.
     * \since QGIS 3.30
     */
    class CORE_EXPORT UsageInformation
    {
      public:

        //! Usage description
        QString description;

        //! Usage must be unique
        bool unique = false;

        //! Usage is required
        bool required = false;

        //! Usage is part of a color component
        bool isColor = false;

        //! Usage is part of a ramp component
        bool isRamp = false;

        //! Usage is supported
        bool supported = false;

        //! May be suitable for classification
        bool maybeClass = false;

        //! Usage allowed types
        QList<QVariant::Type> allowedTypes;
    };

    /**
     * \ingroup core
     * \brief The Field class represents a Raster Attribute Table field, including its name, usage and type.
     * \since QGIS 3.30
     */
    class CORE_EXPORT Field
    {

      public:

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
     * \ingroup core
     * \brief The Field class represents a Raster Attribute Table classification entry for a thematic Raster Attribute Table.
     * \since QGIS 3.30
     */
    class CORE_EXPORT MinMaxClass
    {
      public:
        QString name;

        //! List of values for the class
        QVector< double > minMaxValues;
        QColor color;
    };

    /**
     * Returns the Raster Attribute Table type.
     */
    Qgis::RasterAttributeTableType type() const;

    /**
     * Returns TRUE if the Raster Attribute Table has color RGBA information.
     * \see color()
     * \see setColor()
     * \see hasRamp()
     * \see setRamp()
     * \see ramp()
     */
    bool hasColor() const;

    /**
     * Sets the color for the row at \a rowIndex to \a color.
     * \a returns TRUE on success.
     * \see hasColor()
     * \see setColor()
     * \see hasRamp()
     * \see setRamp()
     * \see ramp()
     */
    bool setColor( const int row, const QColor &color );

    /**
     * Returns TRUE if the Raster Attribute Table has ramp RGBA information.
     * \see setRamp()
     * \see ramp()
     * \see hasColor()
     * \see setColor()
     */
    bool hasRamp() const;

    /**
     * Sets the color ramp for the row at \a rowIndex to \a colorMin and \a colorMax.
     * \a returns TRUE on success.
     * \see hasRamp()
     * \see ramp()
     * \see hasColor()
     * \see setColor()
     */
    bool setRamp( const int row, const QColor &colorMin, const QColor &colorMax );

    /**
     * Returns the list of field usages.
     */
    QList<Qgis::RasterAttributeTableFieldUsage> usages( ) const SIP_SKIP;

///@cond PRIVATE

    /**
     * Returns the list of field usages.
     */
    QList<int> intUsages( ) const SIP_PYNAME( usages );

///@endcond PRIVATE

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
     * Returns the gradient color ramp of the rat \a row or a default constructed gradient if row does not exist or if there is no color ramp definition.
     * \see hasRamp()
     * \see setRamp()
     * \see hasColor()
     * \see setColor()
     * \see color()
     */
    QgsGradientColorRamp ramp( int row ) const;

    /**
     * Returns the Raster Attribute Table fields.
     * \see qgisFields()
     */
    QList<QgsRasterAttributeTable::Field> fields() const;

    /**
     * Returns the Raster Attribute Table fields as QgsFields.
     * \see fields()
     */
    QgsFields qgisFields() const;

    /**
     * Returns the Raster Attribute Table rows as a list of QgsFeature.
     */
    QgsFeatureList qgisFeatures( ) const;

    /**
     * Returns TRUE if the Raster Attribute Table was modified from its last reading from the storage.
     */
    bool isDirty() const;

    /**
     * Sets the Raster Attribute Table dirty state to \a isDirty;
     */
    void setDirty( bool isDirty );

    /**
     * Returns TRUE if the Raster Attribute Table is valid, optionally reporting validity checks results in \a errorMessage.
     */
    bool isValid( QString *errorMessage SIP_OUT = nullptr ) const;

    /**
     * Inserts a new \a field at \a position, optionally reporting any error in \a errorMessage, returns TRUE on success.
     * \note Out of range position is automatically clamped to a valid value.
     */
    bool insertField( int position, const QgsRasterAttributeTable::Field &field, QString *errorMessage SIP_OUT = nullptr );

    /**
     * Creates a new field from \a name, \a usage and \a type and inserts it at \a position, optionally reporting any error in \a errorMessage, returns TRUE on success.
     */
    bool insertField( int position, const QString &name, const Qgis::RasterAttributeTableFieldUsage usage, const QVariant::Type type, QString *errorMessage SIP_OUT = nullptr );

    /**
     * Create RGBA fields and inserts them at \a position, optionally reporting any error in \a errorMessage, returns TRUE on success.
     */
    bool insertColor( int position, QString *errorMessage SIP_OUT = nullptr );

    /**
     * Change the usage of the field at index \a fieldIndex to \a usage with checks for allowed types.
     * \return TRUE on success.
     */
    bool setFieldUsage( int fieldIndex, const Qgis::RasterAttributeTableFieldUsage usage );

    /**
     * Create RGBA minimum and maximum fields and inserts them at \a position, optionally reporting any error in \a errorMessage, returns TRUE on success.
     */
    bool insertRamp( int position, QString *errorMessage SIP_OUT = nullptr );

    /**
     * Creates a new field from \a name, \a usage and \a type and appends it to the fields, optionally reporting any error in \a errorMessage, returns TRUE on success.
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
     * Inserts a row of \a rowData in the Raster Attribute Table at \a position, optionally reporting any error in \a errorMessage, returns TRUE on success.
     * \note Out of range position is automatically clamped to a valid value.
     */
    bool insertRow( int position, const QVariantList &rowData, QString *errorMessage SIP_OUT = nullptr );

    /**
     * Removes the row in the Raster Attribute Table at \a position, optionally reporting any error in \a errorMessage, returns TRUE on success.
     * \note position must be a valid position.
     */
    bool removeRow( int position = 0, QString *errorMessage SIP_OUT = nullptr );

    /**
     * Appends a row of \a data to the RAT, optionally reporting any error in \a errorMessage, returns TRUE on success.
     */
    bool appendRow( const QVariantList &data, QString *errorMessage SIP_OUT = nullptr );

    /**
     * Writes the Raster Attribute Table to a DBF file specified by \a path, optionally reporting any error in \a errorMessage, returns TRUE on success.
     * \note ".vat.dbf" extension is automatically added to the file path if not present.
     */
    bool writeToFile( const QString &path, QString *errorMessage SIP_OUT = nullptr );

    /**
     * Reads the Raster Attribute Table from a DBF file specified by \a path, optionally reporting any error in \a errorMessage, returns TRUE on success.
     */
    bool readFromFile( const QString &path, QString *errorMessage SIP_OUT = nullptr );

    /**
     * Returns the Raster Attribute Table rows.
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
    QVariant value( const int row, const int column ) const;

    /**
     * Returns the minimum value of the MinMax (thematic) or Min (athematic) column, returns NaN on errors.
     */
    double minimumValue( ) const;

    /**
     * Returns the maximum value of the MinMax (thematic) or Max (athematic) column, returns NaN on errors.
     */
    double maximumValue( ) const;

    /**
     * Returns a row of data for the given \a matchValue or and empty row
     * if there is not match.
     */
    QVariantList row( const double matchValue ) const;

    /**
     * Returns the classes for a thematic Raster Attribute Table, classified
     * by \a classificationColumn, the default value of -1 makes the method guess
     * the classification column based on the field usage.
     */
    QList<QgsRasterAttributeTable::MinMaxClass> minMaxClasses( const int classificationColumn  = -1 ) const;

    /**
     * Returns the color ramp for an athematic Raster Attribute Table
     * setting the labels in \a labels, optionally generated from \a labelColumn.
     */
    QgsGradientColorRamp colorRamp( QStringList &labels SIP_OUT, const int labelColumn = -1 ) const;

    /**
     * Creates and returns a (possibly NULLPTR) raster renderer for the
     * specified \a provider and \a bandNumber and optionally reclassified
     * by \a classificationColumn, the default value of -1 makes the method
     * guess the classification column based on the field usage.
     *
     * \note athematic attribute tables with color ramps cannot be reclassified,
     *       the renderer will still use the \a classificationColumn for
     *       generating the class labels.
     */
    QgsRasterRenderer *createRenderer( QgsRasterDataProvider *provider, const int bandNumber, const int classificationColumn = -1 ) SIP_FACTORY;

    /**
     * Returns the data rows ordered by the value column(s) in ascending order, if
     * the attribute table type is athematic the middle value for each row range
     * is considered for ordering.
     * If the attribute table does not have any value field (and hence is not valid),
     * the current data are returned without any change.
     */
    QList<QList<QVariant>> orderedRows( ) const;

    /**
     * Try to determine the field usage from its \a name and \a type.
     */
    static Qgis::RasterAttributeTableFieldUsage guessFieldUsage( const QString &name, const QVariant::Type type );

    /**
    * Returns the (possibly empty) path of the file-based RAT, the path is set when a RAT is read or written from/to a file.
    *
    * \see writeToFile()
    * \see readFromFile()
    */
    QString filePath() const;

    /**
     * Returns the translated human readable name of \a fieldUsage.
     * \see usageInformation()
     */
    static QString usageName( const Qgis::RasterAttributeTableFieldUsage fieldusage );

    /**
     * Returns the list of field usages for colors and values.
     */
    static QList<Qgis::RasterAttributeTableFieldUsage> valueAndColorFieldUsages();

    /**
     * Creates a new Raster Attribute Table from a raster layer, the renderer must be Paletted or SingleBandPseudoColor, optionally reporting the raster band from which the attribute table was created.
     * \param rasterLayer raster layer
     * \param bandNumber band number
     * \returns NULL in case of errors or unsupported renderer.
     */
    static QgsRasterAttributeTable *createFromRaster( QgsRasterLayer *rasterLayer, int *bandNumber SIP_OUT = nullptr ) SIP_FACTORY;

    /**
     * Returns information about supported Raster Attribute Table usages.
     * \see usageName()
     */
    static QHash<Qgis::RasterAttributeTableFieldUsage, QgsRasterAttributeTable::UsageInformation> usageInformation( ) SIP_SKIP;

///@cond PRIVATE

    /**
     * Returns information about supported Raster Attribute Table usages.
     * \see usageName()
     */
    static QHash<int, QgsRasterAttributeTable::UsageInformation> usageInformationInt( ) SIP_PYNAME( usageInformation );

    static QHash<Qgis::RasterAttributeTableFieldUsage, QgsRasterAttributeTable::UsageInformation> sUsageInformation SIP_SKIP;

///@endcond PRIVATE

  private:

    Qgis::RasterAttributeTableType mType = Qgis::RasterAttributeTableType::Thematic;
    QList<Field> mFields;
    QList<QVariantList> mData;
    bool mIsDirty;
    QString mFilePath;

    // Set type from fields.
    void setType( );

};

#endif // QGSRASTERATTRIBUTETABLE_H
