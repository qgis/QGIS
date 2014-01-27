
class QgsRasterRenderer : QgsRasterInterface
{
%TypeHeaderCode
    #include "qgsrasterrenderer.h"
    #include "qgspalettedrasterrenderer.h"
    #include "qgsmultibandcolorrenderer.h"
    #include "qgssinglebandpseudocolorrenderer.h"
    #include "qgssinglebandgrayrenderer.h"
    #include "qgssinglebandcolordatarenderer.h"
    #include "qgsrasterinterface.h"
%End

%ConvertToSubClassCode
  if (dynamic_cast<QgsPalettedRasterRenderer*>(sipCpp) != NULL)
    sipClass = sipClass_QgsPalettedRasterRenderer;
  else if (dynamic_cast<QgsMultiBandColorRenderer*>(sipCpp) != NULL)
    sipClass = sipClass_QgsMultiBandColorRenderer;
  else if (dynamic_cast<QgsSingleBandPseudoColorRenderer*>(sipCpp) != NULL)
    sipClass = sipClass_QgsSingleBandPseudoColorRenderer;
  else if (dynamic_cast<QgsSingleBandGrayRenderer*>(sipCpp) != NULL)
    sipClass = sipClass_QgsSingleBandGrayRenderer;
  else if (dynamic_cast<QgsSingleBandGrayRenderer*>(sipCpp) != NULL)
    sipClass = sipClass_QgsSingleBandGrayRenderer;
  else
    sipClass = 0;
%End

  public:
    QgsRasterRenderer( QgsRasterInterface* input = 0, const QString& type = "" );
    virtual ~QgsRasterRenderer();

    virtual QgsRasterInterface * clone() const = 0 /Factory/;

    virtual int bandCount() const;

    virtual QGis::DataType dataType( int bandNo ) const;

    virtual QString type() const;

    virtual bool setInput( QgsRasterInterface* input );

    virtual QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height ) = 0 / Factory /;

    bool usesTransparency() const;

    void setOpacity( double opacity );
    double opacity() const;

    void setRasterTransparency( QgsRasterTransparency* t /Transfer/);
    const QgsRasterTransparency* rasterTransparency() const;

    void setAlphaBand( int band );
    int alphaBand() const;

    /**Get symbology items if provided by renderer*/
    virtual void legendSymbologyItems( QList< QPair< QString, QColor > >& symbolItems ) const;

    /**Sets base class members from xml. Usually called from create() methods of subclasses*/
    void readXML( const QDomElement& rendererElem );

    /**Returns a list of band numbers used by the renderer*/
    virtual QList<int> usesBands() const;

    static QString minMaxOriginName( int theOrigin );
    static QString minMaxOriginLabel( int theOrigin );
    static int minMaxOriginFromName( QString theName );

  protected:

    /**Write upper class info into rasterrenderer element (called by writeXML method of subclasses)*/
    void _writeXML( QDomDocument& doc, QDomElement& rasterRendererElem ) const;

};
