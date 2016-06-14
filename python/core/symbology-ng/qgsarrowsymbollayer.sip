/** \ingroup core
 * \class QgsArrowSymbolLayer
 * \brief Line symbol layer used for representing lines as arrows.
 * \note Added in version 2.16
 */

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
    void setArrowWidth( double width );
    /** Get the unit for the arrow width */
    QgsSymbolV2::OutputUnit arrowWidthUnit() const;
    /** Set the unit for the arrow width */
    void setArrowWidthUnit( QgsSymbolV2::OutputUnit unit );
    /** Get the scale for the arrow width */
    QgsMapUnitScale arrowWidthUnitScale() const;
    /** Set the scale for the arrow width */
    void setArrowWidthUnitScale( const QgsMapUnitScale& scale );

    /** Get current arrow start width. Only meaningfull for single headed arrows */
    double arrowStartWidth() const;
    /** Set the arrow start width */
    void setArrowStartWidth( double width );
    /** Get the unit for the arrow start width */
    QgsSymbolV2::OutputUnit arrowStartWidthUnit() const;
    /** Set the unit for the arrow start width */
    void setArrowStartWidthUnit( QgsSymbolV2::OutputUnit unit );
    /** Get the scale for the arrow start width */
    QgsMapUnitScale arrowStartWidthUnitScale() const;
    /** Set the scale for the arrow start width */
    void setArrowStartWidthUnitScale( const QgsMapUnitScale& scale );

    /** Get the current arrow head length */
    double headLength() const;
    /** Set the arrow head length */
    void setHeadLength( double length );
    /** Get the unit for the head length */
    QgsSymbolV2::OutputUnit headLengthUnit() const;
    /** Set the unit for the head length */
    void setHeadLengthUnit( QgsSymbolV2::OutputUnit unit );
    /** Get the scale for the head length */
    QgsMapUnitScale headLengthUnitScale() const;
    /** Set the scale for the head length */
    void setHeadLengthUnitScale( const QgsMapUnitScale& scale );

    /** Get the current arrow head height */
    double headThickness() const;
    /** Set the arrow head height */
    void setHeadThickness( double thickness );
    /** Get the unit for the head height */
    QgsSymbolV2::OutputUnit headThicknessUnit() const;
    /** Set the unit for the head height */
    void setHeadThicknessUnit( QgsSymbolV2::OutputUnit unit );
    /** Get the scale for the head height */
    QgsMapUnitScale headThicknessUnitScale() const;
    /** Set the scale for the head height */
    void setHeadThicknessUnitScale( const QgsMapUnitScale& scale );

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
    void setHeadType( HeadType type );

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
    void setArrowType( ArrowType type );

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
