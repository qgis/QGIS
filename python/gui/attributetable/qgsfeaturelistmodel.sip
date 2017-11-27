/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/attributetable/qgsfeaturelistmodel.h                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsFeatureListModel : QAbstractProxyModel, QgsFeatureModel
{

%TypeHeaderCode
#include "qgsfeaturelistmodel.h"
%End
  public:
    struct FeatureInfo
    {
      public:

        FeatureInfo();
%Docstring
 Constructor for FeatureInfo.
%End

        bool isNew;
        bool isEdited;
    };

    enum Role
    {
      FeatureInfoRole,
      FeatureRole
    };

  public:
    explicit QgsFeatureListModel( QgsAttributeTableFilterModel *sourceModel, QObject *parent /TransferThis/ = 0 );

    virtual void setSourceModel( QgsAttributeTableFilterModel *sourceModel );
    QgsVectorLayerCache *layerCache();
%Docstring
 :rtype: QgsVectorLayerCache
%End
    virtual QVariant data( const QModelIndex &index, int role ) const;
    virtual Qt::ItemFlags flags( const QModelIndex &index ) const;

    void setInjectNull( bool injectNull );
%Docstring
 If true is specified, a NULL value will be injected
 \param injectNull state of null value injection
.. versionadded:: 2.9
%End

    bool injectNull();
%Docstring
 Returns the current state of null value injection
 :return: If a NULL value is added
.. versionadded:: 2.9
 :rtype: bool
%End

    QgsAttributeTableModel *masterModel();
%Docstring
 :rtype: QgsAttributeTableModel
%End

    bool setDisplayExpression( const QString &expression );
%Docstring
  \param  expression   A QgsExpression compatible string.
  :return: true if the expression could be set, false if there was a parse error.
          If it fails, the old expression will still be applied. Call parserErrorString()
          for a meaningful error message.
 :rtype: bool
%End

    QString parserErrorString();
%Docstring
 Returns a detailed message about errors while parsing a QgsExpression.
 :return: A message containing information about the parser error.
 :rtype: str
%End

    QString displayExpression() const;
%Docstring
 :rtype: str
%End
    bool featureByIndex( const QModelIndex &index, QgsFeature &feat );
%Docstring
 :rtype: bool
%End
    QgsFeatureId idxToFid( const QModelIndex &index ) const;
%Docstring
 :rtype: QgsFeatureId
%End
    QModelIndex fidToIdx( const QgsFeatureId fid ) const;
%Docstring
 :rtype: QModelIndex
%End

    virtual QModelIndex mapToSource( const QModelIndex &proxyIndex ) const;
    virtual QModelIndex mapFromSource( const QModelIndex &sourceIndex ) const;

    virtual QModelIndex mapToMaster( const QModelIndex &proxyIndex ) const;
%Docstring
 :rtype: QModelIndex
%End
    virtual QModelIndex mapFromMaster( const QModelIndex &sourceIndex ) const;
%Docstring
 :rtype: QModelIndex
%End

    virtual QItemSelection mapSelectionFromMaster( const QItemSelection &selection ) const;
%Docstring
 :rtype: QItemSelection
%End
    virtual QItemSelection mapSelectionToMaster( const QItemSelection &selection ) const;
%Docstring
 :rtype: QItemSelection
%End

    virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
    virtual QModelIndex parent( const QModelIndex &child ) const;
    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;

    virtual QModelIndex fidToIndex( QgsFeatureId fid );

    QModelIndexList fidToIndexList( QgsFeatureId fid );
%Docstring
 :rtype: QModelIndexList
%End

  public slots:
    void onBeginRemoveRows( const QModelIndex &parent, int first, int last );
    void onEndRemoveRows( const QModelIndex &parent, int first, int last );
    void onBeginInsertRows( const QModelIndex &parent, int first, int last );
    void onEndInsertRows( const QModelIndex &parent, int first, int last );

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/attributetable/qgsfeaturelistmodel.h                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
