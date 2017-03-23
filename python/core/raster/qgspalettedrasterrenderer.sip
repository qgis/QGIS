class QgsPalettedRasterRenderer : QgsRasterRenderer
{
%TypeHeaderCode
    #include "qgspalettedrasterrenderer.h"
%End
  public:

    struct Class
    {

      Class( const QColor &color, const QString &label );
      QColor color;
      QString label;
    };

    typedef QMap< int, QgsPalettedRasterRenderer::Class > ClassData;
    QgsPalettedRasterRenderer( QgsRasterInterface *input, int bandNumber, const ClassData &classes );
    ~QgsPalettedRasterRenderer();
    virtual QgsPalettedRasterRenderer * clone() const /Factory/;
    static QgsRasterRenderer* create( const QDomElement& elem, QgsRasterInterface* input ) /Factory/;

    QgsRasterBlock *block( int bandNo, const QgsRectangle & extent, int width, int height, QgsRasterBlockFeedback* feedback = nullptr ) /Factory/;

    /** Returns number of colors*/
    int nColors() const;
    ClassData classes() const;
    QString label( int idx ) const;
    void setLabel( int idx, const QString &label );

    void writeXml( QDomDocument &doc, QDomElement &parentElem ) const;

    void legendSymbologyItems( QList< QPair< QString, QColor > > &symbolItems /Out/ ) const;

    QList<int> usesBands() const;
    void setSourceColorRamp( QgsColorRamp *ramp /Transfer/ );
    QgsColorRamp *sourceColorRamp() const;
    static QgsPalettedRasterRenderer::ClassData colorTableToClassData( const QList<QgsColorRampShader::ColorRampItem> &table );

  private:

    QgsPalettedRasterRenderer( const QgsPalettedRasterRenderer& );
    const QgsPalettedRasterRenderer& operator=( const QgsPalettedRasterRenderer& );
};
