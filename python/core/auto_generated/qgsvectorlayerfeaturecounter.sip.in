/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorlayerfeaturecounter.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/


class QgsVectorLayerFeatureCounter : QgsTask
{
%Docstring

Counts the features in a QgsVectorLayer in task.
You should most likely not use this directly and instead call
QgsVectorLayer.countSymbolFeatures() and connect to the signal
:py:func:`QgsVectorLayer.symbolFeatureCountMapChanged()`

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsvectorlayerfeaturecounter.h"
%End
  public:

    QgsVectorLayerFeatureCounter( QgsVectorLayer *layer, const QgsExpressionContext &context = QgsExpressionContext() );
%Docstring
Create a new feature counter for ``layer``.
%End

    virtual bool run();



    long featureCount( const QString &legendKey ) const;
%Docstring
Gets the feature count for a particular ``legendKey``.
If the key has not been found, -1 will be returned.
%End

  signals:

    void symbolsCounted();
%Docstring
Emitted when the symbols have been counted.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorlayerfeaturecounter.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
