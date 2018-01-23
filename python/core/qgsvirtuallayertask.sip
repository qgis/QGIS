/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvirtuallayertask.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsVirtualLayerTask : QgsTask
{
%Docstring

Initializes a virtual layer in a separated task.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsvirtuallayertask.h"
%End
  public:

    QgsVirtualLayerTask( const QgsVirtualLayerDefinition &definition );

    QgsVectorLayer *layer();

    QgsVirtualLayerDefinition definition() const;

    virtual bool run();


    virtual void cancel();


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvirtuallayertask.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
