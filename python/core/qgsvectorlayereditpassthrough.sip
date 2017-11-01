/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorlayereditpassthrough.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsVectorLayerEditPassthrough : QgsVectorLayerEditBuffer
{

%TypeHeaderCode
#include "qgsvectorlayereditpassthrough.h"
%End
  public:
    QgsVectorLayerEditPassthrough( QgsVectorLayer *layer );
    virtual bool isModified() const;

    virtual bool addFeature( QgsFeature &f );

    virtual bool addFeatures( QgsFeatureList &features );

    virtual bool deleteFeature( QgsFeatureId fid );

    virtual bool deleteFeatures( const QgsFeatureIds &fids );

    virtual bool changeGeometry( QgsFeatureId fid, const QgsGeometry &geom );

    virtual bool changeAttributeValue( QgsFeatureId fid, int field, const QVariant &newValue, const QVariant &oldValue = QVariant() );

    virtual bool addAttribute( const QgsField &field );

    virtual bool deleteAttribute( int attr );

    virtual bool renameAttribute( int attr, const QString &newName );

    virtual bool commitChanges( QStringList &commitErrors );

    virtual void rollBack();


    bool update( QgsTransaction *transaction, const QString &sql );
%Docstring
 Update underlying data with a SQL query embedded in a transaction.

 \param transaction Transaction in which the sql query has been run
 \param sql The SQL query updating data

 :return: true if the undo/redo command is well added to the stack, false otherwise

.. versionadded:: 3.0
 :rtype: bool
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorlayereditpassthrough.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
