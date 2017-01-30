%ModuleHeaderCode
#include <qgshtmlannotation.h>
#include <qgssvgannotation.h>
#include <qgstextannotation.h>
%End

class QgsAnnotation : QObject
{
%TypeHeaderCode
#include <qgsannotation.h>
%End

%ConvertToSubClassCode
  if ( dynamic_cast< QgsTextAnnotation* > ( sipCpp ) )
    sipType = sipType_QgsTextAnnotation;
  else if ( dynamic_cast< QgsSvgAnnotation* > ( sipCpp ) )
    sipType = sipType_QgsSvgAnnotation;
  else if ( dynamic_cast< QgsHtmlAnnotation* > ( sipCpp ) )
    sipType = sipType_QgsHtmlAnnotation;
  else
    sipType = NULL;
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

    void setFillSymbol( QgsFillSymbol* symbol /Transfer/ );
    QgsFillSymbol* fillSymbol() const;

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
