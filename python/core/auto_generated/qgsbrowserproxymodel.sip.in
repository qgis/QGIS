/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsbrowserproxymodel.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsBrowserProxyModel : QSortFilterProxyModel
{
%Docstring
A QSortFilterProxyModel subclass for filtering and sorting browser model items.

.. versionadded:: 3.4
%End

%TypeHeaderCode
#include "qgsbrowserproxymodel.h"
%End
  public:

    enum FilterSyntax
    {
      Normal,
      Wildcards,
      RegularExpression,
    };

    explicit QgsBrowserProxyModel( QObject *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsBrowserProxyModel, with the specified ``parent`` object.
%End

    void setBrowserModel( QgsBrowserModel *model );
%Docstring
Sets the underlying browser ``model``.

.. seealso:: :py:func:`browserModel`
%End

    QgsBrowserModel *browserModel();
%Docstring
Returns the underlying browser model.

.. seealso:: :py:func:`setBrowserModel`
%End

    QgsDataItem *dataItem( const QModelIndex &index ) const;
%Docstring
Returns the data item at the specified proxy ``index``, or ``None`` if no item
exists at the index.
%End

    void setFilterSyntax( FilterSyntax syntax );
%Docstring
Sets the filter ``syntax``.

.. seealso:: :py:func:`filterSyntax`
%End

    FilterSyntax filterSyntax() const;
%Docstring
Returns the filter syntax.

.. seealso:: :py:func:`setFilterSyntax`
%End

    void setFilterString( const QString &filter );
%Docstring
Sets the ``filter`` string to use when filtering items in the model.

.. seealso:: :py:func:`filterString`
%End

    QString filterString() const;
%Docstring
Returns the filter string used when filtering items in the model.

.. seealso:: :py:func:`setFilterString`
%End

    void setFilterCaseSensitivity( Qt::CaseSensitivity sensitivity );
%Docstring
Sets whether item filtering should be case sensitive.

.. seealso:: :py:func:`caseSensitivity`
%End

    Qt::CaseSensitivity caseSensitivity() const;
%Docstring
Returns whether item filtering is case sensitive.

.. seealso:: :py:func:`setFilterCaseSensitivity`
%End

    bool filterByLayerType() const;
%Docstring
Returns ``True`` if the model is filtered by map layer type.

.. seealso:: :py:func:`layerType`

.. seealso:: :py:func:`setFilterByLayerType`
%End

    void setFilterByLayerType( bool enabled );
%Docstring
Sets whether the model is filtered by map layer type.

.. seealso:: :py:func:`filterByLayerType`

.. seealso:: :py:func:`setLayerType`
%End

    QgsMapLayerType layerType() const;
%Docstring
Returns the layer type to filter the model by. This is only used if
filterByLayerType() is ``True``.

.. seealso:: :py:func:`setLayerType`

.. seealso:: :py:func:`filterByLayerType`
%End

    void setLayerType( QgsMapLayerType type );
%Docstring
Sets the layer ``type`` to filter the model by. This is only used if
filterByLayerType() is ``True``.

.. seealso:: :py:func:`layerType`

.. seealso:: :py:func:`setFilterByLayerType`
%End

  protected:

    virtual bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const;


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsbrowserproxymodel.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
