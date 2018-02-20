/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgstrackedvectorlayertools.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/


class QgsTrackedVectorLayerTools : QgsVectorLayerTools
{

%TypeHeaderCode
#include "qgstrackedvectorlayertools.h"
%End
  public:

    QgsTrackedVectorLayerTools();
%Docstring
Constructor for QgsTrackedVectorLayerTools.
%End

    virtual bool addFeature( QgsVectorLayer *layer, const QgsAttributeMap &defaultValues, const QgsGeometry &defaultGeometry, QgsFeature *feature ) const;

    virtual bool startEditing( QgsVectorLayer *layer ) const;

    virtual bool stopEditing( QgsVectorLayer *layer, bool allowCancel ) const;

    virtual bool saveEdits( QgsVectorLayer *layer ) const;

    virtual bool copyMoveFeatures( QgsVectorLayer *layer, QgsFeatureRequest &request, double  dx = 0, double dy = 0, QString *errorMsg = 0 ) const;


    void setVectorLayerTools( const QgsVectorLayerTools *tools );
%Docstring
Set the vector layer tools that will be used to interact with the data
%End

    void rollback();
%Docstring
Delete all features which have been added via this object.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgstrackedvectorlayertools.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
