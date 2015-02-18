class QgsDbFilterProxyModel: QSortFilterProxyModel
{
%TypeHeaderCode
#include <qgsdbfilterproxymodel.h>
%End
  public:
    QgsDbFilterProxyModel( QObject* parent /TransferThis/ = 0 );
    ~QgsDbFilterProxyModel();
    /**Calls QSortFilterProxyModel::setFilterWildcard and triggers update*/
    void _setFilterWildcard( const QString& pattern );
    /**Calls QSortFilterProxyModel::setFilterRegExp and triggers update*/
    void _setFilterRegExp( const QString& pattern );

  protected:
    virtual bool filterAcceptsRow( int row, const QModelIndex & source_parent ) const;
};
