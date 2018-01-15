/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfieldproxymodel.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsFieldProxyModel : QSortFilterProxyModel
{
%Docstring
 The QgsFieldProxyModel class provides an easy to use model to display the list of fields of a layer.

.. versionadded:: 2.3
%End

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
      Time,
      HideReadOnly,
      AllTypes,
    };
    typedef QFlags<QgsFieldProxyModel::Filter> Filters;


    explicit QgsFieldProxyModel( QObject *parent /TransferThis/ = 0 );
%Docstring
 QgsFieldProxModel creates a proxy model with a QgsFieldModel as source model.
It can be used to filter the fields based on their types.
%End

    QgsFieldModel *sourceFieldModel();
%Docstring
Returns the QgsFieldModel used in this QSortFilterProxyModel
%End

    QgsFieldProxyModel *setFilters( QgsFieldProxyModel::Filters filters );
%Docstring
Set flags that affect how fields are filtered in the model.

:param filters: are Filter flags

.. seealso:: :py:func:`filters`
%End

    const Filters &filters() const;
%Docstring
Returns the filters controlling displayed fields.

.. seealso:: :py:func:`setFilters`
%End

  public:
    virtual bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const;

    virtual bool lessThan( const QModelIndex &left, const QModelIndex &right ) const;

};

QFlags<QgsFieldProxyModel::Filter> operator|(QgsFieldProxyModel::Filter f1, QFlags<QgsFieldProxyModel::Filter> f2);


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfieldproxymodel.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
