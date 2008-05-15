
class QgsSymbol
{
%TypeHeaderCode
#include <qgssymbol.h>
%End

 public:
    /**Constructor*/
    QgsSymbol(QGis::VectorType t, QString lvalue="", QString uvalue="", QString label="");
    /**Constructor*/
    QgsSymbol(QGis::VectorType t, QString lvalue, QString uvalue, QString label, QColor c);
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
    virtual void setPointSize(int s);
    /**Get size*/
    virtual int pointSize() const;
    //! Destructor
    virtual ~QgsSymbol();

    //! Get a little icon for the legend
    virtual QImage getLineSymbolAsImage();

    //! Get a little icon for the legend
    virtual QImage getPolygonSymbolAsImage();
    
    /** Get QImage representation of point symbol with current settings
      */
    virtual QImage getPointSymbolAsImage( double widthScale = 1, 
	               bool selected = false, QColor selectionColor = Qt::yellow );

    /**Writes the contents of the symbol to a configuration file
     @ return true in case of success*/
    virtual bool writeXML( QDomNode & item, QDomDocument & document ) const;
    /**Reads the contents of the symbol from a configuration file
     @ return true in case of success*/
    virtual bool readXML( QDomNode & symbol );
    /**Returns if this symbol is point/ line or polygon*/
    QGis::VectorType type() const;

};


