/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmaptoolidentify.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsMapToolIdentify : QgsMapTool
{
%Docstring
 Map tool for identifying features in layers

after selecting a point, performs the identification:
- for raster layers shows value of underlying pixel
- for vector layers shows feature attributes within search radius
(allows editing values when vector layer is in editing mode)
%End

%TypeHeaderCode
#include "qgsmaptoolidentify.h"
%End
  public:

    enum IdentifyMode
    {
      DefaultQgsSetting,
      ActiveLayer,
      TopDownStopAtFirst,
      TopDownAll,
      LayerSelection
    };

    enum Type
    {
      VectorLayer,
      RasterLayer,
      AllLayers
    };
    typedef QFlags<QgsMapToolIdentify::Type> LayerType;


    struct IdentifyResult
    {
      IdentifyResult();
%Docstring
Constructor for IdentifyResult
%End

      IdentifyResult( QgsMapLayer *layer, const QgsFeature &feature, const QMap< QString, QString > &derivedAttributes );
      IdentifyResult( QgsMapLayer *layer, const QString &label, const QMap< QString, QString > &attributes, const QMap< QString, QString > &derivedAttributes );
      IdentifyResult( QgsMapLayer *layer, const QString &label, const QgsFields &fields, const QgsFeature &feature, const QMap< QString, QString > &derivedAttributes );
      QgsMapLayer *mLayer;
      QString mLabel;
      QgsFields mFields;
      QgsFeature mFeature;
      QMap< QString, QString > mAttributes;
      QMap< QString, QString > mDerivedAttributes;
      QMap< QString, QVariant > mParams;
    };

    QgsMapToolIdentify( QgsMapCanvas *canvas );
%Docstring
constructor
%End

    virtual ~QgsMapToolIdentify();

    virtual Flags flags() const;
    virtual void canvasMoveEvent( QgsMapMouseEvent *e );
    virtual void canvasPressEvent( QgsMapMouseEvent *e );
    virtual void canvasReleaseEvent( QgsMapMouseEvent *e );
    virtual void activate();
    virtual void deactivate();

    QList<QgsMapToolIdentify::IdentifyResult> identify( int x, int y, const QList<QgsMapLayer *> &layerList = QList<QgsMapLayer *>(), IdentifyMode mode = DefaultQgsSetting );
%Docstring
 Performs the identification.
\param x x coordinates of mouseEvent
\param y y coordinates of mouseEvent
\param layerList Performs the identification within the given list of layers. Default value is an empty list, i.e. uses all the layers.
\param mode Identification mode. Can use Qgis default settings or a defined mode. Default mode is DefaultQgsSetting.
:return: a list of IdentifyResult*
 :rtype: list of QgsMapToolIdentify.IdentifyResult
%End

    QList<QgsMapToolIdentify::IdentifyResult> identify( int x, int y, IdentifyMode mode, LayerType layerType = AllLayers );
%Docstring
 Performs the identification.
To avoid being forced to specify IdentifyMode with a list of layers
this has been made private and two publics methods are offered
\param x x coordinates of mouseEvent
\param y y coordinates of mouseEvent
\param mode Identification mode. Can use Qgis default settings or a defined mode.
\param layerType Only performs identification in a certain type of layers (raster, vector). Default value is AllLayers.
:return: a list of IdentifyResult*
 :rtype: list of QgsMapToolIdentify.IdentifyResult
%End

    QgsIdentifyMenu *identifyMenu();
%Docstring
 return a pointer to the identify menu which will be used in layer selection mode
 this menu can also be customized
 :rtype: QgsIdentifyMenu
%End

  public slots:
    void formatChanged( QgsRasterLayer *layer );

  signals:
    void identifyProgress( int, int );
    void identifyMessage( const QString & );
    void changedRasterResults( QList<QgsMapToolIdentify::IdentifyResult> & );

  protected:

    QList<QgsMapToolIdentify::IdentifyResult> identify( int x, int y, IdentifyMode mode,  const QList<QgsMapLayer *> &layerList, LayerType layerType = AllLayers );
%Docstring
 Performs the identification.
To avoid being forced to specify IdentifyMode with a list of layers
this has been made private and two publics methods are offered
\param x x coordinates of mouseEvent
\param y y coordinates of mouseEvent
\param mode Identification mode. Can use Qgis default settings or a defined mode.
\param layerList Performs the identification within the given list of layers.
\param layerType Only performs identification in a certain type of layers (raster, vector).
:return: a list of IdentifyResult*
 :rtype: list of QgsMapToolIdentify.IdentifyResult
%End


    bool identifyLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsMapLayer *layer, const QgsPointXY &point, const QgsRectangle &viewExtent, double mapUnitsPerPixel, QgsMapToolIdentify::LayerType layerType = AllLayers );
%Docstring
Call the right method depending on layer type
 :rtype: bool
%End

    bool identifyRasterLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsRasterLayer *layer, QgsPointXY point, const QgsRectangle &viewExtent, double mapUnitsPerPixel );
%Docstring
 :rtype: bool
%End
    bool identifyVectorLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsVectorLayer *layer, const QgsPointXY &point );
%Docstring
 :rtype: bool
%End

};

QFlags<QgsMapToolIdentify::Type> operator|(QgsMapToolIdentify::Type f1, QFlags<QgsMapToolIdentify::Type> f2);


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmaptoolidentify.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
