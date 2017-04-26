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
 \see addFeatures()
 :return: true in case of success and false in case of failure
 :rtype: bool
%End

    virtual bool addFeatures( QgsFeatureList &features ) = 0;
%Docstring
 Adds a list of ``features`` to the sink.
 \see addFeature()
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

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfeaturesink.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
