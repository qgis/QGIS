class QgsMapToolIdentify : QgsMapTool
{
%TypeHeaderCode
#include <qgsmaptoolidentify.h>
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

      IdentifyResult( QgsMapLayer * layer, QgsFeature feature, QMap< QString, QString > derivedAttributes );

      IdentifyResult( QgsMapLayer * layer, QString label, QMap< QString, QString > attributes, QMap< QString, QString > derivedAttributes );

      IdentifyResult( QgsMapLayer * layer, QString label, QgsFields fields, QgsFeature feature, QMap< QString, QString > derivedAttributes );

      QgsMapLayer* mLayer;
      QString mLabel;
      QgsFields mFields;
      QgsFeature mFeature;
      QMap< QString, QString > mAttributes;
      QMap< QString, QString > mDerivedAttributes;
      QMap< QString, QVariant > mParams;
    };

    //! constructor
    QgsMapToolIdentify( QgsMapCanvas * canvas );

    virtual ~QgsMapToolIdentify();

    //! Overridden mouse move event
    virtual void canvasMoveEvent( QMouseEvent * e );

    //! Overridden mouse press event
    virtual void canvasPressEvent( QMouseEvent * e );

    //! Overridden mouse release event
    virtual void canvasReleaseEvent( QMouseEvent * e );

    virtual void activate();

    virtual void deactivate();

    /** Performs the identification.
    @param x x coordinates of mouseEvent
    @param y y coordinates of mouseEvent
    @param layerList Performs the identification within the given list of layers. Default value is an empty list, i.e. uses all the layers.
    @param mode Identification mode. Can use Qgis default settings or a defined mode. Default mode is DefaultQgsSetting.
    @return a list of IdentifyResult*/
    QList<QgsMapToolIdentify::IdentifyResult> identify( int x, int y, QList<QgsMapLayer*> layerList = QList<QgsMapLayer*>(), IdentifyMode mode = DefaultQgsSetting );

    /** Performs the identification.
    To avoid beeing forced to specify IdentifyMode with a list of layers
    this has been made private and two publics methods are offered
    @param x x coordinates of mouseEvent
    @param y y coordinates of mouseEvent
    @param mode Identification mode. Can use Qgis default settings or a defined mode.
    @param layerType Only performs identification in a certain type of layers (raster, vector). Default value is AllLayers.
    @return a list of IdentifyResult*/
    QList<QgsMapToolIdentify::IdentifyResult> identify( int x, int y, IdentifyMode mode, LayerType layerType = AllLayers );

  public slots:
    void formatChanged( QgsRasterLayer *layer );
    void layerDestroyed();

  signals:
    void identifyProgress( int, int );
    void identifyMessage( QString );
    void changedRasterResults( QList<QgsMapToolIdentify::IdentifyResult>& );

  protected:
    //! rubber bands for layer select mode
    void deleteRubberBands();

    /** Performs the identification.
    To avoid beeing forced to specify IdentifyMode with a list of layers
    this has been made private and two publics methods are offered
    @param x x coordinates of mouseEvent
    @param y y coordinates of mouseEvent
    @param mode Identification mode. Can use Qgis default settings or a defined mode.
    @param layerList Performs the identification within the given list of layers.
    @param layerType Only performs identification in a certain type of layers (raster, vector).
    @return true if identification succeeded and a feature has been found, false otherwise.*/
    QList<QgsMapToolIdentify::IdentifyResult> identify( int x, int y, IdentifyMode mode,  QList<QgsMapLayer*> layerList, LayerType layerType = AllLayers );

    /** call the right method depending on layer type */
    bool identifyLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsMapLayer *layer, QgsPoint point, QgsRectangle viewExtent, double mapUnitsPerPixel, LayerType layerType = AllLayers );

    bool identifyRasterLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsRasterLayer *layer, QgsPoint point, QgsRectangle viewExtent, double mapUnitsPerPixel );
    bool identifyVectorLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsVectorLayer *layer, QgsPoint point );
};
