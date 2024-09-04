/***************************************************************************
    qgsdualview.h
     --------------------------------------
    Date                 : 10.2.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDUALVIEW_H
#define QGSDUALVIEW_H

#include <QStackedWidget>

#include "ui_qgsdualviewbase.h"

#include "qgsattributeeditorcontext.h"
#include "qgsattributetablefiltermodel.h"
#include "qgsattributeform.h"
#include "qgis_gui.h"

#include <QPointer>
#include <QUuid>

class QgsFeatureRequest;
class QgsMapLayerAction;
class QgsScrollArea;
class QgsFieldConditionalFormatWidget;

/**
 * \ingroup gui
 * \brief This widget is used to show the attributes of a set of features of a QgsVectorLayer.
 * The attributes can be edited.
 * It supports two different layouts: the table layout, in which the attributes for the features
 * are shown in a table and the editor layout, where the features are shown as a selectable list
 * and the attributes for the currently selected feature are shown in a form.
 */
class GUI_EXPORT QgsDualView : public QStackedWidget, private Ui::QgsDualViewBase
{
    Q_OBJECT

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
      AttributeTable = 0,

      /**
       * Show a list of the features, where one can be chosen
       * and the according attribute dialog will be presented
       * in the neighbouring frame.
       */
      AttributeEditor = 1
    };
    Q_ENUM( ViewMode )


    //! Action on the map canvas when browsing the list of features
    enum FeatureListBrowsingAction
    {
      NoAction = 0, //!< No action is done
      PanToFeature, //!< The map is panned to the center of the feature bounding-box
      ZoomToFeature, //!< The map is zoomed to contained the feature bounding-box
    };
    Q_ENUM( FeatureListBrowsingAction )

    /**
     * \brief Constructor
     * \param parent  The parent widget
     */
    explicit QgsDualView( QWidget *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsDualView() override;

    /**
     * Has to be called to initialize the dual view.
     *
     * \param layer      The layer which should be used to fetch features
     * \param mapCanvas  The mapCanvas (used for the FilterMode
     *                   QgsAttributeTableFilterModel::ShowVisible)
     * \param request    Use a modified request to limit the shown features
     * \param context    The context in which this view is shown
     * \param loadFeatures whether to initially load all features into the view. If set to
     *                   FALSE, limited features can later be loaded using setFilterMode()
     * \param showFirstFeature whether to initially show the first feature form upon initializing the dual view
     */
    void init( QgsVectorLayer *layer,
               QgsMapCanvas *mapCanvas,
               const QgsFeatureRequest &request = QgsFeatureRequest(),
               const QgsAttributeEditorContext &context = QgsAttributeEditorContext(),
               bool loadFeatures = true, bool showFirstFeature = true );

    /**
     * Change the current view mode.
     *
     * \param view The view mode to set
     * \see view()
     */
    void setView( ViewMode view );

    /**
     * Returns the current view mode.
     * \see setView()
     */
    ViewMode view() const;

    /**
     * Set the filter mode
     *
     * \param filterMode
     */
    void setFilterMode( QgsAttributeTableFilterModel::FilterMode filterMode );

    /**
     * Gets the filter mode
     *
     * \returns the filter mode
     */
    QgsAttributeTableFilterModel::FilterMode filterMode() { return mFilterModel->filterMode(); }

    /**
     * Toggle the selectedOnTop flag. If enabled, selected features will be moved to top.
     *
     * \param selectedOnTop TRUE: Show selected features on top.
     *                      FALSE: Use defined sorting column.
     */
    void setSelectedOnTop( bool selectedOnTop );

    /**
     * Returns the number of features on the layer.
     *
     * \returns Number of features
     */
    int featureCount();

    /**
     * Returns the number of features which are currently visible, according to the
     * filter restrictions
     *
     * \returns Number of features
     */
    int filteredFeatureCount();

    /**
     * Set a list of currently visible features
     *
     * \param filteredFeatures  A list of feature ids
     *
     * \deprecated QGIS 3.40. Since filterFeatures is handled in the attribute filter model itself.
    */
    Q_DECL_DEPRECATED void setFilteredFeatures( const QgsFeatureIds &filteredFeatures );

    /**
     * Sets the expression and Updates the filtered features in the filter model.
     * It is called when the filter expression changed.
     *
     * \since QGIS 3.10.3
     */
    void filterFeatures( const QgsExpression &filterExpression, const QgsExpressionContext &context );

    /**
     * Gets a list of currently visible feature ids.
     */
    QgsFeatureIds filteredFeatures() { return mFilterModel->filteredFeatures(); }

    /**
     * Returns the model which has the information about all features (not only filtered)
     *
     * \returns The master model
     */
    QgsAttributeTableModel *masterModel() const { return mMasterModel; }

    /**
     * Set the request
     *
     * \param request The request
     */
    void setRequest( const QgsFeatureRequest &request );

    /**
     * Set the feature selection model
     *
     * \param featureSelectionManager the feature selection model
     */
    void setFeatureSelectionManager( QgsIFeatureSelectionManager *featureSelectionManager );

    /**
     * Returns the table view
     *
     * \returns The table view
     */
    QgsAttributeTableView *tableView() { return mTableView; }

    /**
     * Set the attribute table config which should be used to control
     * the appearance of the attribute table.
     */
    void setAttributeTableConfig( const QgsAttributeTableConfig &config );

    /**
     * Set the expression used for sorting the table and feature list.
     */
    void setSortExpression( const QString &sortExpression, Qt::SortOrder sortOrder = Qt::AscendingOrder );

    /**
     * Gets the expression used for sorting the table and feature list.
     */
    QString sortExpression() const;

    /**
     * The config used for the attribute table.
     * \returns The config used for the attribute table.
     */
    QgsAttributeTableConfig attributeTableConfig() const;

    /**
     * Returns the list of required attributes according to the attribute table configuration of the \a layer,
     * only visible attributes and virtual fields referenced fields are returned.
     * \since QGIS 3.32
     */
    static QgsAttributeList requiredAttributes( const QgsVectorLayer *layer );

  public slots:

    /**
     * \brief Set the current edit selection in the AttributeEditor mode.
     *
     * \param fids   A list of edited features (Currently only one at a time is supported)
     */
    void setCurrentEditSelection( const QgsFeatureIds &fids );

    /**
     * \brief saveEditChanges
     *
     * \returns TRUE if the saving was OK. FALSE is possible due to connected
     *         validation logic.
     */
    bool saveEditChanges();

    void openConditionalStyles();

    /**
     * Sets whether multi edit mode is enabled.
     */
    void setMultiEditEnabled( bool enabled );

    /**
     * Toggles whether search mode should be enabled in the form.
     * \param enabled set to TRUE to switch on search mode
     */
    void toggleSearchMode( bool enabled );

    /**
     * Copy the content of the selected cell in the clipboard.
     */
    void copyCellContent() const;

    /**
     * Cancel the progress dialog (if any)
     */
    void cancelProgress( );

    /**
     * Called in embedded forms when an \a attribute \a value in the parent form has changed.
     *
     * Notify the form widgets that something has changed in case they
     * have filter expression that depend on the parent form scope.
     *
     * \since QGIS 3.14
     */
    void parentFormValueChanged( const QString &attribute, const QVariant &value );

  signals:

    /**
     * Emitted whenever the display expression is successfully changed
     * \param expression The expression that was applied
     */
    void displayExpressionChanged( const QString &expression );

    /**
     * Emitted whenever the filter changes
     */
    void filterChanged();

    /**
     * Emitted when a filter expression is set using the view.
     * \param expression filter expression
     * \param type filter type
     */
    void filterExpressionSet( const QString &expression, QgsAttributeForm::FilterType type );

    /**
     * Emitted when the form changes mode.
     * \param mode new mode
     */
    void formModeChanged( QgsAttributeEditorContext::Mode mode );

    /**
     * Emitted when selecting context menu on the feature list to create the context menu individually
     * \param menu context menu
     * \param fid feature id of the selected feature
     */
    void showContextMenuExternally( QgsActionMenu *menu, QgsFeatureId fid );

  protected:
    void hideEvent( QHideEvent *event ) override;

  private slots:

    void featureListAboutToChangeEditSelection( bool &ok );

    /**
     * Changes the currently visible feature within the attribute editor
     *
     * \param feat  The newly visible feature
     */
    void featureListCurrentEditSelectionChanged( const QgsFeature &feat );

    void previewExpressionBuilder();

    void previewColumnChanged( QAction *previewAction, const QString &expression );

    void viewWillShowContextMenu( QMenu *menu, const QModelIndex &atIndex );

    void widgetWillShowContextMenu( QgsActionMenu *menu, const QModelIndex &atIndex );

    void showViewHeaderMenu( QPoint point );

    void organizeColumns();

    void tableColumnResized( int column, int width );

    void hideColumn();

    void resizeColumn();

    void resizeAllColumns();

    void autosizeColumn();

    void autosizeAllColumns();

    void previewExpressionChanged( const QString &expression );

    void onSortColumnChanged();

    void updateSelectedFeatures();
    void updateEditedAddedFeatures();

    void extentChanged();

    /**
     * Will be called whenever the currently shown feature form changes.
     * Will forward this signal to the feature list to visually represent
     * that there has been an edit event.
     */
    void featureFormAttributeChanged( const QString &attribute, const QVariant &value, bool attributeChanged );

    /**
     * Will be called periodically, when loading layers from slow data providers.
     *
     * \param i       The number of features already loaded
     * \param cancel  Set to TRUE to cancel
     */
    virtual void progress( int i, bool &cancel );

    /**
     * Will be called, once all the features are loaded.
     * Use e.g. to close a dialog created from progress( int i, bool &cancel )
     */
    virtual void finished();

    //! Zooms to the active feature
    void zoomToCurrentFeature();
    //! Pans to the active feature
    void panToCurrentFeature();

    void flashCurrentFeature();

    void rebuildFullLayerCache();

    void panZoomGroupButtonToggled( QAbstractButton *button, bool checked );

    void flashButtonClicked( bool clicked );

    void filterError( const QString &errorMessage );

  private:

    /**
     * Initialize the attribute form to a given \a feature.
     *
     * \since QGIS 3.20
     */
    void initAttributeForm( const QgsFeature &feature );

    /**
     * Initializes widgets which depend on the attributes of this layer
     */
    void columnBoxInit();
    void initLayerCache( bool cacheGeometry );
    void initModels( QgsMapCanvas *mapCanvas, const QgsFeatureRequest &request, bool loadFeatures );
    void restoreRecentDisplayExpressions();
    void saveRecentDisplayExpressions() const;
    void setDisplayExpression( const QString &expression );
    void insertRecentlyUsedDisplayExpression( const QString &expression );
    void updateEditSelectionProgress( int progress, int count );
    void panOrZoomToFeature( const QgsFeatureIds &featureset );
    //! disable/enable the buttons of the browsing toolbar (feature list view)
    void setBrowsingAutoPanScaleAllowed( bool allowed );

    //! Returns TRUE if the expression dialog has been accepted
    bool modifySort();

    QgsFieldConditionalFormatWidget *mConditionalFormatWidget = nullptr;
    QgsAttributeEditorContext mEditorContext;
    QgsAttributeTableModel *mMasterModel = nullptr;
    QgsAttributeTableFilterModel *mFilterModel = nullptr;
    QgsFeatureListModel *mFeatureListModel = nullptr;
    QgsAttributeForm *mAttributeForm = nullptr;
    QMenu *mPreviewColumnsMenu = nullptr;
    QMenu *mPreviewActionMenu = nullptr;
    QAction *mLastDisplayExpressionAction = nullptr;
    QMenu *mHorizontalHeaderMenu = nullptr;
    QgsVectorLayerCache *mLayerCache = nullptr;
    QPointer< QgsVectorLayer > mLayer = nullptr;
    QProgressDialog *mProgressDlg = nullptr;
    QgsIFeatureSelectionManager *mFeatureSelectionManager = nullptr;
    QString mDisplayExpression;
    QgsAttributeTableConfig mConfig;
    QgsScrollArea *mAttributeEditorScrollArea = nullptr;
    QgsFeatureIds mLastFeatureSet;
    bool mBrowsingAutoPanScaleAllowed = true;
    ViewMode mPreviousView = AttributeTable;

    friend class TestQgsDualView;
    friend class TestQgsAttributeTable;
};

