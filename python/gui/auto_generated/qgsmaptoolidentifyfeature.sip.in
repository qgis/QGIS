/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmaptoolidentifyfeature.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsMapToolIdentifyFeature : QgsMapToolIdentify
{
%Docstring
The QgsMapToolIdentifyFeature class is a map tool to identify a feature on a chosen layer.
Once the map tool is enable, user can click on the map canvas to identify a feature.
A signal will then be emitted.
%End

%TypeHeaderCode
#include "qgsmaptoolidentifyfeature.h"
%End
  public:

    QgsMapToolIdentifyFeature( QgsMapCanvas *canvas, QgsVectorLayer *vl = 0 );
%Docstring
QgsMapToolIdentifyFeature is a map tool to identify a feature on a chosen layer

:param canvas: the map canvas
:param vl: the vector layer. The map tool can be initialized without any layer and can be set afterward.
%End

    void setLayer( QgsVectorLayer *vl );
%Docstring
change the layer used by the map tool to identify
%End

    virtual void canvasReleaseEvent( QgsMapMouseEvent *e );


  signals:
    void featureIdentified( const QgsFeature & );
    void featureIdentified( QgsFeatureId );

  protected:
    virtual void keyPressEvent( QKeyEvent *e );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmaptoolidentifyfeature.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
