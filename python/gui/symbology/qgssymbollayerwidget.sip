/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgssymbollayerwidget.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsSymbolLayerWidget : QWidget, protected QgsExpressionContextGenerator
{

%TypeHeaderCode
#include "qgssymbollayerwidget.h"
%End
  public:
    QgsSymbolLayerWidget( QWidget *parent /TransferThis/, const QgsVectorLayer *vl = 0 );

    virtual void setSymbolLayer( QgsSymbolLayer *layer ) = 0;
    virtual QgsSymbolLayer *symbolLayer() = 0;
%Docstring
 :rtype: QgsSymbolLayer
%End

    void setContext( const QgsSymbolWidgetContext &context );
%Docstring
 Sets the context in which the symbol widget is shown, e.g., the associated map canvas and expression contexts.
 \param context symbol widget context
.. seealso:: context()
.. versionadded:: 3.0
%End

    QgsSymbolWidgetContext context() const;
%Docstring
 Returns the context in which the symbol widget is shown, e.g., the associated map canvas and expression contexts.
.. seealso:: setContext()
.. versionadded:: 3.0
 :rtype: QgsSymbolWidgetContext
%End

    const QgsVectorLayer *vectorLayer() const;
%Docstring
 Returns the vector layer associated with the widget.
.. versionadded:: 2.12
 :rtype: QgsVectorLayer
%End

  protected:

    void registerDataDefinedButton( QgsPropertyOverrideButton *button, QgsSymbolLayer::Property key );
%Docstring
 Registers a data defined override button. Handles setting up connections
 for the button and initializing the button to show the correct descriptions
 and help text for the associated property.
.. versionadded:: 3.0
%End

    virtual QgsExpressionContext createExpressionContext() const;


  signals:

    void changed();
%Docstring
 Should be emitted whenever configuration changes happened on this symbol layer configuration.
 If the subsymbol is changed, symbolChanged() should be emitted instead.
%End

    void symbolChanged();
%Docstring
 Should be emitted whenever the sub symbol changed on this symbol layer configuration.
 Normally changed() should be preferred.

.. seealso:: changed()
%End

  protected slots:
    void updateDataDefinedProperty();

};




class QgsSimpleLineSymbolLayerWidget : QgsSymbolLayerWidget
{

%TypeHeaderCode
#include "qgssymbollayerwidget.h"
%End
  public:
    QgsSimpleLineSymbolLayerWidget( const QgsVectorLayer *vl, QWidget *parent /TransferThis/ = 0 );

    static QgsSymbolLayerWidget *create( const QgsVectorLayer *vl ) /Factory/;
%Docstring
 :rtype: QgsSymbolLayerWidget
%End

    virtual void setSymbolLayer( QgsSymbolLayer *layer );
    virtual QgsSymbolLayer *symbolLayer();

  protected:

    void updatePatternIcon();

};




class QgsSimpleMarkerSymbolLayerWidget : QgsSymbolLayerWidget
{

%TypeHeaderCode
#include "qgssymbollayerwidget.h"
%End
  public:
    QgsSimpleMarkerSymbolLayerWidget( const QgsVectorLayer *vl, QWidget *parent /TransferThis/ = 0 );

    static QgsSymbolLayerWidget *create( const QgsVectorLayer *vl ) /Factory/;
%Docstring
 :rtype: QgsSymbolLayerWidget
%End

    virtual void setSymbolLayer( QgsSymbolLayer *layer );
    virtual QgsSymbolLayer *symbolLayer();

  public slots:

    void setColorStroke( const QColor &color );
    void setColorFill( const QColor &color );

  protected:

};




class QgsSimpleFillSymbolLayerWidget : QgsSymbolLayerWidget
{

%TypeHeaderCode
#include "qgssymbollayerwidget.h"
%End
  public:
    QgsSimpleFillSymbolLayerWidget( const QgsVectorLayer *vl, QWidget *parent /TransferThis/ = 0 );

    static QgsSymbolLayerWidget *create( const QgsVectorLayer *vl ) /Factory/;
%Docstring
 :rtype: QgsSymbolLayerWidget
%End

    virtual void setSymbolLayer( QgsSymbolLayer *layer );
    virtual QgsSymbolLayer *symbolLayer();

  public slots:
    void setColor( const QColor &color );
    void setStrokeColor( const QColor &color );

  protected:

};





class QgsFilledMarkerSymbolLayerWidget : QgsSymbolLayerWidget
{
%Docstring
 Widget for configuring QgsFilledMarkerSymbolLayer symbol layers.
.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgssymbollayerwidget.h"
%End
  public:

