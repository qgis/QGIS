class QgsDualView : QStackedWidget
{
%TypeHeaderCode
#include <qgsdualview.h>
%End

  public:

    /**
     * The view modes, in which this widget can present information.
     * Relates to the QStackedWidget stacks.
     *
     */
    enum ViewMode
    {
      /**
       * Shows the features and attributes in a table layout
       */
      AttributeTable,
      /**
       * Show a list of the features, where one can be chosen
       * and the according attribute dialog will be presented
       * in the neighbouring frame.
       */
      AttributeEditor
    };

    /**
     * @brief Constructor
     * @param parent  The parent widget
     */
    explicit QgsDualView( QWidget* parent /TransferThis/ = 0 );
    virtual ~QgsDualView();

    /**
     * Has to be called to initialize the dual view.
     *
     * @param layer      The layer which should be used to fetch features
     * @param mapCanvas  The mapCanvas (used for the FilterMode
     *                   {@link QgsAttributeTableFilterModel::ShowVisible}
     * @param request    Use a modified request to limit the shown features
     * @param context    The context in which this view is shown
     */
    void init( QgsVectorLayer* layer, QgsMapCanvas* mapCanvas, const QgsFeatureRequest& request = QgsFeatureRequest(), const QgsAttributeEditorContext& context = QgsAttributeEditorContext() );

    /**
     * Change the current view mode.
     *
     * @param view The view mode to set
     * @see view()
     */
    void setView( ViewMode view );

    /**
     * Returns the current view mode.
     * @see setView()
     * @note added in QGIS 2.16
     */
    ViewMode view() const;

    /**
     * Set the filter mode
     *
     * @param filterMode
     */
    void setFilterMode( QgsAttributeTableFilterModel::FilterMode filterMode );

    /**
     * Get the filter mode
     *
     * @return the filter mode
     */
    QgsAttributeTableFilterModel::FilterMode filterMode();

    /**
     * Toggle the selectedOnTop flag. If enabled, selected features will be moved to top.
     *
     * @param selectedOnTop True: Show selected features on top.
     *                      False: Use defined sorting column.
     */
    void setSelectedOnTop( bool selectedOnTop );

    /**
     * Returns the number of features on the layer.
     *
     * @return Number of features
     */
    int featureCount();

    /**
     * Returns the number of features which are currently visible, according to the
     * filter restrictions
     *
     * @return Number of features
     */
    int filteredFeatureCount();

    /**
     * Set a list of currently visible features
     *
     * @param filteredFeatures  A list of feature ids
     *
     */
    void setFilteredFeatures( const QgsFeatureIds& filteredFeatures );

    /**
     * Get a list of currently visible feature ids.
     */
    QgsFeatureIds filteredFeatures();

    /**
     * Returns the model which has the information about all features (not only filtered)
     *
     * @return The master model
     */
    QgsAttributeTableModel* masterModel() const;

    void setRequest( const QgsFeatureRequest& request );

    void setFeatureSelectionManager( QgsIFeatureSelectionManager* featureSelectionManager );

    QgsAttributeTableView* tableView();
    /**
     * Set the attribute table config which should be used to control
     * the appearance of the attribute table.
     */
    void setAttributeTableConfig( const QgsAttributeTableConfig& config );

  protected:
    /**
     * Initializes widgets which depend on the attributes of this layer
     */
    void columnBoxInit();

    virtual void hideEvent( QHideEvent * );
    virtual void focusOutEvent( QFocusEvent * );

  public slots:
    /**
     * @brief Set the current edit selection in the {@link AttributeEditor} mode.
     *
     * @param fids   A list of edited features (Currently only one at a time is supported)
     */
    void setCurrentEditSelection( const QgsFeatureIds& fids );

    /**
     * @brief saveEditChanges
     *
     * @return true if the saving was ok. false is possible due to connected
     *         validation logic.
     */
    bool saveEditChanges();

    void openConditionalStyles();

    /** Sets whether multi edit mode is enabled.
     * @note added in QGIS 2.16
     */
    void setMultiEditEnabled( bool enabled );

  signals:
    /**
     * Is emitted, whenever the display expression is successfully changed
     * @param expression The expression that was applied
     */
    void displayExpressionChanged( const QString& expression );

    /**
     * Is emitted, whenever the filter changes
     */
    void filterChanged();
};

class QgsAttributeTableAction : QAction
{
%TypeHeaderCode
#include <qgsdualview.h>
%End

  public:
    QgsAttributeTableAction( const QString &name, QgsDualView *dualView, int action, const QModelIndex &fieldIdx );

  public slots:
    void execute();
    void featureForm();
};

class QgsAttributeTableMapLayerAction : QAction
{
%TypeHeaderCode
#include <qgsdualview.h>
%End

  public:
    QgsAttributeTableMapLayerAction( const QString &name, QgsDualView *dualView, QgsMapLayerAction* action, const QModelIndex &fieldIdx );

  public slots:
    void execute();

};
