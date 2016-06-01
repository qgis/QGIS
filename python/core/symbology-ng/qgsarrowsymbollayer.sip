class QgsArrowSymbolLayer : public QgsLineSymbolLayerV2
{
%TypeHeaderCode
#include <qgsarrowsymbollayer.h>
%End
  public:
    /** Simple constructor */
    QgsArrowSymbolLayer();

    /**
     * Create a new QgsArrowSymbolLayer
     *
     * @param properties A property map to deserialize saved information from properties()
     *
     * @return A new QgsArrowSymbolLayer
     */
    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;

    /** Virtual constructor */
    virtual QgsArrowSymbolLayer* clone() const /Factory/;

    /** Get the sub symbol used for filling */
    virtual QgsSymbolV2* subSymbol();

    /** Set the sub symbol used for filling */
    virtual bool setSubSymbol( QgsSymbolV2* symbol /Transfer/ );

    /** Return a list of attributes required to render this feature */
    virtual QSet<QString> usedAttributes() const;

     /** Get current arrow width */
    double arrowWidth() const;
    /** Set the arrow width */
    void setArrowWidth( double w );
    /** Get the unit for the arrow width */
    QgsSymbolV2::OutputUnit arrowWidthUnit() const;
    /** Set the unit for the arrow width */
    void setArrowWidthUnit( QgsSymbolV2::OutputUnit u );
    /** Get the scale for the arrow width */
    QgsMapUnitScale arrowWidthUnitScale() const;
    /** Set the scale for the arrow width */
    void setArrowWidthUnitScale( const QgsMapUnitScale& s );

    /** Get current arrow start width. Only meaningfull for single headed arrows */
    double arrowStartWidth() const;
    /** Set the arrow start width */
    void setArrowStartWidth( double w );
    /** Get the unit for the arrow start width */
    QgsSymbolV2::OutputUnit arrowStartWidthUnit() const;
    /** Set the unit for the arrow start width */
    void setArrowStartWidthUnit( QgsSymbolV2::OutputUnit u );
    /** Get the scale for the arrow start width */
    QgsMapUnitScale arrowStartWidthUnitScale() const;
    /** Set the scale for the arrow start width */
    void setArrowStartWidthUnitScale( const QgsMapUnitScale& s );

    /** Get the current arrow head width */
    double headWidth() const;
    /** Set the arrow head width */
    void setHeadWidth( double s );
    /** Get the unit for the head width */
    QgsSymbolV2::OutputUnit headWidthUnit() const;
    /** Set the unit for the head width */
    void setHeadWidthUnit( QgsSymbolV2::OutputUnit u );
    /** Get the scale for the head width */
    QgsMapUnitScale headWidthUnitScale() const;
    /** Set the scale for the head width */
    void setHeadWidthUnitScale( const QgsMapUnitScale& s );

    /** Get the current arrow head height */
    double headHeight() const;
    /** Set the arrow head height */
    void setHeadHeight( double s );
    /** Get the unit for the head height */
    QgsSymbolV2::OutputUnit headHeightUnit() const;
    /** Set the unit for the head height */
    void setHeadHeightUnit( QgsSymbolV2::OutputUnit u );
    /** Get the scale for the head height */
    QgsMapUnitScale headHeightUnitScale() const;
    /** Set the scale for the head height */
    void setHeadHeightUnitScale( const QgsMapUnitScale& s );

    /** Return whether it is a curved arrow or a straight one */
    bool isCurved() const;
    /** Set whether it is a curved arrow or a straight one */
    void setIsCurved( bool isCurved );

    /** Return whether the arrow is repeated along the line or not */
    bool isRepeated() const;
    /** Set whether the arrow is repeated along the line */
    void setIsRepeated( bool isRepeated );

    /** Possible head types */
    enum HeadType
    {
      HeadSingle,   //< One single head at the end
      HeadReversed, //< One single head at the beginning
      HeadDouble    //< Two heads
    };

    /** Get the current head type */
    HeadType headType() const;
    /** Set the head type */
    void setHeadType( HeadType t );

    /** Possible arrow types */
    enum ArrowType
    {
      ArrowPlain,     //< Regular arrow
      ArrowLeftHalf,  //< Halved arrow, only the left side of the arrow is rendered (for straight arrows) or the side toward the exterior (for curved arrows)
      ArrowRightHalf  //< Halved arrow, only the right side of the arrow is rendered (for straight arrows) or the side toward the interior (for curved arrows)
    };

    /** Get the current arrow type */
    ArrowType arrowType() const;
    /** Set the arrow type */
    void setArrowType( ArrowType t );

    /**
     * Should be reimplemented by subclasses to return a string map that
     * contains the configuration information for the symbol layer. This
     * is used to serialize a symbol layer perstistently.
     */
    QgsStringMap properties() const;

    /**
     * Returns a string that represents this layer type. Used for serialization.
     * Should match with the string used to register this symbol layer in the registry.
     */
    QString layerType() const;

    /** Prepare the rendering */
    void startRender( QgsSymbolV2RenderContext& context );

    /** End of the rendering */
    void stopRender( QgsSymbolV2RenderContext& context );

    /** Main drawing method */
    void renderPolyline( const QPolygonF& points, QgsSymbolV2RenderContext& context );

    void setColor( const QColor& c );
    virtual QColor color() const;

  private:
    QgsArrowSymbolLayer( const QgsArrowSymbolLayer& );
};