/**
 * \ingroup gui
 * \class QgsAttributeTableAction
 */
class GUI_EXPORT QgsAttributeTableAction : public QAction
{
    Q_OBJECT

  public:

    /**
     * Create a new attribute table action.
     *
     */
    QgsAttributeTableAction( const QString &name, QgsDualView *dualView, QUuid action, const QModelIndex &fieldIdx )
      : QAction( name, dualView )
      , mDualView( dualView )
      , mAction( action )
      , mFieldIdx( fieldIdx )
    {}

  public slots:
    void execute();
    void featureForm();

  private:
    QgsDualView *mDualView = nullptr;
    QUuid mAction;
    QModelIndex mFieldIdx;
};

/**
 * \ingroup gui
 * \class QgsAttributeTableMapLayerAction
 */
class GUI_EXPORT QgsAttributeTableMapLayerAction : public QAction
{
    Q_OBJECT

  public:
    QgsAttributeTableMapLayerAction( const QString &name, QgsDualView *dualView, QgsMapLayerAction *action, const QModelIndex &fieldIdx )
      : QAction( name, dualView )
      , mDualView( dualView )
      , mAction( action )
      , mFieldIdx( fieldIdx )
    {}

  public slots:
    void execute();

  private:
    QgsDualView *mDualView = nullptr;
    QgsMapLayerAction *mAction = nullptr;
    QModelIndex mFieldIdx;
};

Q_DECLARE_METATYPE( QModelIndex );

#endif // QGSDUALVIEW_H
