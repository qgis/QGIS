/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/attributetable/qgsattributetablemodel.h                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsAttributeTableModel: QAbstractTableModel
{
%Docstring
A model backed by a QgsVectorLayerCache which is able to provide
feature/attribute information to a QAbstractItemView.

\brief
Is able to generate editor widgets for its QModelIndexes as well.
Is mostly referred to as "master model" within this doc and the source.

\see <a href="http://doc.qt.digia.com/qt/model-view-programming.html">Qt Model View Programming</a>
%End

%TypeHeaderCode
#include "qgsattributetablemodel.h"
%End
  public:
    enum Role
    {
      SortRole,
      FeatureIdRole,
      FieldIndexRole,
      UserRole
    };

  public:

    QgsAttributeTableModel( QgsVectorLayerCache *layerCache, QObject *parent = 0 );
%Docstring
Constructor
:param layerCache:  A layer cache to use as backend
:param parent:      The parent QObject (owner)
%End

    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;

%Docstring
Returns the number of rows
:param parent: parent index

:rtype: int
%End

    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;

%Docstring
Returns the number of columns
:param parent: parent index

:rtype: int
%End

    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

%Docstring
Returns header data
:param section: required section
:param orientation: horizontal or vertical orientation
:param role: data role

:rtype: QVariant
%End

    virtual QVariant data( const QModelIndex &index, int role ) const;

%Docstring
Returns data on the given index
:param index: model index
:param role: data role

:rtype: QVariant
%End

    virtual bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );

%Docstring
Updates data on given index
:param index: model index
:param value: new data value
:param role: data role

:rtype: bool
%End

    virtual Qt::ItemFlags flags( const QModelIndex &index ) const;

%Docstring
Returns item flags for the index
:param index: model index

:rtype: Qt.ItemFlags
%End

    void reload( const QModelIndex &index1, const QModelIndex &index2 );
%Docstring
Reloads the model data between indices
:param index1: start index
:param index2: end index
%End

    virtual bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() );

%Docstring
Remove rows

:rtype: bool
%End

    void resetModel();
%Docstring
Resets the model

Alias to loadLayer()
%End

    int idToRow( QgsFeatureId id ) const;
%Docstring
Maps feature id to table row
:param id: feature id

:rtype: int
%End

    QModelIndex idToIndex( QgsFeatureId id ) const;
%Docstring

:rtype: QModelIndex
%End

    QModelIndexList idToIndexList( QgsFeatureId id ) const;
%Docstring

:rtype: QModelIndexList
%End

    int fieldIdx( int col ) const;
%Docstring
get field index from column

:rtype: int
%End

    int fieldCol( int idx ) const;
%Docstring
get column from field index

:rtype: int
%End

    QgsFeatureId rowToId( int row ) const;
%Docstring
Maps row to feature id
:param row: row number

:rtype: QgsFeatureId
%End

    void swapRows( QgsFeatureId a, QgsFeatureId b );
%Docstring
Swaps two rows
:param a: first row
:param b: second row
%End

    QgsVectorLayer *layer() const;
%Docstring
Returns the layer this model uses as backend. Retrieved from the layer cache.

:rtype: QgsVectorLayer
%End

    QgsVectorLayerCache *layerCache() const;
%Docstring
Returns the layer cache this model uses as backend.

:rtype: QgsVectorLayerCache
%End

    void executeAction( const QUuid &action, const QModelIndex &idx ) const;
%Docstring
Execute an action
%End

    void executeMapLayerAction( QgsMapLayerAction *action, const QModelIndex &idx ) const;
%Docstring
Execute a QgsMapLayerAction
%End

    QgsFeature feature( const QModelIndex &idx ) const;
%Docstring
Return the feature attributes at given model index

:return: feature attributes at given model index
:rtype: QgsFeature

%End

    void prefetchColumnData( int column );
%Docstring
Caches the entire data for one column. This should be called prior to sorting,
so the data does not have to be fetched for every single comparison.
Specify -1 as column to invalidate the cache

:param column: The column index of the field to catch
%End

    void prefetchSortData( const QString &expression );
%Docstring
Prefetches the entire data for one expression. Based on this cached information
the sorting can later be done in a performant way.

:param expression: The expression to cache
%End

    QString sortCacheExpression() const;
%Docstring
The expression which was used to fill the sorting cache

:rtype: str
%End

    void setRequest( const QgsFeatureRequest &request );
%Docstring
Set a request that will be used to fill this attribute table model.
In contrast to a filter, the request will constrain the data shown without the possibility
to dynamically adjust it.

:param request: The request to use to fill this table model.
%End

    const QgsFeatureRequest &request() const;
%Docstring

:rtype: QgsFeatureRequest
%End

    void setEditorContext( const QgsAttributeEditorContext &context );
%Docstring
Sets the context in which this table is shown.
Will be forwarded to any editor widget created when editing data on this model.

:param context: The context
%End

    const QgsAttributeEditorContext &editorContext() const;
%Docstring
Returns the context in which this table is shown.
Will be forwarded to any editor widget created when editing data on this model.

:return: The context
:rtype: QgsAttributeEditorContext

%End

    int extraColumns() const;
%Docstring
Empty extra columns to announce from this model.
Any extra columns need to be implemented by proxy models in front of this model.

:rtype: int
%End

    void setExtraColumns( int extraColumns );
%Docstring
Empty extra columns to announce from this model.
Any extra columns need to be implemented by proxy models in front of this model.
%End

  public slots:

    virtual void loadLayer();
%Docstring
Loads the layer into the model
Preferably to be called, before using this model as source for any other proxy model
%End

    void fieldConditionalStyleChanged( const QString &fieldName );
%Docstring
Handles updating the model when the conditional style for a field changes.
:param fieldName: name of field whose conditional style has changed

.. versionadded:: 2.12
%End

  signals:

    void modelChanged();
%Docstring
Model has been changed
%End

    void finished();

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/attributetable/qgsattributetablemodel.h                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
