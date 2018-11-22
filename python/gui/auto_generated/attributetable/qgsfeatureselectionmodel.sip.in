/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/attributetable/qgsfeatureselectionmodel.h                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsFeatureSelectionModel : QItemSelectionModel
{

%TypeHeaderCode
#include "qgsfeatureselectionmodel.h"
%End
  public:
    explicit QgsFeatureSelectionModel( QAbstractItemModel *model, QgsFeatureModel *featureModel, QgsIFeatureSelectionManager *featureSelectionHandler, QObject *parent /TransferThis/ );

    void enableSync( bool enable );
%Docstring
Enables or disables synchronisation to the :py:class:`QgsVectorLayer`
When synchronisation is disabled, any selection change will be buffered inside this selection model.
When enabled, any buffered changes are communicated to the layer and the buffer is emptied.
Mainly to be used for performance reasons, because selection change on the layer can cost time as it
repaints the layer.

:param enable: The synchronisation status to set.
%End


    virtual bool isSelected( QgsFeatureId fid );
%Docstring
Returns the selection status of a given feature id.

:param fid: The featureid to determine the selection status of

:return: The selection status
%End

    virtual bool isSelected( const QModelIndex &index );
%Docstring
Returns the selection status of a given QModelIndex.

:param index: The index to determine the selection status of

:return: The selection status
%End

  signals:

    void requestRepaint( const QModelIndexList &indexes );
%Docstring
Request a repaint of a list of model indexes.
Views using this model should connect to and properly process this signal.

:param indexes: The model indexes which need to be repainted
%End

    void requestRepaint();
%Docstring
Request a repaint of the visible items of connected views.
Views using this model should connect to and properly process this signal.
%End

  public slots:

    virtual void select( const QModelIndex &index, QItemSelectionModel::SelectionFlags command );
%Docstring
Overwritten to do NOTHING (we handle selection ourselves)

.. seealso:: :py:func:`selectFeatures`
%End

    virtual void select( const QItemSelection &selection, QItemSelectionModel::SelectionFlags command );
%Docstring
Overwritten to do NOTHING (we handle selection ourselves)

.. seealso:: :py:func:`selectFeatures`
%End

    virtual void selectFeatures( const QItemSelection &selection, QItemSelectionModel::SelectionFlags command );
%Docstring
Select features on this table. Is to be used in favor of the stock select methods.

:param selection: The QItemSelection which will be selected
:param command: The command to apply. Select, Deselect and ClearAndSelect are processed.
%End

    virtual void setFeatureSelectionManager( QgsIFeatureSelectionManager *featureSelectionManager /Transfer/ );

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/attributetable/qgsfeatureselectionmodel.h                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
