/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/locator/qgslocatormodel.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsLocatorModel : QAbstractTableModel
{
%Docstring
 An abstract list model for displaying the results of locator searches.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslocatormodel.h"
%End
  public:

    enum Role
    {
      ResultDataRole,
      ResultTypeRole,
      ResultFilterPriorityRole,
      ResultScoreRole,
      ResultFilterNameRole,
    };

    QgsLocatorModel( QObject *parent = 0 );
%Docstring
 Constructor for QgsLocatorModel.
%End

    void clear();
%Docstring
 Resets the model and clears all existing results.
.. seealso:: deferredClear()
%End

    void deferredClear();
%Docstring
 Resets the model and clears all existing results after a short delay, or whenever the next result is added to the model
 (whichever occurs first). Using deferredClear() instead of clear() can avoid the visually distracting frequent clears
 which may occur if the model is being updated quickly multiple times as a result of users typing in a search query.
.. seealso:: deferredClear()
%End

    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;

    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;

    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;

    virtual Qt::ItemFlags flags( const QModelIndex &index ) const;


  public slots:

    void addResult( const QgsLocatorResult &result );
%Docstring
 Adds a new ``result`` to the model.
%End

};

class QgsLocatorProxyModel : QSortFilterProxyModel
{
%Docstring
 A sort proxy model for QgsLocatorModel, which automatically sorts
 results by precedence.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslocatormodel.h"
%End
  public:

    explicit QgsLocatorProxyModel( QObject *parent = 0 );
    virtual bool lessThan( const QModelIndex &left, const QModelIndex &right ) const;

};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/locator/qgslocatormodel.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
