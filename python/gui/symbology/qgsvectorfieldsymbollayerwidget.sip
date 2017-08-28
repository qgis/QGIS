/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgsvectorfieldsymbollayerwidget.h                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsVectorFieldSymbolLayerWidget: QgsSymbolLayerWidget
{

%TypeHeaderCode
#include "qgsvectorfieldsymbollayerwidget.h"
%End
  public:
    QgsVectorFieldSymbolLayerWidget( const QgsVectorLayer *vl, QWidget *parent /TransferThis/ = 0 );

    static QgsSymbolLayerWidget *create( const QgsVectorLayer *vl ) /Factory/;
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
 * src/gui/symbology/qgsvectorfieldsymbollayerwidget.h                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
