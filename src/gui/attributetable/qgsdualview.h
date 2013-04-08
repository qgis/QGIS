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
 * @brief
 */
class GUI_EXPORT QgsDualView : public QStackedWidget, private Ui::QgsDualViewBase
{
    Q_OBJECT

  public:

    /**
     * @brief
     *
     */
    enum ViewMode
    {
      AttributeTable = 0,
      AttributeEditor = 1
    };

    /**
     * @brief
     *
     * @param parent
     */
    explicit QgsDualView( QWidget* parent = 0 );
    virtual ~QgsDualView();

    void init( QgsVectorLayer* layer, QgsMapCanvas* mapCanvas, QgsDistanceArea myDa );
    void columnBoxInit();

    /**
     * @brief
     *
     * @param view
     */
    void setView( ViewMode view );

    /**
     * @brief
     *
     * @param filterMode
     */
    void setFilterMode( QgsAttributeTableFilterModel::FilterMode filterMode );

    void setSelectedOnTop( bool selectedOnTop );

    /**
     * @brief
     *
     * @param featureRequest
     */
    void setFeatureRequest( const QgsFeatureRequest* featureRequest );

    /**
     * @brief
     *
     */
    void setSelectionMode();

    int featureCount();

    int filteredFeatureCount();

    void setFilteredFeatures( QgsFeatureIds filteredFeatures );

    QgsAttributeTableModel* masterModel() const { return mMasterModel; }

  public slots:
    void  setCurrentEditSelection( const QgsFeatureIds& fids );

  signals:
    void filterChanged();

  private slots:
    /**
     * Changes the currently visible feature within the attribute editor
     */
    void on_mFeatureList_currentEditSelectionChanged( const QgsFeature &feat );

    void previewExpressionBuilder();

    void previewColumnChanged( QObject* previewAction );

    void editingToggled();

    void viewWillShowContextMenu( QMenu* menu, QModelIndex atIndex );

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
