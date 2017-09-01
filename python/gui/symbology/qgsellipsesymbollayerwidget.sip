/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgsellipsesymbollayerwidget.h                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsEllipseSymbolLayerWidget: QgsSymbolLayerWidget
{

%TypeHeaderCode
#include "qgsellipsesymbollayerwidget.h"
%End
  public:
    QgsEllipseSymbolLayerWidget( QgsVectorLayer *vl, QWidget *parent /TransferThis/ = 0 );

    static QgsSymbolLayerWidget *create( QgsVectorLayer *vl ) /Factory/;
%Docstring
 :rtype: QgsSymbolLayerWidget
%End

    virtual void setSymbolLayer( QgsSymbolLayer *layer );
    virtual QgsSymbolLayer *symbolLayer();

  protected:

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgsellipsesymbollayerwidget.h                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
