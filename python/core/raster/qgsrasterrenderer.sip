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

    virtual QgsRasterInterface::DataType dataType( int bandNo ) const;

    virtual QString type() const;

    virtual bool setInput( QgsRasterInterface* input );

    virtual void * readBlock( int bandNo, const QgsRectangle &extent, int width, int height );

    bool usesTransparency() const;

    void setOpacity( double opacity );
    double opacity() const;

    void setRasterTransparency( QgsRasterTransparency* t );
    const QgsRasterTransparency* rasterTransparency() const;

    void setAlphaBand( int band );
    int alphaBand() const;

    void setInvertColor( bool invert );
    bool invertColor() const;

    /**Get symbology items if provided by renderer*/
    virtual void legendSymbologyItems( QList< QPair< QString, QColor > >& symbolItems ) const;

    virtual void writeXML( QDomDocument&, QDomElement& ) const;
    /**Sets base class members from xml. Usually called from create() methods of subclasses*/
    void readXML( const QDomElement& rendererElem );

    /**Returns a list of band numbers used by the renderer*/
    virtual QList<int> usesBands() const;
};
