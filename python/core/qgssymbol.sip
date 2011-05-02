
class QgsSymbol
{
%TypeHeaderCode
#include <qgssymbol.h>
%End

 public:
    /**Constructor*/
    QgsSymbol(QGis::GeometryType t, QString lvalue="", QString uvalue="", QString label="");
    /**Constructor*/
    QgsSymbol(QGis::GeometryType t, QString lvalue, QString uvalue, QString label, QColor c);
    QgsSymbol(const QgsSymbol&);
    /**old constructors*/
    QgsSymbol();
    QgsSymbol(QColor c);
    /**Sets the brush*/
    virtual void setBrush(QBrush b);
    /**Gets a reference to m_brush, Don't use the brush to change color/style */
    virtual const QBrush& brush() const;
    /**Set the color*/
    virtual void setColor(QColor c);
    /**Get the current color*/
    virtual QColor color() const;
    /**Get the fill color*/
    virtual QColor fillColor() const;
    /**Sets the fill color*/
    virtual void setFillColor(QColor c);
    /**Get the line width*/
    virtual double lineWidth() const;
    /**Sets the line width*/
    virtual void setLineWidth(double w);
    /**Sets the pen*/
    virtual void setPen(QPen p);
    /**Gets a reference to m_pen. Don't use the pen to change color/style  */
    virtual const QPen& pen() const;

    /**Gets the path to the customs texture image*/
    virtual QString customTexture() const;
    /**Sets the path to the custom texture, and sets the brush to use TexturePattern */
    virtual void setCustomTexture( QString path );

    /**Set the line (pen) style*/
    virtual void setLineStyle(Qt::PenStyle s);
    /**Set the fill (brush) style*/
    virtual void setFillStyle(Qt::BrushStyle s);
    
    virtual void setLowerValue(QString value);
    virtual QString lowerValue() const;
    virtual void setUpperValue(QString value);
    virtual QString upperValue() const;
    virtual void setLabel(QString label);
    virtual QString label() const;

    /**Set point symbol from name*/
    virtual void setNamedPointSymbol(QString name);
    /**Get point symbol*/
    virtual QString pointSymbolName() const;
    /**Set size*/
    virtual void setPointSize(double s);
    /**Get size*/
    virtual double pointSize() const;
    //! Destructor
    virtual ~QgsSymbol();

    //! Get a little icon for the legend
    virtual QImage getLineSymbolAsImage();

    //! Get a little icon for the legend
    virtual QImage getPolygonSymbolAsImage();
    
    /** Get QImage representation of point symbol with current settings
     * and scaled (can be slow when scale != 1.0)
     */
    virtual QImage getPointSymbolAsImage( double widthScale = 1.0,
                                          bool selected = false,
                                          QColor selectionColor = Qt::yellow,
                                          double scale = 1.0,
                                          double rotation = 0.0,
                                          double rasterScaleFactor = 1.0,
                                          double opacity = 1.0 );

    /**Writes the contents of the symbol to a configuration file
     @ return true in case of success*/
    virtual bool writeXML( QDomNode & item, QDomDocument & document, const QgsVectorLayer *vl ) const;
    /**Reads the contents of the symbol from a configuration file
     @ return true in case of success*/
    virtual bool readXML( QDomNode & symbol, const QgsVectorLayer *vl );
    /**Returns if this symbol is point/ line or polygon*/
    QGis::GeometryType type() const;

    /**Returns the number of the rotation classification field*/
    int rotationClassificationField() const;
    /**Sets the number of the rotation classicifation field
    \param field the number of the field to classify for rotation*/
    void setRotationClassificationField( int field );

    /**Returns the number of the scale classification field*/
    int scaleClassificationField() const;
    /**Sets the number of the scale classicifation field
    \param field the number of the field to classify for scale*/
    void setScaleClassificationField( int field );

};


