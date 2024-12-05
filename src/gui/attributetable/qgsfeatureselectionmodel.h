/***************************************************************************
    qgsfeatureselectionmodel.h
    ---------------------
    begin                : April 2013
    copyright            : (C) 2013 by Matthias Kuhn
    email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFEATURESELECTIONMODEL_H
#define QGSFEATURESELECTIONMODEL_H

#include <QItemSelectionModel>
#include "qgsfeatureid.h"

#include "qgis_gui.h"

class QgsVectorLayer;
class QgsFeatureModel;
class QgsIFeatureSelectionManager;

/**
 * \ingroup gui
 * \class QgsFeatureSelectionModel
 */
class GUI_EXPORT QgsFeatureSelectionModel : public QItemSelectionModel
{
    Q_OBJECT
  public:
    explicit QgsFeatureSelectionModel( QAbstractItemModel *model, QgsFeatureModel *featureModel, QgsIFeatureSelectionManager *featureSelectionHandler, QObject *parent SIP_TRANSFERTHIS );

    /**
     * Enables or disables synchronisation to the QgsVectorLayer
     * When synchronisation is disabled, any selection change will be buffered inside this selection model.
     * When enabled, any buffered changes are communicated to the layer and the buffer is emptied.
     * Mainly to be used for performance reasons, because selection change on the layer can cost time as it
     * repaints the layer.
     *
     * \param enable The synchronisation status to set.
     */
    void enableSync( bool enable );

    /**
     * Returns the selection status of a given feature id.
     *
     * \param fid  The featureid to determine the selection status of
     *
     * \returns The selection status
     */

    virtual bool isSelected( QgsFeatureId fid );

    /**
     * Returns the selection status of a given QModelIndex.
     *
     * \param index  The index to determine the selection status of
     *
     * \returns The selection status
     */
    virtual bool isSelected( const QModelIndex &index );

  signals:

    /**
     * Request a repaint of a list of model indexes.
     * Views using this model should connect to and properly process this signal.
     *
     * \param indexes The model indexes which need to be repainted
     */
    void requestRepaint( const QModelIndexList &indexes );

    /**
     * Request a repaint of the visible items of connected views.
     * Views using this model should connect to and properly process this signal.
     */
    void requestRepaint();

  public slots:

    /**
     * Overwritten to do NOTHING (we handle selection ourselves)
     *
     * \see selectFeatures( const QItemSelection&, SelectionFlags )
     */
    void select( const QModelIndex &index, QItemSelectionModel::SelectionFlags command ) override
    {
      Q_UNUSED( index )
      Q_UNUSED( command );
    }

    /**
     * Overwritten to do NOTHING (we handle selection ourselves)
     *
     * \see selectFeatures( const QItemSelection&, SelectionFlags )
     */
    void select( const QItemSelection &selection, QItemSelectionModel::SelectionFlags command ) override
    {
      Q_UNUSED( selection )
      Q_UNUSED( command );
    }

    /**
     * Select features on this table. Is to be used in favor of the stock select methods.
     *
     * \param selection  The QItemSelection which will be selected
     * \param command    The command to apply. Select, Deselect and ClearAndSelect are processed.
     */
    virtual void selectFeatures( const QItemSelection &selection, QItemSelectionModel::SelectionFlags command );

    virtual void setFeatureSelectionManager( QgsIFeatureSelectionManager *featureSelectionManager SIP_TRANSFER );

  private slots:
    virtual void layerSelectionChanged( const QgsFeatureIds &selected, const QgsFeatureIds &deselected, bool clearAndSelect );

  private:
    QModelIndexList expandIndexToRow( const QModelIndex &index ) const;

  private:
    QgsFeatureModel *mFeatureModel = nullptr;
    QgsIFeatureSelectionManager *mFeatureSelectionManager = nullptr;
    bool mSyncEnabled;

    /**
     * If sync is disabled
     * Holds a list of newly selected features which will be synced when re-enabled
     */
    QgsFeatureIds mSelectedBuffer;

    /**
     * If sync is disabled
     * Holds a list of newly deselected features which will be synced when re-enabled
     */
    QgsFeatureIds mDeselectedBuffer;

    /**
     * If sync is disabled
     * Is set to TRUE, if a clear and select operation should be performed before syncing
     */
    bool mClearAndSelectBuffer;
};

#endif // QGSFEATURESELECTIONMODEL_H
