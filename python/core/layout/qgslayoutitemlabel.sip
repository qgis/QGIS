/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitemlabel.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsLayoutItemLabel: QgsLayoutItem
{
%Docstring
 A layout item subclass for text labels.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutitemlabel.h"
%End
  public:

    enum Mode
    {
      ModeFont,
      ModeHtml,
    };

    QgsLayoutItemLabel( QgsLayout *layout );
%Docstring
 Constructor for QgsLayoutItemLabel, with the specified parent ``layout``.
%End

    static QgsLayoutItemLabel *create( QgsLayout *layout ) /Factory/;
%Docstring
 Returns a new label item for the specified ``layout``.

 The caller takes responsibility for deleting the returned object.
 :rtype: QgsLayoutItemLabel
%End


    virtual int type() const;

    virtual QString stringType() const;

    virtual QString displayName() const;

    void adjustSizeToText();
%Docstring
 Resizes the item so that the label's text fits to the item. Keeps the top left point stationary.
.. seealso:: sizeForText()
%End

    QSizeF sizeForText() const;
%Docstring
 Returns the required item size (in layout units) for the label's text to fill the item.
.. seealso:: adjustSizeToText()
 :rtype: QSizeF
%End

    QString text();
%Docstring
 Returns the label's preset text.
.. seealso:: currentText()
.. seealso:: setText()
 :rtype: str
%End

    void setText( const QString &text );
%Docstring
 Sets the label's preset ``text``.
.. seealso:: text()
%End

    QString currentText() const;
%Docstring
 Returns the text as it appears on the label (with evaluated expressions
 and other dynamic content).
.. seealso:: text()
 :rtype: str
%End

    Mode mode() const;
%Docstring
 Returns the label's current mode.
.. seealso:: setMode()
 :rtype: Mode
%End

    void setMode( Mode mode );
%Docstring
 Sets the label's current ``mode``, allowing the label
 to switch between font based and HTML based rendering.
.. seealso:: mode()
%End

    QFont font() const;
%Docstring
 Returns the label's current font.
.. seealso:: setFont()
 :rtype: QFont
%End

    void setFont( const QFont &font );
%Docstring
 Sets the label's current ``font``.
.. seealso:: font()
%End

    Qt::AlignmentFlag vAlign() const;
%Docstring
 Returns for the vertical alignment of the label.
.. seealso:: setVAlign()
.. seealso:: hAlign()
 :rtype: Qt.AlignmentFlag
%End

    Qt::AlignmentFlag hAlign() const;
%Docstring
 Returns the horizontal alignment of the label.
.. seealso:: vAlign()
.. seealso:: setHAlign()
 :rtype: Qt.AlignmentFlag
%End

    void setHAlign( Qt::AlignmentFlag alignment );
%Docstring
 Sets the horizontal ``alignment`` of the label.
.. seealso:: hAlign()
.. seealso:: setVAlign()
%End

    void setVAlign( Qt::AlignmentFlag alignment );
%Docstring
 Sets for the vertical ``alignment`` of the label.
.. seealso:: vAlign()
.. seealso:: setHAlign()
%End

    double marginX() const;
%Docstring
 Returns the horizontal margin between the edge of the frame and the label
 contents, in layout units.
.. seealso:: setMargin()
.. seealso:: marginY()
 :rtype: float
%End

    double marginY() const;
%Docstring
 Returns the vertical margin between the edge of the frame and the label
 contents, in layout units.
.. seealso:: setMargin()
.. seealso:: marginX()
 :rtype: float
%End

    void setMargin( double margin );
%Docstring
 Sets the ``margin`` between the edge of the frame and the label contents.
 This method sets both the horizontal and vertical margins to the same
 value. The margins can be individually controlled using the setMarginX()
 and setMarginY() methods.

 Margins are set using the current layout units.

.. seealso:: setMarginX()
.. seealso:: setMarginY()
%End

    void setMarginX( double margin );
%Docstring
 Sets the horizontal ``margin`` between the edge of the frame and the label
 contents, in layout units.
.. seealso:: setMargin()
.. seealso:: setMarginY()
%End

    void setMarginY( double margin );
%Docstring
 Sets the vertical ``margin`` between the edge of the frame and the label
 contents, in layout units.
.. seealso:: setMargin()
.. seealso:: setMarginX()
%End

    void setFontColor( const QColor &color );
%Docstring
 Sets the label font ``color``.
.. seealso:: fontColor()
%End

    QColor fontColor() const;
%Docstring
 Returns the label font color.
.. seealso:: setFontColor()
 :rtype: QColor
%End

    virtual QRectF boundingRect() const;


    virtual void setFrameEnabled( bool drawFrame );


    virtual void setFrameStrokeWidth( const QgsLayoutMeasurement &strokeWidth );


  public slots:

    virtual void refresh();


  protected:
    virtual void draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = 0 );

    virtual bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const;

    virtual bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitemlabel.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
