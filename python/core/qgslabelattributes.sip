
class QgsLabelAttributes
{
%TypeHeaderCode
#include <qgslabelattributes.h>
%End

public:
    /** Constructor.
     *  @param def if true, defaults are set, if false all all attributes are unset
     */
    QgsLabelAttributes( bool def = true );

    /* Units type */
    enum Units
    {
      MapUnits,
      PointUnits
    };

    static QString unitsName( int units );
    static int unitsCode( const QString &name );

    static QString alignmentName( int alignment );
    static int alignmentCode( const QString &name );

    /* Text */
    void setText( const QString & text );
    bool textIsSet() const;
    const QString text() const;

    /* Font */
    void setFamily( const QString & family );
    bool familyIsSet() const;
    const QString family() const;

    void setBold( bool enable );
    bool boldIsSet() const;
    bool bold() const;

    void setItalic( bool enable );
    bool italicIsSet() const;
    bool italic() const;

    void setUnderline( bool enable );
    bool underlineIsSet() const;
    bool underline() const;

    void setStrikeOut( bool enable );
    bool strikeOutIsSet() const;
    bool strikeOut() const;

    void   setSize( double size, int type );
    bool   sizeIsSet() const;
    int    sizeType() const;
    double size() const;

    void  setColor( const QColor &color );
    bool  colorIsSet() const;
    const QColor & color() const;

    /* Offset */
    void   setOffset( double x, double y, int type );
    bool   offsetIsSet() const;
    int    offsetType() const;
    double xOffset() const;
    double yOffset() const;

    /* Angle */
    void   setAngle( double angle );
    bool   angleIsSet() const;
    double angle() const;

    bool   angleIsAuto() const;
    void   setAutoAngle( bool state );

    /* Alignment */
    void setAlignment( int alignment );
    bool alignmentIsSet() const;
    int  alignment() const;

    /* Buffer */
    bool   bufferEnabled() const;
    void   setBufferEnabled( bool useBufferFlag );
    void   setBufferSize( double size, int type );
    bool   bufferSizeIsSet() const;
    int    bufferSizeType() const;
    double bufferSize() const;

    void  setBufferColor( const QColor &color );
    bool  bufferColorIsSet() const;
    QColor bufferColor() const;

    void  setBufferStyle( Qt::BrushStyle style );
    bool  bufferStyleIsSet() const;
    Qt::BrushStyle bufferStyle() const;

    /* Border */
    void  setBorderColor( const QColor &color );
    bool  borderColorIsSet() const;
    QColor borderColor() const;

    void  setBorderWidth( int width );
    bool  borderWidthIsSet() const;
    int   borderWidth() const;

    void  setBorderStyle( Qt::PenStyle style );
    bool  borderStyleIsSet() const;
    Qt::PenStyle   borderStyle() const;

    bool  multilineEnabled() const;
    void  setMultilineEnabled( bool useMultiline );

    /* label only selected features */
    bool  selectedOnly() const;
    void  setSelectedOnly( bool selectedonly );
};

