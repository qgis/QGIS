/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/processing/qgsnativealgorithms.h                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsNativeAlgorithms: QgsProcessingProvider
{
%Docstring
.. versionadded:: 3.0
 Native c++ processing algorithm provider.
%End

%TypeHeaderCode
#include "qgsnativealgorithms.h"
%End
  public:

    QgsNativeAlgorithms( QObject *parent = 0 );
%Docstring
 Constructor for QgsNativeAlgorithms.
%End

    virtual QIcon icon() const;

    virtual QString svgIconPath() const;

    virtual QString id() const;

    virtual QString name() const;

    virtual bool supportsNonFileBasedOutput() const;


  protected:

    virtual void loadAlgorithms();


};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/processing/qgsnativealgorithms.h                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
