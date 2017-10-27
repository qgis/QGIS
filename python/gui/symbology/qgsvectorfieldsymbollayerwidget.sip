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

    QgsVectorFieldSymbolLayerWidget( QgsVectorLayer *vl, QWidget *parent /TransferThis/ = 0 );
%Docstring
 Constructor for QgsVectorFieldSymbolLayerWidget.
 \param vl associated vector layer
 \param parent parent widget
%End

    static QgsSymbolLayerWidget *create( QgsVectorLayer *vl ) /Factory/;
%Docstring
 Creates a new QgsVectorFieldSymbolLayerWidget.
 \param vl associated vector layer
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
