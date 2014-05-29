/**
 * @brief The QgsMapLayerProxModel class provides an easy to use model to display the list of layers in widgets.
 * @note added in 2.3
 */
class QgsMapLayerProxyModel : QSortFilterProxyModel
{

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
      All
    };
    typedef QFlags<QgsMapLayerProxyModel::Filter> Filters;

    /**
     * @brief QgsMapLayerProxModel creates a proxy model with a QgsMapLayerModel as source model.
     * It can be used to filter the layers list in a widget.
     */
    explicit QgsMapLayerProxyModel( QObject *parent /TransferThis/ = 0 );

    /**
     * @brief layerModel returns the QgsMapLayerModel used in this QSortFilterProxyModel
     */
    QgsMapLayerModel* sourceLayerModel();

    /**
     * @brief setFilters set flags that affect how layers are filtered
     * @param filters are Filter flags
     * @note added in 2.3
     */
    QgsMapLayerProxyModel* setFilters( Filters filters );
    const Filters& filters() const;

    // QSortFilterProxyModel interface
  public:
    bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const;
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const;
};

