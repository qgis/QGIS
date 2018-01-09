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

    enum Flag
    {

      FastInsert,
    };
    typedef QFlags<QgsFeatureSink::Flag> Flags;


    virtual ~QgsFeatureSink();

    virtual bool addFeature( QgsFeature &feature, QgsFeatureSink::Flags flags = 0 );
%Docstring
Adds a single ``feature`` to the sink. Feature addition behavior is controlled by the specified ``flags``.

.. seealso:: :py:func:`addFeatures`

:return: true in case of success and false in case of failure
%End

    virtual bool addFeatures( QgsFeatureList &features, QgsFeatureSink::Flags flags = 0 ) = 0;
%Docstring
Adds a list of ``features`` to the sink. Feature addition behavior is controlled by the specified ``flags``.

.. seealso:: :py:func:`addFeature`

:return: true in case of success and false in case of failure
%End

    virtual bool addFeatures( QgsFeatureIterator &iterator, QgsFeatureSink::Flags flags = 0 );
%Docstring
Adds all features from the specified ``iterator`` to the sink. Feature addition behavior is controlled by the specified ``flags``.

:return: true if all features were added successfully, or false if any feature could not be added
%End

    virtual bool flushBuffer();
%Docstring
Flushes any internal buffer which may exist in the sink, causing any buffered features to be added to the sink's destination.

:return: false if any buffered features could not be added to the sink.
%End
};

QFlags<QgsFeatureSink::Flag> operator|(QgsFeatureSink::Flag f1, QFlags<QgsFeatureSink::Flag> f2);



class QgsProxyFeatureSink : QgsFeatureSink
{
%Docstring
A simple feature sink which proxies feature addition on to another feature sink.

This class is designed to allow factory methods which always return new QgsFeatureSink
objects. Since it is not always possible to create an entirely new QgsFeatureSink
(e.g. if the feature sink is a layer's data provider), a new :py:class:`QgsProxyFeatureSink`
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
    virtual bool addFeature( QgsFeature &feature, QgsFeatureSink::Flags flags = 0 );
    virtual bool addFeatures( QgsFeatureList &features, QgsFeatureSink::Flags flags = 0 );
    virtual bool addFeatures( QgsFeatureIterator &iterator, QgsFeatureSink::Flags flags = 0 );

    QgsFeatureSink *destinationSink();
%Docstring
Returns the destination QgsFeatureSink which the proxy will forward features to.
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfeaturesink.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
