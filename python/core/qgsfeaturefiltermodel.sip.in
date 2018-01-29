/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfeaturefiltermodel.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsFeatureFilterModel : QAbstractItemModel
{
%Docstring
Provides a list of features based on filter conditions.
Features are fetched asynchronously.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsfeaturefiltermodel.h"
%End
  public:

    enum Role
    {
      IdentifierValueRole,
      ValueRole
    };

    explicit QgsFeatureFilterModel( QObject *parent = 0 );
%Docstring
Create a new QgsFeatureFilterModel, optionally specifying a ``parent``.
%End
    ~QgsFeatureFilterModel();

    QgsVectorLayer *sourceLayer() const;
%Docstring
The source layer from which features will be fetched.
%End

    void setSourceLayer( QgsVectorLayer *sourceLayer );
%Docstring
The source layer from which features will be fetched.
%End

    QString displayExpression() const;
%Docstring
The display expression will be used for

- displaying values in the combobox
- filtering based on filterValue
%End

    void setDisplayExpression( const QString &displayExpression );
%Docstring
The display expression will be used for

- displaying values in the combobox
- filtering based on filterValue
%End

    QString filterValue() const;
%Docstring
This value will be used to filter the features available from
this model. Whenever a substring of the displayExpression of a feature
matches the filter value, it will be accessible by this model.
%End

    void setFilterValue( const QString &filterValue );
%Docstring
This value will be used to filter the features available from
this model. Whenever a substring of the displayExpression of a feature
matches the filter value, it will be accessible by this model.
%End

    virtual QModelIndex index( int row, int column, const QModelIndex &parent ) const;

    virtual QModelIndex parent( const QModelIndex &child ) const;

    virtual int rowCount( const QModelIndex &parent ) const;

    virtual int columnCount( const QModelIndex &parent ) const;

    virtual QVariant data( const QModelIndex &index, int role ) const;


    QString filterExpression() const;
%Docstring
An additional filter expression to apply, next to the filterValue.
Can be used for spatial filtering etc.
%End

    void setFilterExpression( const QString &filterExpression );
%Docstring
An additional filter expression to apply, next to the filterValue.
Can be used for spatial filtering etc.
%End

    bool isLoading() const;
%Docstring
Indicator if the model is currently performing any feature iteration in the background.
%End

    QString identifierField() const;
%Docstring
The identifier field should be a unique field that can be used to identify individual features.
It is normally set to the primary key of the layer.
%End

    void setIdentifierField( const QString &identifierField );
%Docstring
The identifier field should be a unique field that can be used to identify individual features.
It is normally set to the primary key of the layer.
%End

    QVariant extraIdentifierValue() const;
%Docstring
Allows specifying one value that does not need to match the filter criteria but will
still be available in the model.
%End

    void setExtraIdentifierValue( const QVariant &extraIdentifierValue );
%Docstring
Allows specifying one value that does not need to match the filter criteria but will
still be available in the model.
%End

    int extraIdentifierValueIndex() const;
%Docstring
The index at which the extra identifier value is available within the model.
%End

    bool extraValueDoesNotExist() const;
%Docstring
Flag indicating that the extraIdentifierValue does not exist in the data.
%End

    bool allowNull() const;
%Docstring
Add a NULL entry to the list.
%End

    void setAllowNull( bool allowNull );
%Docstring
Add a NULL entry to the list.
%End

  signals:

    void sourceLayerChanged();
%Docstring
The source layer from which features will be fetched.
%End

    void displayExpressionChanged();
%Docstring
The display expression will be used for

- displaying values in the combobox
- filtering based on filterValue
%End

    void filterValueChanged();
%Docstring
This value will be used to filter the features available from
this model. Whenever a substring of the displayExpression of a feature
matches the filter value, it will be accessible by this model.
%End

    void filterExpressionChanged();
%Docstring
An additional filter expression to apply, next to the filterValue.
Can be used for spatial filtering etc.
%End

    void isLoadingChanged();
%Docstring
Indicator if the model is currently performing any feature iteration in the background.
%End

    void identifierFieldChanged();
%Docstring
The identifier field should be a unique field that can be used to identify individual features.
It is normally set to the primary key of the layer.
%End

    void filterJobCompleted();
%Docstring
Indicates that a filter job has been completed and new data may be available.
%End

    void extraIdentifierValueChanged();
%Docstring
Allows specifying one value that does not need to match the filter criteria but will
still be available in the model.
%End

    void extraIdentifierValueIndexChanged( int index );
%Docstring
The index at which the extra identifier value is available within the model.
%End

    void extraValueDoesNotExistChanged();
%Docstring
Flag indicating that the extraIdentifierValue does not exist in the data.
%End

    void beginUpdate();
%Docstring
Notification that the model is about to be changed because a job was completed.
%End

    void endUpdate();
%Docstring
Notification that the model change is finished. Will always be emitted in sync with beginUpdate.
%End

    void allowNullChanged();
%Docstring
Add a NULL entry to the list.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfeaturefiltermodel.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
