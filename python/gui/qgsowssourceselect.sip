
/*!
 * \brief  Dialog to create connections and add layers from WMS, WFS, WCS etc.
 *
 * This dialog allows the user to define and save connection information
 * for WMS servers, etc.
 *
 * The user can then connect and add
 * layers from the WMS server to the map canvas.
 */
class QgsOWSSourceSelect : QDialog
{
%TypeHeaderCode
#include <qgsowssourceselect.h>
%End

  public:
    /** Formats supported by provider */
    struct SupportedFormat
    {
      QString format;
      QString label;
    };

    //! Constructor
    QgsOWSSourceSelect( QString service, QWidget *parent /TransferThis/ = 0, Qt::WindowFlags fl = QgisGui::ModalDialogFlags, bool managerMode = false, bool embeddedMode = false );
    //! Destructor
    ~QgsOWSSourceSelect();

  public slots:

    //! Opens the create connection dialog to build a new connection
    void on_mNewButton_clicked();
    //! Opens a dialog to edit an existing connection
    void on_mEditButton_clicked();
    //! Deletes the selected connection
    void on_mDeleteButton_clicked();
    //! Saves connections to the file
    void on_mSaveButton_clicked();
    //! Loads connections from the file
    void on_mLoadButton_clicked();

    /** Connects to the database using the stored connection parameters.
    * Once connected, available layers are displayed.
    */
    void on_mConnectButton_clicked();

    //! Determines the layers the user selected
    virtual void addClicked();

    void searchFinished();

    //! Opens the Spatial Reference System dialog.
    void on_mChangeCRSButton_clicked();

    //! Signaled when a layer selection is changed.
    virtual void on_mLayersTreeWidget_itemSelectionChanged();

    //! Set status message to theMessage
    void showStatusMessage( const QString &theMessage );

    //! show whatever error is exposed.
    void showError( const QString &theTitle, const QString &theFormat, const QString &theError );

    //! Stores the selected datasource whenerver it is changed
    void on_mConnectionsComboBox_activated( int );

    //! Add some default wms servers to the list
    void on_mAddDefaultButton_clicked();

    void on_mDialogButtonBox_helpRequested();

  signals:
    void addRasterLayer( const QString & rasterLayerPath,
                         const QString & baseName,
                         const QString & providerKey );
    void connectionsChanged();

  protected:
    /**
     * List of image formats (encodings) supported by provider
     * @return list of format/label pairs
     */
    virtual QList<QgsOWSSourceSelect::SupportedFormat> providerFormats();

    //! List of formats supported for currently selected layer item(s)
    virtual QStringList selectedLayersFormats();

    //! Server CRS supported for currently selected layer item(s)
    virtual QStringList selectedLayersCRSs();

    //! List of times (temporalDomain timePosition/timePeriod for currently selected layer item(s)
    virtual QStringList selectedLayersTimes();

    //virtual QStringList layerCRS( int id );

    //! Populate the connection list combo box
    void populateConnectionList();

    //! Populate supported formats
    void populateFormats();

    //! Clear previously set formats
    void clearFormats();

    //! Set supported CRSs
    void populateCRS();

    //! Clear CRSs
    void clearCRS();

    //! Populate times
    void populateTimes();

    //! Clear times
    void clearTimes();

    //! Connection name
    QString connName();

    //! Connection info (uri)
    QString connectionInfo();

    //! Set the server connection combo box to that stored in the config file.
    void setConnectionListPosition();

    //! Add a few example servers to the list.
    void addDefaultServers();

    /**
     * \brief Populate the layer list.
     *
     * \retval false if the layers could not be retrieved or parsed
     */
    virtual void populateLayerList();

    //! create an item including possible parents
    //! @note not available in python bindings
    /*
    QgsNumericSortTreeWidgetItem *createItem( int id,
        const QStringList &names,
        QMap<int, QgsNumericSortTreeWidgetItem *> &items,
        int &layerAndStyleCount,
        const QMap<int, int> &layerParents,
        const QMap<int, QStringList> &layerParentNames ) /Factory/;
    */

    //! Returns a textual description for the authority id
    QString descriptionForAuthId( QString authId );

    void addWMSListRow( const QDomElement& item, int row );
    void addWMSListItem( const QDomElement& el, int row, int column );

    virtual void enableLayersForCrs( QTreeWidgetItem *item );

    //! Returns currently selected format
    QString selectedFormat();

    //! Returns currently selected Crs
    QString selectedCRS();

    //! Returns currently selected time
    QString selectedTime();

    //! Returns currently selected cache load control
    QNetworkRequest::CacheLoadControl selectedCacheLoadControl();

};
