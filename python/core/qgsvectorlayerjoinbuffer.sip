/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorlayerjoinbuffer.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






typedef QList< QgsVectorLayerJoinInfo > QgsVectorJoinList;


class QgsVectorLayerJoinBuffer : QObject
{
%Docstring
 Manages joined fields for a vector layer*
%End

%TypeHeaderCode
#include "qgsvectorlayerjoinbuffer.h"
%End
  public:
    QgsVectorLayerJoinBuffer( QgsVectorLayer *layer = 0 );

    bool addJoin( const QgsVectorLayerJoinInfo &joinInfo );
%Docstring
 Joins another vector layer to this layer
\param joinInfo join object containing join layer id, target and source field
:return: (since 2.6) whether the join was successfully added *
 :rtype: bool
%End

    bool removeJoin( const QString &joinLayerId );
%Docstring
 Removes a vector layer join
:return: true if join was found and successfully removed *
 :rtype: bool
%End

    void updateFields( QgsFields &fields );
%Docstring
 Updates field map with joined attributes
\param fields map to append joined attributes
%End

    void createJoinCaches();
%Docstring
Calls cacheJoinLayer() for all vector joins
%End

    void writeXml( QDomNode &layer_node, QDomDocument &document ) const;
%Docstring
Saves mVectorJoins to xml under the layer node
%End

    void readXml( const QDomNode &layer_node );
%Docstring
Does not resolve layer IDs to layers - call resolveReferences() afterwards
%End

    void resolveReferences( QgsProject *project );
%Docstring
.. versionadded:: 3.0
%End

    bool containsJoins() const;
%Docstring
Quick way to test if there is any join at all
 :rtype: bool
%End

    const QgsVectorJoinList &vectorJoins() const;
%Docstring
 :rtype: QgsVectorJoinList
%End

    const QgsVectorLayerJoinInfo *joinForFieldIndex( int index, const QgsFields &fields, int &sourceFieldIndex /Out/ ) const;
%Docstring
 Finds the vector join for a layer field index.
\param index this layers attribute index
\param fields fields of the vector layer (including joined fields)
\param sourceFieldIndex Output: field's index in source layer *
 :rtype: QgsVectorLayerJoinInfo
%End

    int joinedFieldsOffset( const QgsVectorLayerJoinInfo *info, const QgsFields &fields );
%Docstring
.. versionadded:: 2.6
 :rtype: int
%End

    static QVector<int> joinSubsetIndices( QgsVectorLayer *joinLayer, const QStringList &joinFieldsSubset );
%Docstring
.. versionadded:: 2.6
 :rtype: list of int
%End

    QList<const QgsVectorLayerJoinInfo *> joinsWhereFieldIsId( const QgsField &field ) const;
%Docstring
 Returns joins where the field of a target layer is considered as an id.
 \param field the field of a target layer
 :return: a list of vector joins
.. versionadded:: 3.0
 :rtype: list of const QgsVectorLayerJoinInfo
%End

    QgsFeature joinedFeatureOf( const QgsVectorLayerJoinInfo &info, const QgsFeature &feature ) const;
%Docstring
 Returns the joined feature corresponding to the feature.
 \param info the vector join information
 \param feature the feature of the target layer
.. versionadded:: 3.0
 :rtype: QgsFeature
%End

    QgsVectorLayerJoinBuffer *clone() const /Factory/;
%Docstring
.. versionadded:: 2.6
 :rtype: QgsVectorLayerJoinBuffer
%End

  signals:
    void joinedFieldsChanged();
%Docstring
.. versionadded:: 2.6
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorlayerjoinbuffer.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
