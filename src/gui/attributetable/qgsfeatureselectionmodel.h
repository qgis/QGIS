#ifndef QGSFEATURESELECTIONMODEL_H
#define QGSFEATURESELECTIONMODEL_H

#include <QItemSelectionModel>

#include "qgsfeature.h"

class QgsVectorLayer;
class QgsFeatureModel;

class QgsFeatureSelectionModel : public QItemSelectionModel
{
    Q_OBJECT
  public:
    explicit QgsFeatureSelectionModel( QAbstractItemModel* model, QgsFeatureModel* featureModel, QgsVectorLayer* layer, QObject* parent );

    /**
     * Enables or disables synchronisation to the {@link QgsVectorLayer}
     * When synchronisation is disabled, any selection change will be buffered inside this selection model.
     * When enabled, any buffered changes are communicated to the layer and the buffer is emptied.
     * Mainly to be used for performance reasons, because selection change on the layer can cost time as it
     * repaints the layer.
     *
     * @param enable The synchronisation status to set.
     */
    void enableSync( bool enable );

    /**
     * Returns the selection status of a given feature id.
     *
     * @param fid  The featureid to determine the selection status of
     *
     * @return The selection status
     */

    virtual bool isSelected( QgsFeatureId fid );
    /**
     * Returns the selection status of a given QModelIndex.
     *
     * @param index  The index to determine the selection status of
     *
     * @return The selection status
     */
    virtual bool isSelected( const QModelIndex& index );

  signals:
    /**
     * Request a repaint of a list of model indexes.
     * Views using this model should connect to and properly process this signal.
     *
     * @param indexes The model indexes which need to be repainted
     */
    void requestRepaint( QModelIndexList indexes );

    /**
     * Request a repaint of the visible items of connected views.
     * Views using this model should connect to and properly process this signal.
     */
    void requestRepaint();

  public slots:
    /**
     * Overwritten to do NOTHING (we handle selection ourselves)
     *
     * @see selectFeatures( const QItemSelection&, SelectionFlags )
     */
    virtual void select( const QModelIndex &index, SelectionFlags command ) { Q_UNUSED( index ); Q_UNUSED( command ); }

    /**
     * Overwritten to do NOTHING (we handle selection ourselves)
     *
     * @see selectFeatures( const QItemSelection&, SelectionFlags )
     */
    virtual void select( const QItemSelection &selection, SelectionFlags command ) { Q_UNUSED( selection ); Q_UNUSED( command ); }

    /**
     * Select features on this table. Is to be used in favor of the stock select methods.
     *
     * @param selection  The QItemSelection which will be selected
     * @param command    The command to apply. Select, Deselect and ClearAndSelect are processed.
     */
    virtual void selectFeatures( const QItemSelection &selection, SelectionFlags command );

  private slots:
    virtual void layerSelectionChanged( QgsFeatureIds selected, QgsFeatureIds deselected, bool clearAndSelect );

  private:
    QModelIndexList expandIndexToRow( const QModelIndex& index ) const;

  private:
    QgsFeatureModel* mFeatureModel;
    QgsVectorLayer* mLayer;
    bool mSyncEnabled;
    QgsFeatureIds mSelectedBuffer;
    QgsFeatureIds mDeselectedBuffer;
    bool mClearAndSelectBuffer;
};

#endif // QGSFEATURESELECTIONMODEL_H
