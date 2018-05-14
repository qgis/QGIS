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

Note that this class should generally be used with a QgsLocatorProxyModel
in order to ensure correct sorting of results by priority and match level.

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

    QgsLocatorModel( QObject *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsLocatorModel.
%End

    void clear();
%Docstring
Resets the model and clears all existing results.

.. seealso:: :py:func:`deferredClear`
%End

    void deferredClear();
%Docstring
Resets the model and clears all existing results after a short delay, or whenever the next result is added to the model
(whichever occurs first). Using deferredClear() instead of clear() can avoid the visually distracting frequent clears
which may occur if the model is being updated quickly multiple times as a result of users typing in a search query.

.. seealso:: :py:func:`deferredClear`
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

class QgsLocatorAutomaticModel : QgsLocatorModel
{
%Docstring
A QgsLocatorModel which has is associated directly with a
:py:class:`QgsLocator`, and is automatically populated with results
from locator searches.

Use this QgsLocatorModel subclass when you want the connections
between a QgsLocator and the model to be automatically created
for you. If more flexibility in model behavior is required,
use the base QgsLocatorModel class instead and setup the
connections manually.

Note that this class should generally be used with a :py:class:`QgsLocatorProxyModel`
in order to ensure correct sorting of results by priority and match level.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslocatormodel.h"
%End
  public:

    explicit QgsLocatorAutomaticModel( QgsLocator *locator /TransferThis/ );
%Docstring
Constructor for QgsLocatorAutomaticModel, linked with the specified ``locator``.

The ``locator`` is used as the model's parent.
%End

    QgsLocator *locator();
%Docstring
Returns a pointer to the locator utilized by this model.
%End

    void search( const QString &string );
%Docstring
Enqueues a search for a specified ``string`` within the model.

Note that the search may not begin immediately if an existing search request
is still running. In this case the existing search must be completely
terminated before the new search can begin. The model handles this
situation automatically, and will trigger a search for the new
search string as soon as possible.
%End

    virtual QgsLocatorContext createContext();
%Docstring
Returns a new locator context for searches. The default implementation
returns a default constructed :py:class:`QgsLocatorContext`. Subclasses can override
this method to implement custom context creation logic.
%End

};

class QgsLocatorProxyModel : QSortFilterProxyModel
{
%Docstring
A sort proxy model for :py:class:`QgsLocatorModel`, which automatically sorts
results by precedence.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslocatormodel.h"
%End
  public:

    explicit QgsLocatorProxyModel( QObject *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsLocatorProxyModel, with the specified ``parent`` object.
%End
    virtual bool lessThan( const QModelIndex &left, const QModelIndex &right ) const;

};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/locator/qgslocatormodel.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
