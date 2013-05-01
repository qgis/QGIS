/***************************************************************************
    qgsdualview.h
     --------------------------------------
    Date                 : 10.2.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFEATURELIST_H
#define QGSFEATURELIST_H

#include <QStackedWidget>

#include "qgsdistancearea.h"
#include "qgsattributetablefiltermodel.h"
#include "qgscachedfeatureiterator.h"
#include "ui_qgsdualviewbase.h"

class QgsFeatureRequest;
class QgsAttributeDialog;
class QSignalMapper;

/**
 * This widget is used to show the attributes of a set of features of a {@link QgsVectorLayer}.
 * The attributes can be edited.
 * It supports two different layouts: the table layout, in which the attributes for the features
 * are shown in a table and the editor layout, where the features are shown as a selectable list
 * and the attributes for the currently selected feature are shown in a form.
 *
 * @note Added in 2.0
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

    /**
     * @brief Constructor
     * @param parent  The parent widget
     */
    explicit QgsDualView( QWidget* parent = 0 );
    virtual ~QgsDualView();

    /**
     * Has to be called to initialize the dual view.
     *
     * @param layer      The layer which should be used to fetch features
     * @param mapCanvas  The mapCanvas (used for the FilterMode
     *                   {@link QgsAttributeTableFilterModel::ShowVisible}
     * @param myDa       Used for attribute dialog creation
     */
    void init( QgsVectorLayer* layer, QgsMapCanvas* mapCanvas, QgsDistanceArea myDa );

    /**
     * Change the current view mode.
     *
     * @param view The view mode to set
     */
    void setView( ViewMode view );

    /**
     * Set the filter mode
     *
     * @param filterMode
     */
    void setFilterMode( QgsAttributeTableFilterModel::FilterMode filterMode );

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
    void setFilteredFeatures( QgsFeatureIds filteredFeatures );

    /**
     * Returns the model which has the information about all features (not only filtered)
     *
     * @return The master model
     */
    QgsAttributeTableModel* masterModel() const { return mMasterModel; }

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
     */

    void saveEditChanges();

  signals:
    /**
     * Is emitted, whenever the display expression is successfully changed
     * @param expression The expression that was applied
     */
    void displayExpressionChanged( const QString expression );

    /**
     * Is emitted, whenever the filter changes
     */
    void filterChanged();

  private slots:
    /**
     * Changes the currently visible feature within the attribute editor
     *
     * @param feat  The newly visible feature
     */
    void on_mFeatureList_currentEditSelectionChanged( const QgsFeature &feat );

    void previewExpressionBuilder();

    void previewColumnChanged( QObject* previewAction );

    void editingToggled();

    void viewWillShowContextMenu( QMenu* menu, QModelIndex atIndex );

    void previewExpressionChanged( const QString expression );

    /**
     * Will be called periodically, when loading layers from slow data providers.
     *
     * @param i       The number of features already loaded
     * @param cancel  Set to true to cancel
     */
    virtual void progress( int i, bool& cancel );

    /**
     * Will be called, once all the features are loaded.
     * Use e.g. to close a dialog created from {@link progress(int i,bool& cancel )}
     */
    virtual void finished();

  private:
    void initLayerCache( QgsVectorLayer *layer );
    void initModels( QgsMapCanvas* mapCanvas );

    QgsAttributeTableModel* mMasterModel;
    QgsAttributeTableFilterModel* mFilterModel;
    QgsFeatureListModel* mFeatureListModel;
    QgsAttributeDialog* mAttributeDialog;
    QgsCachedFeatureIterator* mFeatureCache;
    QSignalMapper* mPreviewActionMapper;
    QMenu* mPreviewColumnsMenu;
    QgsVectorLayerCache* mLayerCache;
    QProgressDialog* mProgressDlg;

    QgsDistanceArea mDistanceArea;

    friend class TestQgsDualView;
};

class QgsAttributeTableAction : public QAction
{
    Q_OBJECT

  public:
    QgsAttributeTableAction( const QString &name, QgsDualView *dualView, int action, const QModelIndex &fieldIdx ) :
        QAction( name, dualView ), mDualView( dualView ), mAction( action ), mFieldIdx( fieldIdx )
    {}

  public slots:
    void execute();
    void featureForm();

  private:
    QgsDualView* mDualView;
    int mAction;
    QModelIndex mFieldIdx;
};

#endif // QGSFEATURELIST_H
