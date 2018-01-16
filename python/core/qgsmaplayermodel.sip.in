/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmaplayermodel.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsMapLayerModel : QAbstractItemModel
{
%Docstring
 The QgsMapLayerModel class is a model to display layers in widgets.

.. seealso:: :py:class:`QgsMapLayerProxyModel`

.. seealso:: :py:class:`QgsFieldModel`

.. versionadded:: 2.3
%End

%TypeHeaderCode
#include "qgsmaplayermodel.h"
%End
  public:

    enum ItemDataRole
    {
      LayerIdRole,
      LayerRole,
      EmptyRole,
      AdditionalRole,
    };

    explicit QgsMapLayerModel( QObject *parent /TransferThis/ = 0 );
%Docstring
 QgsMapLayerModel creates a model to display layers in widgets.
%End

    explicit QgsMapLayerModel( const QList<QgsMapLayer *> &layers, QObject *parent = 0 );
%Docstring
 QgsMapLayerModel creates a model to display a specific list of layers in a widget.
%End

    void setItemsCheckable( bool checkable );
%Docstring
 setItemsCheckable defines if layers should be selectable in the widget
%End

    void checkAll( Qt::CheckState checkState );
%Docstring
 checkAll changes the checkstate for all the layers
%End

    void setAllowEmptyLayer( bool allowEmpty );
%Docstring
Sets whether an optional empty layer ("not set") option is present in the model.

.. seealso:: :py:func:`allowEmptyLayer`

.. versionadded:: 3.0
%End

    bool allowEmptyLayer() const;
%Docstring
Returns true if the model allows the empty layer ("not set") choice.

.. seealso:: :py:func:`setAllowEmptyLayer`

.. versionadded:: 3.0
%End

    void setShowCrs( bool showCrs );
%Docstring
Sets whether the CRS of layers is also included in the model's display role.

.. seealso:: :py:func:`showCrs`

.. versionadded:: 3.0
%End

    bool showCrs() const;
%Docstring
Returns true if the model includes layer's CRS in the display role.

.. seealso:: :py:func:`setShowCrs`

.. versionadded:: 3.0
%End

    QList<QgsMapLayer *> layersChecked( Qt::CheckState checkState = Qt::Checked );
%Docstring
 layersChecked returns the list of layers which are checked (or unchecked)
%End
    bool itemsCheckable() const;
%Docstring
returns if the items can be checked or not
%End

    QModelIndex indexFromLayer( QgsMapLayer *layer ) const;
%Docstring
 indexFromLayer returns the model index for a given layer

.. seealso:: :py:func:`layerFromIndex`
%End

    QgsMapLayer *layerFromIndex( const QModelIndex &index ) const;
%Docstring
Returns the map layer corresponding to the specified ``index``.

.. versionadded:: 3.0

.. seealso:: :py:func:`indexFromLayer`
%End

    void setAdditionalItems( const QStringList &items );
%Docstring
Sets a list of additional (non map layer) items to include at the end of the model.
These may represent additional layers such as layers which are not included in the map
layer registry, or paths to layers which have not yet been loaded into QGIS.

.. seealso:: :py:func:`additionalItems`

.. versionadded:: 3.0
%End

    QStringList additionalItems() const;
%Docstring
Return the list of additional (non map layer) items included at the end of the model.

.. seealso:: :py:func:`setAdditionalItems`

.. versionadded:: 3.0
%End

    virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;

    virtual QModelIndex parent( const QModelIndex &child ) const;

    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;

    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;

    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;



    virtual bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );

    virtual Qt::ItemFlags flags( const QModelIndex &index ) const;


    static QIcon iconForLayer( QgsMapLayer *layer );
%Docstring
Returns the icon corresponding to a specified map ``layer``.

.. versionadded:: 3.0
%End

  protected slots:
    void removeLayers( const QStringList &layerIds );
    void addLayers( const QList<QgsMapLayer *> &layers );

  protected:

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmaplayermodel.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
