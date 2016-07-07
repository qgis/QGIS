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

#include "qgsfieldconditionalformatwidget.h"
#include "qgsattributeeditorcontext.h"
#include "qgsattributetablefiltermodel.h"
#include "qgscachedfeatureiterator.h"
#include "qgsdistancearea.h"
#include "qgsattributeform.h"

class QgsAttributeForm;
class QgsFeatureRequest;
class QSignalMapper;
class QgsMapLayerAction;
class QScrollArea;

/** \ingroup gui
 * This widget is used to show the attributes of a set of features of a {@link QgsVectorLayer}.
 * The attributes can be edited.
 * It supports two different layouts: the table layout, in which the attributes for the features
 * are shown in a table and the editor layout, where the features are shown as a selectable list
 * and the attributes for the currently selected feature are shown in a form.
 */
class GUI_EXPORT QgsDualView : public QStackedWidget, private Ui::QgsDualViewBase
{
    Q_OBJECT
    Q_ENUMS( ViewMode )

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

    /**
     * @brief Constructor
     * @param parent  The parent widget
     */
    explicit QgsDualView( QWidget* parent = nullptr );

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
    QgsAttributeTableFilterModel::FilterMode filterMode() { return mFilterModel->filterMode(); }

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
    QgsFeatureIds filteredFeatures() { return mFilterModel->filteredFeatures(); }

    /**
     * Returns the model which has the information about all features (not only filtered)
     *
     * @return The master model
     */
    QgsAttributeTableModel* masterModel() const { return mMasterModel; }

    /**
     * Set the request
     *
     * @param request The request
     */
    void setRequest( const QgsFeatureRequest& request );

    /**
     * Set the feature selection model
     *
     * @param featureSelectionManager the feature selection model
     */
    void setFeatureSelectionManager( QgsIFeatureSelectionManager* featureSelectionManager );

    /**
     * Returns the table view
     *
     * @return The table view
     */
    QgsAttributeTableView* tableView() { return mTableView; }

    /**
     * Set the attribute table config which should be used to control
     * the appearance of the attribute table.
     */
    void setAttributeTableConfig( const QgsAttributeTableConfig& config );

    /**
     * Set the expression used for sorting the table and feature list.
     */
    void setSortExpression( const QString& sortExpression , Qt::SortOrder sortOrder = Qt::AscendingOrder );

    /**
     * Get the expression used for sorting the table and feature list.
     */
    QString sortExpression() const;

  protected:
    /**
     * Initializes widgets which depend on the attributes of this layer
     */
    void columnBoxInit();

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

    /** Toggles whether search mode should be enabled in the form.
     * @param enabled set to true to switch on search mode
     * @note added in QGIS 2.16
     */
    void toggleSearchMode( bool enabled );

    /**
     * Copy the content of the selected cell in the clipboard.
     * @note added in QGIS 1.16
     */
    void copyCellContent() const;

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

    /** Is emitted when a filter expression is set using the view.
     * @param expression filter expression
     * @param type filter type
     * @note added in QGIS 2.16
     */
    void filterExpressionSet( const QString& expression, QgsAttributeForm::FilterType type );

    /** Emitted when the form changes mode.
     * @param mode new mode
     */
    void formModeChanged( QgsAttributeForm::Mode mode );

  private slots:

    void on_mFeatureList_aboutToChangeEditSelection( bool& ok );

    /**
     * Changes the currently visible feature within the attribute editor
     *
     * @param feat  The newly visible feature
     */
    void on_mFeatureList_currentEditSelectionChanged( const QgsFeature& feat );

    void previewExpressionBuilder();

    void previewColumnChanged( QObject* previewAction );

    void viewWillShowContextMenu( QMenu* menu, const QModelIndex& atIndex );

    void showViewHeaderMenu( QPoint point );

    void organizeColumns();

    void tableColumnResized( int column, int width );

    void hideColumn();

    void resizeColumn();

    void autosizeColumn();

    void modifySort();

    void previewExpressionChanged( const QString& expression );

    void onSortColumnChanged();

    void sortByPreviewExpression();

    /**
     * Will be called whenever the currently shown feature form changes.
     * Will forward this signal to the feature list to visually represent
     * that there has been an edit event.
     */
    void featureFormAttributeChanged();

    /**
     * Will be called periodically, when loading layers from slow data providers.
     *
     * @param i       The number of features already loaded
     * @param cancel  Set to true to cancel
     */
    virtual void progress( int i, bool &cancel );

    /**
     * Will be called, once all the features are loaded.
     * Use e.g. to close a dialog created from progress( int i, bool &cancel )
     */
    virtual void finished();

    /** Zooms to the active feature*/
    void zoomToCurrentFeature();

  private:
    void initLayerCache( QgsVectorLayer *layer, bool cacheGeometry );
    void initModels( QgsMapCanvas* mapCanvas, const QgsFeatureRequest& request );

    QgsAttributeEditorContext mEditorContext;
    QgsAttributeTableModel* mMasterModel;
    QgsAttributeTableFilterModel* mFilterModel;
    QgsFeatureListModel* mFeatureListModel;
    QgsAttributeForm* mAttributeForm;
    QSignalMapper* mPreviewActionMapper;
    QMenu* mPreviewColumnsMenu;
    QMenu* mHorizontalHeaderMenu;
    QgsVectorLayerCache* mLayerCache;
    QProgressDialog* mProgressDlg;
    QgsIFeatureSelectionManager* mFeatureSelectionManager;
    QgsDistanceArea mDistanceArea;
    QString mDisplayExpression;
    QgsAttributeTableConfig mConfig;
    QScrollArea* mAttributeEditorScrollArea;

    friend class TestQgsDualView;
};

/** \ingroup gui
 * \class QgsAttributeTableAction
 */
class GUI_EXPORT QgsAttributeTableAction : public QAction
{
    Q_OBJECT

  public:
    QgsAttributeTableAction( const QString &name, QgsDualView *dualView, int action, const QModelIndex &fieldIdx )
        : QAction( name, dualView )
        , mDualView( dualView )
        , mAction( action )
        , mFieldIdx( fieldIdx )
    {}

  public slots:
    void execute();
    void featureForm();

  private:
    QgsDualView* mDualView;
    int mAction;
    QModelIndex mFieldIdx;
};

/** \ingroup gui
 * \class QgsAttributeTableMapLayerAction
 */
class GUI_EXPORT QgsAttributeTableMapLayerAction : public QAction
{
    Q_OBJECT

  public:
    QgsAttributeTableMapLayerAction( const QString &name, QgsDualView *dualView, QgsMapLayerAction* action, const QModelIndex &fieldIdx )
        : QAction( name, dualView )
        , mDualView( dualView )
        , mAction( action )
        , mFieldIdx( fieldIdx )
    {}

  public slots:
    void execute();

  private:
    QgsDualView* mDualView;
    QgsMapLayerAction* mAction;
    QModelIndex mFieldIdx;
};

Q_DECLARE_METATYPE( QModelIndex );

#endif // QGSDUALVIEW_H
