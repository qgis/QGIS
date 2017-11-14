/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsdatumtransformstore.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsDatumTransformStore
{
%Docstring
 The QgsDatumTransformStore class keeps track of datum transformations
 as chosen by the user.

.. versionadded:: 2.4
%End

%TypeHeaderCode
#include "qgsdatumtransformstore.h"
%End
  public:
    explicit QgsDatumTransformStore( const QgsCoordinateReferenceSystem &destCrs );

    void clear();

    void setDestinationCrs( const QgsCoordinateReferenceSystem &destCrs );

    void addEntry( const QString &layerId, const QString &srcAuthId, const QString &destAuthId, int srcDatumTransform, int destDatumTransform );

    bool hasEntryForLayer( QgsMapLayer *layer ) const;
%Docstring
 :rtype: bool
%End

    QgsCoordinateTransform transformation( const QgsMapLayer *layer, QString srcAuthId = QString(), QString dstAuthId = QString() ) const;
%Docstring
 Will return transform from layer's CRS to current destination CRS.
 :return: transformation associated with layer, or an invalid QgsCoordinateTransform
 if no transform is associated with the layer
 \param layer the associated layer
 \param srcAuthId source CRS (defaults to layer crs)
 \param dstAuthId destination CRS (defaults to store's crs)
 :rtype: QgsCoordinateTransform
%End

    void readXml( const QDomNode &parentNode );

    void writeXml( QDomNode &parentNode, QDomDocument &doc ) const;

    struct Entry
    {
      QString srcAuthId;
      QString destAuthId;
      int srcDatumTransform; //-1 if unknown or not specified
      int destDatumTransform;
    };

  protected:

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsdatumtransformstore.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
