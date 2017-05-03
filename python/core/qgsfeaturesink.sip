/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfeaturesink.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsFeatureSink
{
%Docstring
 An interface for objects which accept features via addFeature(s) methods.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsfeaturesink.h"
%End
  public:

    virtual ~QgsFeatureSink();

    virtual bool addFeature( QgsFeature &feature );
%Docstring
 Adds a single ``feature`` to the sink.
.. seealso:: addFeatures()
 :return: true in case of success and false in case of failure
 :rtype: bool
%End

    virtual bool addFeatures( QgsFeatureList &features ) = 0;
%Docstring
 Adds a list of ``features`` to the sink.
.. seealso:: addFeature()
 :return: true in case of success and false in case of failure
 :rtype: bool
%End

    virtual bool addFeatures( QgsFeatureIterator &iterator );
%Docstring
 Adds all features from the specified ``iterator`` to the sink.
 :return: true if all features were added successfully, or false if any feature could not be added
 :rtype: bool
%End

};


class QgsProxyFeatureSink : QgsFeatureSink
{
%Docstring
 A simple feature sink which proxies feature addition on to another feature sink.

 This class is designed to allow factory methods which always return new QgsFeatureSink
 objects. Since it is not always possible to create an entirely new QgsFeatureSink
 (e.g. if the feature sink is a layer's data provider), a new QgsProxyFeatureSink
 can instead be returned which forwards features on to the destination sink. The
 proxy sink can be safely deleted without affecting the destination sink.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsfeaturesink.h"
%End
  public:

    QgsProxyFeatureSink( QgsFeatureSink *sink );
%Docstring
 Constructs a new QgsProxyFeatureSink which forwards features onto a destination ``sink``.
%End
    virtual bool addFeature( QgsFeature &feature );
    virtual bool addFeatures( QgsFeatureList &features );
    virtual bool addFeatures( QgsFeatureIterator &iterator );

    QgsFeatureSink *destinationSink();
%Docstring
 Returns the destination QgsFeatureSink which the proxy will forward features to.
 :rtype: QgsFeatureSink
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfeaturesink.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
