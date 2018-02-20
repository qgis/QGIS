/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmaplayerproxymodel.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsMapLayerProxyModel : QSortFilterProxyModel
{
%Docstring
 The QgsMapLayerProxyModel class provides an easy to use model to display the list of layers in widgets.

.. versionadded:: 2.3
%End

%TypeHeaderCode
#include "qgsmaplayerproxymodel.h"
%End
  public:
    enum Filter
    {
      RasterLayer,
      NoGeometry,
      PointLayer,
      LineLayer,
      PolygonLayer,
      HasGeometry,
      VectorLayer,
      PluginLayer,
      WritableLayer,
      All
    };
    typedef QFlags<QgsMapLayerProxyModel::Filter> Filters;


    explicit QgsMapLayerProxyModel( QObject *parent /TransferThis/ = 0 );
%Docstring
 QgsMapLayerProxModel creates a proxy model with a QgsMapLayerModel as source model.
It can be used to filter the layers list in a widget.
%End

    QgsMapLayerModel *sourceLayerModel() const;
%Docstring
 layerModel returns the QgsMapLayerModel used in this QSortFilterProxyModel
%End

    QgsMapLayerProxyModel *setFilters( QgsMapLayerProxyModel::Filters filters );
%Docstring
 setFilters set flags that affect how layers are filtered

:param filters: are Filter flags

.. versionadded:: 2.3
%End
    const Filters &filters() const;

    void setExceptedLayerList( const QList<QgsMapLayer *> &exceptList );
%Docstring
offer the possibility to except some layers to be listed
%End
    QList<QgsMapLayer *> exceptedLayerList();
%Docstring
Get the list of maplayers which are excluded from the list
%End

    void setExceptedLayerIds( const QStringList &ids );
%Docstring
Set the list of maplayer ids which are excluded from the list
%End
    QStringList exceptedLayerIds() const;
%Docstring
Get the list of maplayer ids which are excluded from the list
%End

    void setExcludedProviders( const QStringList &providers );
%Docstring
Sets a list of data providers which should be excluded from the model.

.. versionadded:: 3.0

.. seealso:: :py:func:`excludedProviders`
%End

    QStringList excludedProviders() const;
%Docstring
Returns the list of data providers which are excluded from the model.

.. seealso:: :py:func:`setExcludedProviders`

.. versionadded:: 3.0
%End

    virtual bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const;

    virtual bool lessThan( const QModelIndex &left, const QModelIndex &right ) const;


};

QFlags<QgsMapLayerProxyModel::Filter> operator|(QgsMapLayerProxyModel::Filter f1, QFlags<QgsMapLayerProxyModel::Filter> f2);


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmaplayerproxymodel.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
