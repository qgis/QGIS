/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgsinvertedpolygonrendererwidget.h                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsInvertedPolygonRendererWidget : QgsRendererWidget
{
%Docstring
A widget used represent options of a QgsInvertedPolygonRenderer

.. versionadded:: 2.4
%End

%TypeHeaderCode
#include "qgsinvertedpolygonrendererwidget.h"
%End
  public:

    static QgsRendererWidget *create( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer ) /Factory/;
%Docstring
Static creation method

:param layer: the layer where this renderer is applied
:param style:
:param renderer: the mask renderer (will not take ownership)
%End

    QgsInvertedPolygonRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer );
%Docstring
Constructor

:param layer: the layer where this renderer is applied
:param style:
:param renderer: the mask renderer (will not take ownership)
%End

    virtual QgsFeatureRenderer *renderer();


    virtual void setContext( const QgsSymbolWidgetContext &context );


  protected:

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgsinvertedpolygonrendererwidget.h                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
