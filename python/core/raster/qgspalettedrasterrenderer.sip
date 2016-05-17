class QgsPalettedRasterRenderer : QgsRasterRenderer
{
%TypeHeaderCode
    #include "qgspalettedrasterrenderer.h"
%End
  public:
    /** Renderer owns color array*/
    QgsPalettedRasterRenderer( QgsRasterInterface* input, int bandNumber, QColor* colorArray /Array,Transfer/, int nColors /ArraySize/, const QVector<QString>& labels = QVector<QString>() );
    ~QgsPalettedRasterRenderer();
    virtual QgsPalettedRasterRenderer * clone() const /Factory/;
    static QgsRasterRenderer* create( const QDomElement& elem, QgsRasterInterface* input ) /Factory/;

    QgsRasterBlock *block( int bandNo, const QgsRectangle & extent, int width, int height ) /Factory/;

    /** Returns number of colors*/
    int nColors() const;
    /** Returns copy of color array (caller takes ownership)*/
    QColor* colors() const /Factory/;

    /** Returns copy of rgb array (caller takes ownership)
     @note not available in python bindings
     */
    // QRgb* rgbArray() const;

    /** Return optional category label
     *  @note added in 2.1 */
    QString label( int idx ) const;

    /** Set category label
     *  @note added in 2.1 */
    void setLabel( int idx, const QString& label );

    void writeXML( QDomDocument& doc, QDomElement& parentElem ) const;

    void legendSymbologyItems( QList< QPair< QString, QColor > >& symbolItems ) const;

    QList<int> usesBands() const;

  private:

    QgsPalettedRasterRenderer( const QgsPalettedRasterRenderer& );
    const QgsPalettedRasterRenderer& operator=( const QgsPalettedRasterRenderer& );
};
