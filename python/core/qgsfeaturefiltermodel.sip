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

    QgsFeatureFilterModel( QObject *parent = 0 );
    ~QgsFeatureFilterModel();

    QgsVectorLayer *sourceLayer() const;
%Docstring
 :rtype: QgsVectorLayer
%End
    void setSourceLayer( QgsVectorLayer *sourceLayer );

    QString displayExpression() const;
%Docstring
 :rtype: str
%End
    void setDisplayExpression( const QString &displayExpression );

    QString filterValue() const;
%Docstring
 :rtype: str
%End
    void setFilterValue( const QString &filterValue );

    virtual QModelIndex index( int row, int column, const QModelIndex &parent ) const;
    virtual QModelIndex parent( const QModelIndex &child ) const;
    virtual int rowCount( const QModelIndex &parent ) const;
    virtual int columnCount( const QModelIndex &parent ) const;
    virtual QVariant data( const QModelIndex &index, int role ) const;

    QString filterExpression() const;
%Docstring
 An additional filter expression to apply, next to the filterValue.
 Can be used for spatial filtering etc.
 :rtype: str
%End

    void setFilterExpression( const QString &filterExpression );
%Docstring
 An additional filter expression to apply, next to the filterValue.
 Can be used for spatial filtering etc.
%End

    bool isLoading() const;
%Docstring
 :rtype: bool
%End

    QString identifierField() const;
%Docstring
 :rtype: str
%End
    void setIdentifierField( const QString &identifierField );

    QVariant extraIdentifierValue() const;
%Docstring
 :rtype: QVariant
%End
    void setExtraIdentifierValue( const QVariant &extraIdentifierValue );

    int extraIdentifierValueIndex() const;
%Docstring
 :rtype: int
%End

    bool extraValueDoesNotExist() const;
%Docstring
 :rtype: bool
%End

  signals:
    void sourceLayerChanged();
    void displayExpressionChanged();
    void filterValueChanged();
    void filterExpressionChanged();
    void isLoadingChanged();
    void identifierFieldChanged();
    void filterJobCompleted();
    void extraIdentifierValueChanged();
    void extraIdentifierValueIndexChanged( int index );
    void extraValueDoesNotExistChanged();
    void beginUpdate();
    void endUpdate();

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfeaturefiltermodel.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
