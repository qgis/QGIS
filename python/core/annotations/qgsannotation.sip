class QgsAnnotation : QObject
{
%TypeHeaderCode
#include <qgsannotation.h>
%End
  public:

    QgsAnnotation( QObject* parent /TransferThis/ = nullptr );

    bool isVisible() const;
    void setVisible( bool visible );

    bool hasFixedMapPosition() const;
    void setHasFixedMapPosition( bool fixed );

    QgsPoint mapPosition() const;
    void setMapPosition( const QgsPoint& position );

    QgsCoordinateReferenceSystem mapPositionCrs() const;
    void setMapPositionCrs( const QgsCoordinateReferenceSystem& crs );

    QPointF relativePosition() const;
    void setRelativePosition( QPointF position );

    void setFrameOffsetFromReferencePoint( QPointF offset );
    QPointF frameOffsetFromReferencePoint() const;

    void setFrameSize( QSizeF size );
    QSizeF frameSize() const;

    void setContentsMargin( const QgsMargins& margins );
    QgsMargins contentsMargin() const;

    void setFrameBorderWidth( double width );
    double frameBorderWidth() const;

    void setFrameColor( const QColor& color );
    QColor frameColor() const;

    void setFrameBackgroundColor( const QColor& color );
    QColor frameBackgroundColor() const;

    void render( QgsRenderContext& context ) const;

    virtual void writeXml( QDomElement& elem, QDomDocument & doc ) const = 0;
    virtual void readXml( const QDomElement& itemElem, const QDomDocument& doc ) = 0;

    void setMarkerSymbol( QgsMarkerSymbol* symbol /Transfer/ );
    QgsMarkerSymbol* markerSymbol() const;

    QgsMapLayer* mapLayer() const;
    void setMapLayer( QgsMapLayer* layer );

    QgsFeature associatedFeature() const;
    virtual void setAssociatedFeature( const QgsFeature& feature );

  signals:

    void appearanceChanged();
    void moved();

  protected:

    virtual void renderAnnotation( QgsRenderContext& context, QSizeF size ) const = 0;

    virtual QSizeF minimumFrameSize() const;

    void _writeXml( QDomElement& itemElem, QDomDocument& doc ) const;
    void _readXml( const QDomElement& annotationElem, const QDomDocument& doc );

};
