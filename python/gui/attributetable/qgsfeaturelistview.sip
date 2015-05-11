class QgsFeatureListView : QListView
{
%TypeHeaderCode
#include <qgsfeaturelistview.h>
%End
  public:
    /**
     * Creates a feature list view
     *
     * @param parent   owner
     */
    explicit QgsFeatureListView( QWidget* parent /TransferThis/ = 0 );

    /**
     * Destructor
     */
    virtual ~QgsFeatureListView();

    /**
     * Returns the layer cache
     * @return the layer cache used as backend
     */
    QgsVectorLayerCache* layerCache();

    /**
     * Set the {@link QgsFeatureListModel} which is used to retrieve information
     *
     * @param featureListModel  The model to use
     */
    virtual void setModel( QgsFeatureListModel* featureListModel );
    /**
     * Get the featureListModel used by this view
     *
     * @return The model in use
     */
    QgsFeatureListModel* featureListModel();

    /**
     * The display expression is an expression used to render the fields into a single string
     * which is displaied.
     *
     * @param displayExpression  The expression used to render the feature
     *
     * @see QgsExpression
     */
    bool setDisplayExpression( const QString displayExpression );

    /**
     * Returns the expression which is currently used to render the features.
     *
     * @return A string containing the currend display expression
     *
     * @see QgsExpression
     */
    const QString displayExpression() const;

    /**
     * Returns a detailed message about errors while parsing a QgsExpression.
     *
     * @return A message containg information about the parser error.
     */
    QString parserErrorString();

    /**
     * Get the currentEditSelection
     *
     * @return A list of edited feature ids
     */
    QgsFeatureIds currentEditSelection();

    /**
     * Sets if the currently shown form has received any edit events so far.
     *
     * @param state The state
     */
    void setCurrentFeatureEdited( bool state );

  protected:
    virtual void mouseMoveEvent( QMouseEvent *event );
    virtual void mousePressEvent( QMouseEvent *event );
    virtual void mouseReleaseEvent( QMouseEvent *event );
    virtual void keyPressEvent( QKeyEvent *event );
    virtual void contextMenuEvent( QContextMenuEvent *event );

  signals:
    /**
     * Is emitted, whenever the current edit selection has been changed.
     *
     * @param feat the feature, which will be edited.
     */
    void currentEditSelectionChanged( QgsFeature &feat );

    /**
     * Is emitted, whenever the display expression is successfully changed
     * @param expression The expression that was applied
     */
    void displayExpressionChanged( const QString expression );

    //void aboutToChangeEditSelection( bool& ok );

  public slots:
    /**
     * Set the feature(s) to be edited
     *
     * @param fids  A list of features to be edited
     */
    void setEditSelection( const QgsFeatureIds &fids );

    /**
     * Set the feature(s) to be edited
     *
     * @param index The selection to set
     * @param command selection update mode
     */
    void setEditSelection( const QModelIndex& index, QItemSelectionModel::SelectionFlags command );

    /**
     * Select all currently visible features
     */
    virtual void selectAll();

    void repaintRequested( QModelIndexList indexes );
    void repaintRequested();
};