    QgsFilledMarkerSymbolLayerWidget( const QgsVectorLayer *vl, QWidget *parent /TransferThis/ = 0 );
%Docstring
 Constructor for QgsFilledMarkerSymbolLayerWidget.
 \param vl associated vector layer
 \param parent parent widget
%End

    static QgsSymbolLayerWidget *create( const QgsVectorLayer *vl ) /Factory/;
%Docstring
 Creates a new QgsFilledMarkerSymbolLayerWidget.
 \param vl associated vector layer
 :rtype: QgsSymbolLayerWidget
%End

    virtual void setSymbolLayer( QgsSymbolLayer *layer );
    virtual QgsSymbolLayer *symbolLayer();

  protected:

};




class QgsGradientFillSymbolLayerWidget : QgsSymbolLayerWidget
{

%TypeHeaderCode
#include "qgssymbollayerwidget.h"
%End
  public:
    QgsGradientFillSymbolLayerWidget( const QgsVectorLayer *vl, QWidget *parent /TransferThis/ = 0 );

    static QgsSymbolLayerWidget *create( const QgsVectorLayer *vl ) /Factory/;
%Docstring
 :rtype: QgsSymbolLayerWidget
%End

    virtual void setSymbolLayer( QgsSymbolLayer *layer );
    virtual QgsSymbolLayer *symbolLayer();

  public slots:
    void setColor( const QColor &color );
    void setColor2( const QColor &color );

    void applyColorRamp();
%Docstring
 Applies the color ramp passed on by the color ramp button
%End
    void setGradientType( int index );
    void setCoordinateMode( int index );
    void setGradientSpread( int index );

  protected:

};




class QgsShapeburstFillSymbolLayerWidget : QgsSymbolLayerWidget
{

%TypeHeaderCode
#include "qgssymbollayerwidget.h"
%End
  public:
    QgsShapeburstFillSymbolLayerWidget( const QgsVectorLayer *vl, QWidget *parent /TransferThis/ = 0 );

    static QgsSymbolLayerWidget *create( const QgsVectorLayer *vl ) /Factory/;
%Docstring
 :rtype: QgsSymbolLayerWidget
%End

    virtual void setSymbolLayer( QgsSymbolLayer *layer );
    virtual QgsSymbolLayer *symbolLayer();

  public slots:
    void setColor( const QColor &color );
    void setColor2( const QColor &color );

  protected:

};




class QgsMarkerLineSymbolLayerWidget : QgsSymbolLayerWidget
{

%TypeHeaderCode
#include "qgssymbollayerwidget.h"
%End
  public:
    QgsMarkerLineSymbolLayerWidget( const QgsVectorLayer *vl, QWidget *parent /TransferThis/ = 0 );

    static QgsSymbolLayerWidget *create( const QgsVectorLayer *vl ) /Factory/;
%Docstring
 :rtype: QgsSymbolLayerWidget
%End

    virtual void setSymbolLayer( QgsSymbolLayer *layer );
    virtual QgsSymbolLayer *symbolLayer();

  public slots:

    void setInterval( double val );
    void setOffsetAlongLine( double val );

  protected:


};





class QgsSvgMarkerSymbolLayerWidget : QgsSymbolLayerWidget
{

%TypeHeaderCode
#include "qgssymbollayerwidget.h"
%End
  public:
    QgsSvgMarkerSymbolLayerWidget( const QgsVectorLayer *vl, QWidget *parent /TransferThis/ = 0 );

    static QgsSymbolLayerWidget *create( const QgsVectorLayer *vl ) /Factory/;
%Docstring
 :rtype: QgsSymbolLayerWidget
%End

    virtual void setSymbolLayer( QgsSymbolLayer *layer );
    virtual QgsSymbolLayer *symbolLayer();

  protected:

    void populateList();
    void setGuiForSvg( const QgsSvgMarkerSymbolLayer *layer );


};




class QgsRasterFillSymbolLayerWidget : QgsSymbolLayerWidget
{

%TypeHeaderCode
#include "qgssymbollayerwidget.h"
%End
  public:
    QgsRasterFillSymbolLayerWidget( const QgsVectorLayer *vl, QWidget *parent /TransferThis/ = 0 );

    static QgsSymbolLayerWidget *create( const QgsVectorLayer *vl ) /Factory/;
%Docstring
 :rtype: QgsSymbolLayerWidget
%End

    virtual void setSymbolLayer( QgsSymbolLayer *layer );
    virtual QgsSymbolLayer *symbolLayer();

  protected:

};




