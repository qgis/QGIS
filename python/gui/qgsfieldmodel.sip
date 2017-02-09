
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

    //! Roles utilised by the model
    enum FieldRoles
    {
      FieldNameRole, /*!< return field name if index corresponds to a field */
      FieldIndexRole, /*!< return field index if index corresponds to a field */
      ExpressionRole, /*!< return field name or expression */
      IsExpressionRole, /*!< return if index corresponds to an expression */
      ExpressionValidityRole, /*!< return if expression is valid or not */
      FieldTypeRole, /*!< return the field type (if a field, return QVariant if expression) */
      FieldOriginRole, /*!< return the field origin (if a field, returns QVariant if expression) */
      IsEmptyRole, //!< Return if the index corresponds to the empty value
    };

    /**
     * Constructor for QgsFieldModel - creates a model to display the fields of a given layer.
     */
    explicit QgsFieldModel( QObject *parent /TransferThis/ = 0 );

    /**
     * Returns the index corresponding to a given fieldName.
     */
    QModelIndex indexFromName( const QString &fieldName );

    /**
     * Sets whether custom expressions are accepted and displayed in the model.
     * @see allowExpression()
     * @see setExpression()
     */
    void setAllowExpression( bool allowExpression );

    /**
     * Returns true if the model allows custom expressions to be created and displayed.
     * @see setAllowExpression()
     */
    bool allowExpression();

    /**
     * Sets whether an optional empty field ("not set") option is present in the model.
     * @see allowEmptyFieldName()
     * @note added in QGIS 3.0
     */
    void setAllowEmptyFieldName( bool allowEmpty );

    /**
     * Returns true if the model allows the empty field ("not set") choice.
     * @see setAllowEmptyFieldName()
     * @note added in QGIS 3.0
     */
    bool allowEmptyFieldName() const;

    /**
     * Returns true if a string represents a field reference, or false if it is an
     * expression consisting of more than direct field reference.
     */
    bool isField( const QString& expression ) const;

    /**
     * Sets a single expression to be added after the fields at the end of the model.
     * @see setAllowExpression()
     * @see allowExpression()
     * @see removeExpression()
     */
    void setExpression( const QString &expression );

    /**
     * Removes any custom expression from the model.
     * @see setExpression()
     * @see allowExpression()
     */
    void removeExpression();

    /**
     * Returns the layer associated with the model.
     * @see setLayer()
     */
    QgsVectorLayer* layer();

  public slots:
    /**
     * Set the layer from which fields are displayed.
     * @see layer()
     */
    void setLayer( QgsVectorLayer *layer );

  protected slots:

    /**
     * Called when the model must be updated.
     */
    virtual void updateModel();

    // QAbstractItemModel interface
  public:
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
    QModelIndex parent( const QModelIndex &child ) const;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    int columnCount( const QModelIndex &parent ) const;
    QVariant data( const QModelIndex &index, int role ) const;
};
