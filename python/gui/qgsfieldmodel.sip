
/**
 * @brief The QgsFieldModel class is a model to display the list of fields of a layer in widgets.
 * If allowed, expressions might be added to the end of the model.
 * It can be associated with a QgsMapLayerModel to dynamically display a layer and its fields.
 * @note added in 2.3
 */
class QgsFieldModel : QAbstractItemModel
{
%TypeHeaderCode
#include "qgsfieldmodel.h"
%End

  public:
    enum FieldRoles {
      FieldNameRole = 33, /* Qt::UserRole + 1, SIP does not accept any arithmetic" */
      FieldIndexRole = 34,
      ExpressionRole = 35
    };

    /**
     * @brief QgsFieldModel creates a model to display the fields of a given layer
     */
    explicit QgsFieldModel( QObject *parent /TransferThis/ = 0 );

    /**
     * @brief indexFromName returns the index corresponding to a given fieldName
     */
    QModelIndex indexFromName( QString fieldName );
    
    /**
     * @brief setAllowExpression determines if expressions are allowed to be added to the model
     */
    void setAllowExpression( bool allowExpression );
    bool allowExpression();

    /**
     * @brief setExpression sets a single expression to be added after the fields at the end of the model
     * @return the model index of the newly added expression
     */
    QModelIndex setExpression( QString expression );

    /**
     * @brief layer returns the currently used layer
     */
    QgsMapLayer* layer();

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
