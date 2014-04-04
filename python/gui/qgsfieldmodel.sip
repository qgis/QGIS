
/**
 * @brief The QgsFieldModel class is a model to display the list of fields of a layer in widgets.
 * It can be associated with a QgsMapLayerModel to dynamically display a layer and its fields.
 * @note added in 2.3
 */

class QgsFieldModel : QAbstractItemModel
{
%TypeHeaderCode
#include "qgsfieldmodel.h"
%End

  public:
    static const int FieldNameRole;
    static const int FieldIndexRole;

    /**
     * @brief QgsFieldModel creates a model to display the fields of a given layer
     */
    explicit QgsFieldModel( QObject *parent /TransferThis/ = 0 );

    /**
     * @brief indexFromName returns the index corresponding to a given fieldName
     */
    QModelIndex indexFromName( QString fieldName );

  public slots:
    /**
     * @brief setLayer sets the layer of whch fields are displayed
     */
    void setLayer( QgsMapLayer *layer );

    // QAbstractItemModel interface
  public:
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
    QModelIndex parent( const QModelIndex &child ) const;
    int rowCount( const QModelIndex &parent ) const;
    int columnCount( const QModelIndex &parent ) const;
    QVariant data( const QModelIndex &index, int role ) const;

};