class QgsSVGFillSymbolLayerWidget : QgsSymbolLayerWidget
{

%TypeHeaderCode
#include "qgssymbollayerwidget.h"
%End
  public:
    QgsSVGFillSymbolLayerWidget( const QgsVectorLayer *vl, QWidget *parent /TransferThis/ = 0 );

    static QgsSymbolLayerWidget *create( const QgsVectorLayer *vl ) /Factory/;
%Docstring
 :rtype: QgsSymbolLayerWidget
%End

    virtual void setSymbolLayer( QgsSymbolLayer *layer );
    virtual QgsSymbolLayer *symbolLayer();

  protected:
    void insertIcons();

    void updateParamGui( bool resetValues = true );
%Docstring
 Enables or disables svg fill color, stroke color and stroke width based on whether the
 svg file supports custom parameters.
 \param resetValues set to true to overwrite existing layer fill color, stroke color and stroke width
 with default values from svg file
%End

};




class QgsLinePatternFillSymbolLayerWidget : QgsSymbolLayerWidget
{

%TypeHeaderCode
#include "qgssymbollayerwidget.h"
%End
  public:

    QgsLinePatternFillSymbolLayerWidget( const QgsVectorLayer *vl, QWidget *parent /TransferThis/ = 0 );
    static QgsSymbolLayerWidget *create( const QgsVectorLayer *vl ) /Factory/;
%Docstring
 :rtype: QgsSymbolLayerWidget
%End

    virtual void setSymbolLayer( QgsSymbolLayer *layer );
    virtual QgsSymbolLayer *symbolLayer();

  protected:

};




class QgsPointPatternFillSymbolLayerWidget: QgsSymbolLayerWidget
{

%TypeHeaderCode
#include "qgssymbollayerwidget.h"
%End
  public:
    QgsPointPatternFillSymbolLayerWidget( const QgsVectorLayer *vl, QWidget *parent /TransferThis/ = 0 );
    static QgsSymbolLayerWidget *create( const QgsVectorLayer *vl ) /Factory/;
%Docstring
 :rtype: QgsSymbolLayerWidget
%End

    virtual void setSymbolLayer( QgsSymbolLayer *layer );
    virtual QgsSymbolLayer *symbolLayer();

  protected:

};




class QgsFontMarkerSymbolLayerWidget : QgsSymbolLayerWidget
{

%TypeHeaderCode
#include "qgssymbollayerwidget.h"
%End
  public:
    QgsFontMarkerSymbolLayerWidget( const QgsVectorLayer *vl, QWidget *parent /TransferThis/ = 0 );

    static QgsSymbolLayerWidget *create( const QgsVectorLayer *vl ) /Factory/;
%Docstring
 :rtype: QgsSymbolLayerWidget
%End

    virtual void setSymbolLayer( QgsSymbolLayer *layer );
    virtual QgsSymbolLayer *symbolLayer();

  public slots:
    void setFontFamily( const QFont &font );
    void setColor( const QColor &color );

    void setColorStroke( const QColor &color );
%Docstring
 Set stroke color.
.. versionadded:: 2.16
%End
    void setSize( double size );
    void setAngle( double angle );
    void setCharacter( QChar chr );

  protected:

};





class QgsCentroidFillSymbolLayerWidget : QgsSymbolLayerWidget
{

%TypeHeaderCode
#include "qgssymbollayerwidget.h"
%End
  public:
    QgsCentroidFillSymbolLayerWidget( const QgsVectorLayer *vl, QWidget *parent /TransferThis/ = 0 );

    static QgsSymbolLayerWidget *create( const QgsVectorLayer *vl ) /Factory/;
%Docstring
 :rtype: QgsSymbolLayerWidget
%End

    virtual void setSymbolLayer( QgsSymbolLayer *layer );
    virtual QgsSymbolLayer *symbolLayer();

  protected:

};




class QgsGeometryGeneratorSymbolLayerWidget : QgsSymbolLayerWidget
{

%TypeHeaderCode
#include "qgssymbollayerwidget.h"
%End
  public:
    QgsGeometryGeneratorSymbolLayerWidget( const QgsVectorLayer *vl, QWidget *parent /TransferThis/ = 0 );

    static QgsSymbolLayerWidget *create( const QgsVectorLayer *vl ) /Factory/;
%Docstring
 Will be registered as factory
 :rtype: QgsSymbolLayerWidget
%End

    virtual void setSymbolLayer( QgsSymbolLayer *layer );
    virtual QgsSymbolLayer *symbolLayer();

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgssymbollayerwidget.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
