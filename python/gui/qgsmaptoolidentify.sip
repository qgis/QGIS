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

      IdentifyResult( QgsMapLayer * layer, const QgsFeature& feature, const QMap< QString, QString >& derivedAttributes );

      IdentifyResult( QgsMapLayer * layer, const QString& label, const QMap< QString, QString >& attributes, const QMap< QString, QString >& derivedAttributes );

      IdentifyResult( QgsMapLayer * layer, const QString& label, const QgsFields& fields, const QgsFeature& feature, const QMap< QString, QString >& derivedAttributes );

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

    virtual Flags flags() const;

    //! Overridden mouse move event
    virtual void canvasMoveEvent( QgsMapMouseEvent * e );

    //! Overridden mouse press event
    virtual void canvasPressEvent( QgsMapMouseEvent * e );

    //! Overridden mouse release event
    virtual void canvasReleaseEvent( QgsMapMouseEvent * e );

    virtual void activate();

    virtual void deactivate();

    /** Performs the identification.
    @param x x coordinates of mouseEvent
    @param y y coordinates of mouseEvent
    @param layerList Performs the identification within the given list of layers. Default value is an empty list, i.e. uses all the layers.
    @param mode Identification mode. Can use Qgis default settings or a defined mode. Default mode is DefaultQgsSetting.
    @return a list of IdentifyResult*/
    QList<QgsMapToolIdentify::IdentifyResult> identify( int x, int y, const QList<QgsMapLayer*>& layerList = QList<QgsMapLayer*>(), IdentifyMode mode = DefaultQgsSetting );

    /** Performs the identification.
    To avoid beeing forced to specify IdentifyMode with a list of layers
    this has been made private and two publics methods are offered
    @param x x coordinates of mouseEvent
    @param y y coordinates of mouseEvent
    @param mode Identification mode. Can use Qgis default settings or a defined mode.
    @param layerType Only performs identification in a certain type of layers (raster, vector). Default value is AllLayers.
    @return a list of IdentifyResult*/
    QList<QgsMapToolIdentify::IdentifyResult> identify( int x, int y, IdentifyMode mode, const LayerType& layerType = AllLayers );

    //! return a pointer to the identify menu which will be used in layer selection mode
    //! this menu can also be customized
    QgsIdentifyMenu* identifyMenu();

  public slots:
    void formatChanged( QgsRasterLayer *layer );

  signals:
    void identifyProgress( int, int );
    void identifyMessage( const QString& );
    void changedRasterResults( QList<QgsMapToolIdentify::IdentifyResult>& );

  protected:
    /** Performs the identification.
    To avoid beeing forced to specify IdentifyMode with a list of layers
    this has been made private and two publics methods are offered
    @param x x coordinates of mouseEvent
    @param y y coordinates of mouseEvent
    @param mode Identification mode. Can use Qgis default settings or a defined mode.
    @param layerList Performs the identification within the given list of layers.
    @param layerType Only performs identification in a certain type of layers (raster, vector).
    @return a list of IdentifyResult*/
    QList<QgsMapToolIdentify::IdentifyResult> identify( int x, int y, IdentifyMode mode, const QList<QgsMapLayer*>& layerList, const LayerType& layerType = AllLayers );

    /** Call the right method depending on layer type */
    bool identifyLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsMapLayer *layer, const QgsPoint& point, const QgsRectangle& viewExtent, double mapUnitsPerPixel, const LayerType& layerType = AllLayers );

    bool identifyRasterLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsRasterLayer *layer, QgsPoint point, const QgsRectangle& viewExtent, double mapUnitsPerPixel );
    bool identifyVectorLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsVectorLayer *layer, const QgsPoint& point );

  private:

    //! Private helper
    //! @deprecated use displayDistanceUnits() and displayAreaUnits() instead
    virtual void convertMeasurement( QgsDistanceArea &calc, double &measure, QGis::UnitType &u, bool isArea ) /Deprecated/;

    /** Transforms the measurements of derived attributes in the desired units
     * @deprecated use displayDistanceUnits() and displayAreaUnits() instead
    */
    virtual QGis::UnitType displayUnits() /Deprecated/;

    /** Desired units for distance display.
     * @note added in QGIS 2.14
     * @see displayAreaUnits()
     */
    virtual QGis::UnitType displayDistanceUnits() const;

    /** Desired units for area display.
     * @note added in QGIS 2.14
     * @see displayDistanceUnits()
     */
    virtual QgsUnitTypes::AreaUnit displayAreaUnits() const;
};
