/**
 * @brief The QgsMapLayerProxModel class provides an easy to use model to display the list of layers in widgets.
 * @note added in 2.3
 */
class QgsFieldProxyModel : QSortFilterProxyModel
{

%TypeHeaderCode
#include "qgsfieldproxymodel.h"
%End

  public:
    enum Filter
    {
      String,
      Int,
      LongLong,
      Double,
      Numeric,
      Date,
      All
    };
    typedef QFlags<QgsFieldProxyModel::Filter> Filters;

    /**
     * @brief QgsFieldProxModel creates a proxy model with a QgsFieldModel as source model.
     * It can be used to filter the fields based on their types.
     */
    explicit QgsFieldProxyModel( QObject *parent /TransferThis/ = 0 );

    //! sourceFieldModel returns the QgsFieldModel used in this QSortFilterProxyModel
    QgsFieldModel* sourceFieldModel() ;

    /**
     * @brief setFilters set flags that affect how fields are filtered
     * @param filters are Filter flags
     * @note added in 2.3
     */
    QgsFieldProxyModel* setFilters( Filters filters );
    const Filters& filters() const;

    // QSortFilterProxyModel interface
  public:
    bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const;
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const;
};

QFlags<QgsFieldProxyModel::Filter> operator|(QgsFieldProxyModel::Filter f1, QFlags<QgsFieldProxyModel::Filter> f2);

