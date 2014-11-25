

class QgsDataProvider : QObject
{
%TypeHeaderCode
#include <qgsdataprovider.h>
%End

%ConvertToSubClassCode
    if (sipCpp->inherits("QgsVectorDataProvider"))
    {
        sipClass = sipClass_QgsVectorDataProvider;
    }
    else if (sipCpp->inherits("QgsRasterDataProvider"))
    {
	sipClass = sipClass_QgsRasterDataProvider;
    }
    else
    {
        sipClass = 0;
    }
%End


  public:

    QgsDataProvider( const QString & uri = "" );

    /**
     * We need this so the subclass destructors get called
     */
    virtual ~QgsDataProvider();


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
    virtual void setDataSourceUri( const QString & uri );

    /**
     * Get the data source specification. This may be a path or database
     * connection string
     * @return data source specification
     */
    virtual QString dataSourceUri() const;


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
    virtual void updateExtents();


    /**
     * Set the subset string used to create a subset of features in
     * the layer. This may be a sql where clause or any other string
     * that can be used by the data provider to create a subset.
     * Must be implemented in the dataprovider.
     */
    virtual bool setSubsetString( QString subset, bool updateFeatureCount = true );

    /** provider supports setting of subset strings */
    virtual bool supportsSubsetString();

    /**
     * Returns the subset definition string (typically sql) currently in
     * use by the layer and used by the provider to limit the feature set.
     * Must be overridden in the dataprovider, otherwise returns a null
     * QString.
     */
    virtual QString subsetString();


    /**
     * Sub-layers handled by this provider, in order from bottom to top
     *
     * Sub-layers are used when the provider's source can combine layers
     * it knows about in some way before it hands them off to the provider.
     */
    virtual QStringList subLayers() const;


    /**
     * Sub-layer styles for each sub-layer handled by this provider,
     * in order from bottom to top
     *
     * Sub-layer styles are used to abstract the way the provider's source can symbolise
     * layers in some way at the server, before it serves them to the provider.
     */
    virtual QStringList subLayerStyles() const;


    /**
     * return the number of layers for the current data source
     */
    virtual uint subLayerCount() const;


    /**
     * Reorder the list of layer names to be rendered by this provider
     * (in order from bottom to top)
     * \note   layers must have been previously added.
     */
    virtual void setLayerOrder( const QStringList &layers );


    /**
     * Set the visibility of the given sublayer name
     */
    virtual void setSubLayerVisibility( const QString &name, bool vis );


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
    virtual QString fileVectorFilters() const;


    /** return raster file filter string

      Returns a string suitable for a QFileDialog of raster file formats
      supported by the data provider.  Naturally this will be an empty string
      for those data providers that do not deal with plain files, such as
      databases and servers.

      @note

      It'd be nice to eventually be raster/vector neutral.
    */
    virtual QString fileRasterFilters() const;

    /**Reloads the data from the source. Needs to be implemented by providers with data caches to
      synchronize with changes in the data source*/
    virtual void reloadData();

    /** Time stamp of data source in the moment when data/metadata were loaded by provider */
    virtual QDateTime timestamp() const;

    /** Current time stamp of data source */
    virtual QDateTime dataTimestamp() const;

    /** Get current status error. This error describes some principal problem
     *  for which provider cannot work and thus is not valid. It is not last error
     *  after accessing data by block(), identify() etc.
     */
    virtual QgsError error() const;

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
     */
    void dataChanged();

    /**
     *   This is emitted whenever data or metadata (e.g. color table, extent) has changed
     *   @param changed binary combination of changes
     */
    void dataChanged( int changed );

};
