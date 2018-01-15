/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfieldmodel.h                                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsFieldModel : QAbstractItemModel
{
%Docstring
 The QgsFieldModel class is a model to display the list of fields of a layer in widgets.
If allowed, expressions might be added to the end of the model.
It can be associated with a QgsMapLayerModel to dynamically display a layer and its fields.

.. versionadded:: 2.3
%End

%TypeHeaderCode
#include "qgsfieldmodel.h"
%End
  public:

    enum FieldRoles
    {
      FieldNameRole,
      FieldIndexRole,
      ExpressionRole,
      IsExpressionRole,
      ExpressionValidityRole,
      FieldTypeRole,
      FieldOriginRole,
      IsEmptyRole,
    };

    explicit QgsFieldModel( QObject *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsFieldModel - creates a model to display the fields of a given layer.
%End

    QModelIndex indexFromName( const QString &fieldName );
%Docstring
Returns the index corresponding to a given fieldName.
%End

    void setAllowExpression( bool allowExpression );
%Docstring
Sets whether custom expressions are accepted and displayed in the model.

.. seealso:: :py:func:`allowExpression`

.. seealso:: :py:func:`setExpression`
%End

    bool allowExpression();
%Docstring
Returns true if the model allows custom expressions to be created and displayed.

.. seealso:: :py:func:`setAllowExpression`
%End

    void setAllowEmptyFieldName( bool allowEmpty );
%Docstring
Sets whether an optional empty field ("not set") option is present in the model.

.. seealso:: :py:func:`allowEmptyFieldName`

.. versionadded:: 3.0
%End

    bool allowEmptyFieldName() const;
%Docstring
Returns true if the model allows the empty field ("not set") choice.

.. seealso:: :py:func:`setAllowEmptyFieldName`

.. versionadded:: 3.0
%End

    bool isField( const QString &expression ) const;
%Docstring
Returns true if a string represents a field reference, or false if it is an
expression consisting of more than direct field reference.
%End

    void setExpression( const QString &expression );
%Docstring
Sets a single expression to be added after the fields at the end of the model.

.. seealso:: :py:func:`setAllowExpression`

.. seealso:: :py:func:`allowExpression`

.. seealso:: :py:func:`removeExpression`
%End

    void removeExpression();
%Docstring
Removes any custom expression from the model.

.. seealso:: :py:func:`setExpression`

.. seealso:: :py:func:`allowExpression`
%End

    QgsVectorLayer *layer();
%Docstring
Returns the layer associated with the model.

.. seealso:: :py:func:`setLayer`
%End

    virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;

    virtual QModelIndex parent( const QModelIndex &child ) const;

    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;

    virtual int columnCount( const QModelIndex &parent ) const;

    virtual QVariant data( const QModelIndex &index, int role ) const;


  public slots:

    void setLayer( QgsVectorLayer *layer );
%Docstring
Set the layer from which fields are displayed.

.. seealso:: :py:func:`layer`
%End

  protected slots:

    virtual void updateModel();
%Docstring
Called when the model must be updated.
%End

  protected:


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfieldmodel.h                                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
