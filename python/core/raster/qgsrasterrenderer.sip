
class QgsRasterRenderer : QgsRasterInterface
{
%TypeHeaderCode
#include "qgsrasterrenderer.h"
%End

  public:

    static const QRgb NODATA_COLOR;

    QgsRasterRenderer( QgsRasterInterface* input = 0, const QString& type = "" );
    virtual ~QgsRasterRenderer();

    virtual QgsRasterRenderer * clone() const = 0 /Factory/;

    virtual int bandCount() const;

    virtual Qgis::DataType dataType( int bandNo ) const;

    virtual QString type() const;

    virtual bool setInput( QgsRasterInterface* input );

    virtual QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback* feedback = nullptr ) = 0 / Factory /;

    bool usesTransparency() const;

    void setOpacity( double opacity );
    double opacity() const;

    void setRasterTransparency( QgsRasterTransparency* t /Transfer/);
    const QgsRasterTransparency* rasterTransparency() const;

    void setAlphaBand( int band );
    int alphaBand() const;

    /** Get symbology items if provided by renderer*/
    virtual void legendSymbologyItems( QList< QPair< QString, QColor > >& symbolItems /Out/ ) const;

    /** Sets base class members from xml. Usually called from create() methods of subclasses*/
    void readXml( const QDomElement& rendererElem );

    /** Copies common properties like opacity / transparency data from other renderer.
     *  Useful when cloning renderers.
     *  @note added in 2.16  */
    void copyCommonProperties( const QgsRasterRenderer* other, bool copyMinMaxOrigin = true );

    /** Returns a list of band numbers used by the renderer*/
    virtual QList<int> usesBands() const;

    //! Returns const reference to origin of min/max values
    const QgsRasterMinMaxOrigin& minMaxOrigin() const;

    //! Sets origin of min/max values
    void setMinMaxOrigin( const QgsRasterMinMaxOrigin& origin );

  protected:

    /** Write upper class info into rasterrenderer element (called by writeXML method of subclasses)*/
    void _writeXml( QDomDocument& doc, QDomElement& rasterRendererElem ) const;

  private:

    QgsRasterRenderer( const QgsRasterRenderer& );
    const QgsRasterRenderer& operator=( const QgsRasterRenderer& );

};
