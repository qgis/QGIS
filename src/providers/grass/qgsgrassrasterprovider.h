/***************************************************************************
      qgsgrassrasterprovider.h  -  QGIS Data provider for
                           GRASS rasters
                             -------------------
    begin                : 16 Jan, 2010
    copyright            : (C) 2010 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSGRASSRASTERPROVIDER_H
#define QGSGRASSRASTERPROVIDER_H

#include "qgscoordinatereferencesystem.h"
#include "qgsrasterdataprovider.h"
#include "qgsrectangle.h"
#include "qgscolorrampshader.h"
#include "qgis_grass_lib.h"

extern "C"
{
#include <grass/version.h>
#include <grass/gis.h>
#include <grass/raster.h>
}

#include <QString>
#include <QStringList>
#include <QDomElement>
#include <QMap>
#include <QVector>
#include <QTemporaryFile>
#include <QProcess>
#include <QHash>

class QgsCoordinateTransform;

/**
 * \brief Read raster value for given coordinates
 *
 * Executes qgis.g.info and keeps it open comunicating through pipe. Restarts the command if raster was updated.
*/

class GRASS_LIB_EXPORT QgsGrassRasterValue
{
  public:
    QgsGrassRasterValue() = default;
    ~QgsGrassRasterValue();

    QgsGrassRasterValue( const QgsGrassRasterValue &other ) = delete;
    QgsGrassRasterValue &operator=( const QgsGrassRasterValue &other ) = delete;

    void set( const QString &gisdbase, const QString &location, const QString &mapset, const QString &map );
    void stop();
    // returns raster value, NaN for no data
    // OK is set to true if OK or false on error
    double value( double x, double y, bool *ok );
  private:

    void start();
    QString mGisdbase;      // map gisdabase
    QString mLocation;      // map location name (not path!)
    QString mMapset;        // map mapset
    QString mMapName;       // map name
    QTemporaryFile mGisrcFile;
    QProcess *mProcess = nullptr;
};

/**
 *
 * \brief Data provider for GRASS raster layers.
 *
 * This provider implements the
 * interface defined in the QgsDataProvider class to provide access to spatial
 * data residing in a OGC Web Map Service.
 *
*/
class GRASS_LIB_EXPORT QgsGrassRasterProvider : public QgsRasterDataProvider
{
    Q_OBJECT

  public:

    /**
     * Constructor for the provider.
     *
     * \param   uri   HTTP URL of the Web Server.  If needed a proxy will be used
     *                otherwise we contact the host directly.
     *
     */
    explicit QgsGrassRasterProvider( QString const &uri = QString() );


    ~QgsGrassRasterProvider() override;

    QgsGrassRasterProvider *clone() const override;

    /**
     * Returns a provider name
     *
     * Essentially just returns the provider key.  Should be used to build file
     * dialogs so that providers can be shown with their supported types. Thus
     * if more than one provider supports a given format, the user is able to
     * select a specific provider to open that file.
     *
     * \note
     *
     * Instead of being pure virtual, might be better to generalize this
     * behavior and presume that none of the sub-classes are going to do
     * anything strange with regards to their name or description?
     *
     */
    QString name() const override;


    /**
     * Returns description
     *
     * Returns a terse string describing what the provider is.
     *
     * \note
     *
     * Instead of being pure virtual, might be better to generalize this
     * behavior and presume that none of the sub-classes are going to do
     * anything strange with regards to their name or description?
     *
     */
    QString description() const override;

    QgsCoordinateReferenceSystem crs() const override;

    /**
     * Returns the extent for this data layer
     */
    QgsRectangle extent() const override;

    bool isValid() const override;

    QgsRasterIdentifyResult identify( const QgsPointXY &point, Qgis::RasterIdentifyFormat format, const QgsRectangle &boundingBox = QgsRectangle(), int width = 0, int height = 0, int dpi = 96 ) override;

    /**
     * \brief   Returns the caption error text for the last error in this provider
     *
     * If an operation returns 0 (e.g. draw()), this function
     * returns the text of the error associated with the failure.
     * Interactive users of this provider can then, for example,
     * call a QMessageBox to display the contents.
     */
    QString lastErrorTitle() override;

    /**
     * \brief   Returns the verbose error text for the last error in this provider
     *
     * If an operation returns 0 (e.g. draw()), this function
     * returns the text of the error associated with the failure.
     * Interactive users of this provider can then, for example,
     * call a QMessageBox to display the contents.
     */

    QString lastError() override;

    /**
     * Returns a bitmask containing the supported capabilities
     * Note, some capabilities may change depending on which
     * sublayers are visible on this provider, so it may
     * be prudent to check this value per intended operation.
    */
    int capabilities() const override;

    Qgis::DataType dataType( int bandNo ) const override;
    Qgis::DataType sourceDataType( int bandNo ) const override;

    int bandCount() const override;

    Qgis::RasterColorInterpretation colorInterpretation( int bandNo ) const override;

    int xBlockSize() const override;
    int yBlockSize() const override;

    int xSize() const override;
    int ySize() const override;

    bool readBlock( int bandNo, int xBlock, int yBlock, void *data ) override;
    bool readBlock( int bandNo, QgsRectangle  const &viewExtent, int width, int height, void *data, QgsRasterBlockFeedback *feedback = nullptr ) override;

    QgsRasterBandStats bandStatistics( int bandNo,
                                       int stats = QgsRasterBandStats::All,
                                       const QgsRectangle &boundingBox = QgsRectangle(),
                                       int sampleSize = 0, QgsRasterBlockFeedback *feedback = nullptr ) override;

    QList<QgsColorRampShader::ColorRampItem> colorTable( int bandNo )const override;

    // void buildSupportedRasterFileFilter( QString & fileFiltersString );

    /**
     * Gets metadata in a format suitable for feeding directly
     * into a subset of the GUI raster properties "Metadata" tab.
     */
    QString htmlMetadata() override;

    QDateTime dataTimestamp() const override;

    // used by GRASS tools
    void freeze();
    void thaw();

  private:
    void setLastError( const QString &error );
    void clearLastError();
    // append error if it is not empty
    void appendIfError( const QString &error );

    /**
     * Flag indicating if the layer data source is a valid layer
     */
    bool mValid = false;

    QString mGisdbase;      // map gisdabase
    QString mLocation;      // map location name (not path!)
    QString mMapset;        // map mapset
    QString mMapName;       // map name

    RASTER_MAP_TYPE mGrassDataType = 0; // CELL_TYPE, DCELL_TYPE, FCELL_TYPE

    int mCols = 0;
    int mRows = 0;
    int mYBlockSize = 0;

    QHash<QString, QString> mInfo;

    QgsCoordinateReferenceSystem mCrs;

    QgsGrassRasterValue mRasterValue;

    double mNoDataValue;

    QString mLastErrorTitle;
    QString mLastError;
};

#endif
