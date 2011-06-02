/***************************************************************************
                qgsdataprovider.h - DataProvider Interface class
                     --------------------------------------
    Date                 : 09-Sep-2003
    Copyright            : (C) 2003 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QQGSDATAPROVIDER_H
#define QQGSDATAPROVIDER_H

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QStringList>

//#include "qgsdataitem.h"

class QgsRectangle;
class QgsCoordinateReferenceSystem;


/** \ingroup core
 * Abstract base class for spatial data provider implementations.
 * @author Gary E.Sherman
 *
 * This object needs to inherit from QObject to enable event
 * processing in the Postgres/PostGIS provider (QgsPostgresProvider).
 * It is called *here* so that this vtable and the vtable for
 * QgsPostgresProvider don't get misaligned -
 * the QgsVectorLayer class factory (which refers
 * to generic QgsVectorDataProvider's) depends on it.
 */


class CORE_EXPORT QgsDataProvider : public QObject
{
    Q_OBJECT

  public:

    Q_ENUMS( DataCapability )

    enum DataCapability
    {
      NoDataCapabilities  = 0,
      File                = 1,
      Dir                 = 1 << 1,
      Database            = 1 << 2,
      Net                 = 1 << 3  // Internet source
    };

    QgsDataProvider( QString const & uri = "" )
        : mDataSourceURI( uri )
    {}

    /**
     * We need this so the subclass destructors get called
     */
    virtual ~QgsDataProvider() {};


    /*! Get the QgsCoordinateReferenceSystem for this layer
     * @note Must be reimplemented by each provider.
     * If the provider isn't capable of returning
     * its projection an empty srs will be return, ti will return 0
     */
    virtual QgsCoordinateReferenceSystem crs() = 0;


    /**
     * Set the data source specification. This may be a path or database
     * connection string
     * @param uri source specification
     */
    virtual void setDataSourceUri( QString const & uri )
    {
      mDataSourceURI = uri;
    }

    /**
     * Get the data source specification. This may be a path or database
     * connection string
     * @return data source specification
     */
    virtual QString dataSourceUri() const
    {
      return mDataSourceURI;
    }


    /**
     * Get the extent of the layer
     * @return QgsRectangle containing the extent of the layer
     */
    virtual QgsRectangle extent() = 0;


    /**
     * Returns true if this is a valid layer. It is up to individual providers
     * to determine what constitutes a valid layer
     */
    virtual bool isValid() = 0;


    /**
     * Update the extents of the layer. Not implemented by default
     */
    virtual void updateExtents()
    {
      // NOP by default
    }


    /**
     * Set the subset string used to create a subset of features in
     * the layer. This may be a sql where clause or any other string
     * that can be used by the data provider to create a subset.
     * Must be implemented in the dataprovider.
     */
    virtual bool setSubsetString( QString subset, bool updateFeatureCount = true )
    {
      // NOP by default
      Q_UNUSED( subset );
      return false;
    }


    /**
     * provider supports setting of subset strings
     * @note added in 1.4
     */
    virtual bool supportsSubsetString() { return false; }

    /**
     * Returns the subset definition string (typically sql) currently in
     * use by the layer and used by the provider to limit the feature set.
     * Must be overridden in the dataprovider, otherwise returns a null
     * QString.
     */
    virtual QString subsetString()
    {
      return QString::null;
    }


    /**
     * Sub-layers handled by this provider, in order from bottom to top
     *
     * Sub-layers are used when the provider's source can combine layers
     * it knows about in some way before it hands them off to the provider.
     */
    virtual QStringList subLayers() const
    {
      return QStringList();  // Empty
    }


    /**
     * Sub-layer styles for each sub-layer handled by this provider,
     * in order from bottom to top
     *
     * Sub-layer styles are used to abstract the way the provider's source can symbolise
     * layers in some way at the server, before it serves them to the provider.
     */
    virtual QStringList subLayerStyles() const
    {
      return QStringList();  // Empty
    }


    /**
     * return the number of layers for the current data source
     */
    virtual uint subLayerCount() const
    {
      return 0;
    }


    /**
     * Reorder the list of layer names to be rendered by this provider
     * (in order from bottom to top)
     * \note   layers must have been previously added.
     */
    virtual void setLayerOrder( QStringList layers )
    {
      //prevent unused var warnings
      if ( layers.count() < 1 )
      {
        return;
      }
      // NOOP
    }


    /**
     * Set the visibility of the given sublayer name
     */
    virtual void setSubLayerVisibility( QString name, bool vis )
    {
      //prevent unused var warnings
      if ( name.isEmpty() || !vis )
      {
        return;
      }
      // NOOP
    }


    /** return a provider name

    Essentially just returns the provider key.  Should be used to build file
    dialogs so that providers can be shown with their supported types. Thus
    if more than one provider supports a given format, the user is able to
    select a specific provider to open that file.

    @note

    Instead of being pure virtual, might be better to generalize this
    behavior and presume that none of the sub-classes are going to do
    anything strange with regards to their name or description?

    */
    virtual QString name() const = 0;


    /** return description

      Return a terse string describing what the provider is.

      @note

      Instead of being pure virtual, might be better to generalize this
      behavior and presume that none of the sub-classes are going to do
      anything strange with regards to their name or description?

     */
    virtual QString description() const = 0;


    /** return vector file filter string

      Returns a string suitable for a QFileDialog of vector file formats
      supported by the data provider.  Naturally this will be an empty string
      for those data providers that do not deal with plain files, such as
      databases and servers.

      @note

      It'd be nice to eventually be raster/vector neutral.
    */
    virtual QString fileVectorFilters() const
    {
      return "";
    }


    /** return raster file filter string

      Returns a string suitable for a QFileDialog of raster file formats
      supported by the data provider.  Naturally this will be an empty string
      for those data providers that do not deal with plain files, such as
      databases and servers.

      @note

      It'd be nice to eventually be raster/vector neutral.
    */
    virtual QString fileRasterFilters() const
    {
      return "";
    }

    /**Reloads the data from the the source. Needs to be implemented by providers with data caches to
      synchronize with changes in the data source*/
    virtual void reloadData() {}

    /** Time stamp of data source in the moment when data/metadata were loaded by provider */
    virtual QDateTime timestamp() const { return mTimestamp; }

    /** Current time stamp of data source */
    virtual QDateTime dataTimestamp() const { return QDateTime(); }

  signals:

    /**
     *   This is emitted whenever the worker thread has fully calculated the
     *   PostGIS extents for this layer, and its event has been received by this
     *   provider.
     */
    void fullExtentCalculated();

    /**
     *   This is emitted whenever an asynchronous operation has finished
     *   and the data should be redrawn
     *   @note added in 1.5
     */
    void dataChanged();

    /**
     *   This is emitted whenever data or metadata (e.g. color table, extent) has changed
     *   @param changed binary combination of changes
     *   @note added in 1.7
     */
    void dataChanged( int changed );

  protected:
    /**
    * Timestamp of data in the moment when the data were loaded by provider.
    */
    QDateTime mTimestamp;
  private:

    /**
     * Universal Resource Identifier for source data.
     * This could be a file, database, or server address.
     */
    QString mDataSourceURI;
};


#endif
