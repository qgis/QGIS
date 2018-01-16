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
%End

    virtual int type() const;

    virtual QIcon icon() const;

    virtual QString displayName() const;


    void adjustSizeToText();
%Docstring
Resizes the item so that the label's text fits to the item. Keeps the top left point stationary.

.. seealso:: :py:func:`sizeForText`
%End

    QSizeF sizeForText() const;
%Docstring
Returns the required item size (in layout units) for the label's text to fill the item.

.. seealso:: :py:func:`adjustSizeToText`
%End

    QString text() const;
%Docstring
Returns the label's preset text.

.. seealso:: :py:func:`currentText`

.. seealso:: :py:func:`setText`
%End

    void setText( const QString &text );
%Docstring
Sets the label's preset ``text``.

.. seealso:: :py:func:`text`
%End

    QString currentText() const;
%Docstring
Returns the text as it appears on the label (with evaluated expressions
and other dynamic content).

.. seealso:: :py:func:`text`
%End

    Mode mode() const;
%Docstring
Returns the label's current mode.

.. seealso:: :py:func:`setMode`
%End

    void setMode( Mode mode );
%Docstring
Sets the label's current ``mode``, allowing the label
to switch between font based and HTML based rendering.

.. seealso:: :py:func:`mode`
%End

    QFont font() const;
%Docstring
Returns the label's current font.

.. seealso:: :py:func:`setFont`
%End

    void setFont( const QFont &font );
%Docstring
Sets the label's current ``font``.

.. seealso:: :py:func:`font`
%End

    Qt::AlignmentFlag vAlign() const;
%Docstring
Returns for the vertical alignment of the label.

.. seealso:: :py:func:`setVAlign`

.. seealso:: :py:func:`hAlign`
%End

    Qt::AlignmentFlag hAlign() const;
%Docstring
Returns the horizontal alignment of the label.

.. seealso:: :py:func:`vAlign`

.. seealso:: :py:func:`setHAlign`
%End

    void setHAlign( Qt::AlignmentFlag alignment );
%Docstring
Sets the horizontal ``alignment`` of the label.

.. seealso:: :py:func:`hAlign`

.. seealso:: :py:func:`setVAlign`
%End

    void setVAlign( Qt::AlignmentFlag alignment );
%Docstring
Sets for the vertical ``alignment`` of the label.

.. seealso:: :py:func:`vAlign`

.. seealso:: :py:func:`setHAlign`
%End

    double marginX() const;
%Docstring
Returns the horizontal margin between the edge of the frame and the label
contents, in layout units.

.. seealso:: :py:func:`setMargin`

.. seealso:: :py:func:`marginY`
%End

    double marginY() const;
%Docstring
Returns the vertical margin between the edge of the frame and the label
contents, in layout units.

.. seealso:: :py:func:`setMargin`

.. seealso:: :py:func:`marginX`
%End

    void setMargin( double margin );
%Docstring
Sets the ``margin`` between the edge of the frame and the label contents.
This method sets both the horizontal and vertical margins to the same
value. The margins can be individually controlled using the setMarginX()
and setMarginY() methods.

Margins are set using the current layout units.

.. seealso:: :py:func:`setMarginX`

.. seealso:: :py:func:`setMarginY`
%End

    void setMarginX( double margin );
%Docstring
Sets the horizontal ``margin`` between the edge of the frame and the label
contents, in layout units.

.. seealso:: :py:func:`setMargin`

.. seealso:: :py:func:`setMarginY`
%End

    void setMarginY( double margin );
%Docstring
Sets the vertical ``margin`` between the edge of the frame and the label
contents, in layout units.

.. seealso:: :py:func:`setMargin`

.. seealso:: :py:func:`setMarginX`
%End

    void setFontColor( const QColor &color );
%Docstring
Sets the label font ``color``.

.. seealso:: :py:func:`fontColor`
%End

    QColor fontColor() const;
%Docstring
Returns the label font color.

.. seealso:: :py:func:`setFontColor`
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
