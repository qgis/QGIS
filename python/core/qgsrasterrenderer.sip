class QgsRasterRenderer
{
%TypeHeaderCode
    #include "qgsrasterrenderer.h"
    #include "qgspalettedrasterrenderer.h"
    #include "qgsmultibandcolorrenderer.h"
    #include "qgssinglebandpseudocolorrenderer.h"
    #include "qgssinglebandgrayrenderer.h"
    #include "qgssinglebandcolordatarenderer.h"
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
    QgsRasterRenderer( QgsRasterDataProvider* provider, const QString& type );
    virtual ~QgsRasterRenderer();
    virtual QgsRasterInterface * clone() const = 0;

    virtual QString type() const;
    virtual void * readBlock( int bandNo, const QgsRectangle & extent, int width, int height );

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

    virtual void writeXML( QDomDocument& doc, QDomElement& parentElem ) const = 0;

    /**Sets base class members from xml. Usually called from create() methods of subclasses*/
    void readXML( const QDomElement& rendererElem );
 };

class QgsPalettedRasterRenderer: QgsRasterRenderer
{
%TypeHeaderCode
    #include "qgspalettedrasterrenderer.h"
%End
  public:
    /**Renderer owns color array*/
    QgsPalettedRasterRenderer( QgsRasterDataProvider* provider, int bandNumber, QColor* colorArray, int nColors );
    ~QgsPalettedRasterRenderer();
    QgsRasterInterface * clone();
    static QgsRasterRenderer* create( const QDomElement& elem, QgsRasterDataProvider* provider );

    void * readBlock( int bandNo, const QgsRectangle & extent, int width, int height );

    /**Returns number of colors*/
    int nColors() const;
    /**Returns copy of color array (caller takes ownership)*/
    QColor* colors() const;

    void writeXML( QDomDocument& doc, QDomElement& parentElem ) const;

    void legendSymbologyItems( QList< QPair< QString, QColor > >& symbolItems ) const;
 };

class QgsMultiBandColorRenderer: QgsRasterRenderer
{
%TypeHeaderCode
    #include "qgsmultibandcolorrenderer.h"
%End
  public:
    QgsMultiBandColorRenderer( QgsRasterDataProvider* provider, int redBand, int greenBand, int blueBand,
                               QgsContrastEnhancement* redEnhancement = 0, QgsContrastEnhancement* greenEnhancement = 0,
                               QgsContrastEnhancement* blueEnhancement = 0 );
    ~QgsMultiBandColorRenderer();
    QgsRasterInterface * clone();

    static QgsRasterRenderer* create( const QDomElement& elem, QgsRasterDataProvider* provider );

    void * readBlock( int bandNo, const QgsRectangle & extent, int width, int height );

    int redBand() const;
    void setRedBand( int band );
    int greenBand() const;
    void setGreenBand( int band );
    int blueBand() const;
    void setBlueBand( int band );

    const QgsContrastEnhancement* redContrastEnhancement() const;
    /**Takes ownership*/
    void setRedContrastEnhancement( QgsContrastEnhancement* ce );

    const QgsContrastEnhancement* greenContrastEnhancement() const;
    /**Takes ownership*/
    void setGreenContrastEnhancement( QgsContrastEnhancement* ce );

    const QgsContrastEnhancement* blueContrastEnhancement() const;
    /**Takes ownership*/
    void setBlueContrastEnhancement( QgsContrastEnhancement* ce );

    void writeXML( QDomDocument& doc, QDomElement& parentElem ) const;
};

class QgsSingleBandPseudoColorRenderer: QgsRasterRenderer
{
%TypeHeaderCode
    #include "qgssinglebandpseudocolorrenderer.h"
%End
  public:
    /**Note: takes ownership of QgsRasterShader*/
    QgsSingleBandPseudoColorRenderer( QgsRasterDataProvider* provider, int band, QgsRasterShader* shader );
    ~QgsSingleBandPseudoColorRenderer();
    QgsRasterInterface * clone();

    static QgsRasterRenderer* create( const QDomElement& elem, QgsRasterDataProvider* provider );

    void * readBlock( int bandNo, const QgsRectangle & extent, int width, int height );

    /**Takes ownership of the shader*/
    void setShader( QgsRasterShader* shader );
    QgsRasterShader* shader();

    void writeXML( QDomDocument& doc, QDomElement& parentElem ) const;

    void legendSymbologyItems( QList< QPair< QString, QColor > >& symbolItems ) const;
};

class QgsSingleBandGrayRenderer: QgsRasterRenderer
{
%TypeHeaderCode
    #include "qgssinglebandgrayrenderer.h"
%End
  public:
    QgsSingleBandGrayRenderer( QgsRasterDataProvider* provider, int grayBand );
    ~QgsSingleBandGrayRenderer();
    QgsRasterInterface * clone();

    static QgsRasterRenderer* create( const QDomElement& elem, QgsRasterDataProvider* provider );

    void * readBlock( int bandNo, const QgsRectangle & extent, int width, int height );

    int grayBand() const;
    void setGrayBand( int band );
    const QgsContrastEnhancement* contrastEnhancement() const;
    /**Takes ownership*/
    void setContrastEnhancement( QgsContrastEnhancement* ce );

    void writeXML( QDomDocument& doc, QDomElement& parentElem ) const;

    void legendSymbologyItems( QList< QPair< QString, QColor > >& symbolItems ) const;
};

class QgsSingleBandColorDataRenderer: QgsRasterRenderer
{
%TypeHeaderCode
    #include "qgssinglebandcolordatarenderer.h"
%End
  public:
    QgsSingleBandColorDataRenderer( QgsRasterDataProvider* provider, int band );
    ~QgsSingleBandColorDataRenderer();
    QgsRasterInterface * clone();

    static QgsRasterRenderer* create( const QDomElement& elem, QgsRasterDataProvider* provider );

    void * readBlock( int bandNo, const QgsRectangle & extent, int width, int height );

    void writeXML( QDomDocument& doc, QDomElement& parentElem ) const;
};
