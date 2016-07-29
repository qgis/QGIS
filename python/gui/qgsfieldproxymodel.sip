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

    //! Field type filters
    enum Filter
    {
      String, /*!< String fields */
      Int, /*!< Integer fields */
      LongLong, /*!< Longlong fields */
      Double, /*!< Double fields */
      Numeric, /*!< All numeric fields */
      Date, /*!< Date or datetime fields */
      Time, /*!< Time fields */
      HideReadOnly,  /*!< Hide read-only fields */
      All, /*!< All field types */
    };
    typedef QFlags<QgsFieldProxyModel::Filter> Filters;

    /**
     * @brief QgsFieldProxModel creates a proxy model with a QgsFieldModel as source model.
     * It can be used to filter the fields based on their types.
     */
    explicit QgsFieldProxyModel( QObject *parent /TransferThis/ = 0 );

    //! Returns the QgsFieldModel used in this QSortFilterProxyModel
    QgsFieldModel* sourceFieldModel();

    /**
     * Set flags that affect how fields are filtered in the model.
     * @param filters are Filter flags
     * @see filters()
     */
    QgsFieldProxyModel* setFilters( const Filters& filters );

    /** Returns the filters controlling displayed fields.
     * @see setFilters()
     */
    const Filters& filters() const;

    // QSortFilterProxyModel interface
  public:
    bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const;
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const;
};

QFlags<QgsFieldProxyModel::Filter> operator|(QgsFieldProxyModel::Filter f1, QFlags<QgsFieldProxyModel::Filter> f2);

