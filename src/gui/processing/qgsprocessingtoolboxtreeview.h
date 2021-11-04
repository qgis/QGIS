/***************************************************************************
    qgsprocessingtoolboxtreeview.h
    -----------------------------
    begin                : July 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGTOOLBOXTREEVIEW_H
#define QGSPROCESSINGTOOLBOXTREEVIEW_H

#include "qgis.h"
#include "qgis_gui.h"
#include <QTreeView>
#include "qgsprocessingtoolboxmodel.h"

class QgsProcessingRegistry;
class QgsProcessingRecentAlgorithmLog;
class QgsProcessingAlgorithm;

///@cond PRIVATE

/**
 * Processing toolbox tree view, showing algorithms and providers in a tree structure.
 * \ingroup gui
 * \warning Not part of stable API and may change in future QGIS releases.
 * \since QGIS 3.4
 */
class GUI_EXPORT QgsProcessingToolboxTreeView : public QTreeView
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProcessingToolboxTreeView, with the specified \a parent widget.
     *
     * If \a registry is set, then the view will automatically be populated with algorithms
     * and providers from the registry. Otherwise, users must manually call setRegistry()
     * to associate a registry with the view.
     *
     * If \a recentLog is specified then it will be used to create a "Recently used" top
     * level group containing recently used algorithms.
     */
    QgsProcessingToolboxTreeView( QWidget *parent SIP_TRANSFERTHIS = nullptr,
                                  QgsProcessingRegistry *registry = nullptr,
                                  QgsProcessingRecentAlgorithmLog *recentLog = nullptr );

    /**
     * Sets the processing \a registry associated with the view.
     *
     * If \a recentLog is specified then it will be used to create a "Recently used" top
     * level group containing recently used algorithms.
     */
    void setRegistry(
      QgsProcessingRegistry *registry,
      QgsProcessingRecentAlgorithmLog *recentLog = nullptr );

    /**
     * Sets the toolbox proxy model used to drive the view.
     */
    void setToolboxProxyModel( QgsProcessingToolboxProxyModel *model SIP_TRANSFER );

    /**
     * Returns the algorithm at the specified tree view \a index, or NULLPTR
     * if the index does not correspond to an algorithm.
     */
    const QgsProcessingAlgorithm *algorithmForIndex( const QModelIndex &index );

    /**
     * Returns the currently selected algorithm in the tree view, or NULLPTR
     * if no algorithm is currently selected.
     */
    const QgsProcessingAlgorithm *selectedAlgorithm();

    /**
     * Sets \a filters controlling the view's contents.
     * \see filters()
     */
    void setFilters( QgsProcessingToolboxProxyModel::Filters filters );

    /**
     * Returns the current filters controlling the view's contents.
     * \see setFilters()
     * \since QGIS 3.8
     */
    QgsProcessingToolboxProxyModel::Filters filters() const;

    /**
     * Sets the vector \a layer for the in-place algorithms
     */
    void setInPlaceLayer( QgsVectorLayer *layer );

  public slots:

    /**
     * Sets a \a filter string, used to filter out the contents of the view
     * to matching algorithms.
     */
    void setFilterString( const QString &filter );

  protected:

    void keyPressEvent( QKeyEvent *event ) override;

  private:

    QgsProcessingToolboxProxyModel *mModel = nullptr;
    QgsProcessingToolboxModel *mToolboxModel = nullptr;

    /**
     * Returns the first visible algorithm in the tree.
     */
    QModelIndex findFirstVisibleAlgorithm( const QModelIndex &parent );

    friend class TestQgsProcessingModel;

};

///@endcond
#endif // QGSPROCESSINGTOOLBOXTREEVIEW_H
