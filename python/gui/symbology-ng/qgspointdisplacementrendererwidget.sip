/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology-ng/qgspointdisplacementrendererwidget.h            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsPointDisplacementRendererWidget: QgsRendererWidget
{

%TypeHeaderCode
#include "qgspointdisplacementrendererwidget.h"
%End
  public:
    static QgsRendererWidget *create( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer ) /Factory/;
%Docstring
 :rtype: QgsRendererWidget
%End
    QgsPointDisplacementRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer );
    ~QgsPointDisplacementRendererWidget();

    virtual QgsFeatureRenderer *renderer();

    virtual void setContext( const QgsSymbolWidgetContext &context );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology-ng/qgspointdisplacementrendererwidget.h            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
