/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposerlabel.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsComposerLabel: QgsComposerItem
{
%Docstring
 A label that can be placed onto a map composition.
%End

%TypeHeaderCode
#include "qgscomposerlabel.h"
%End
  public:
    QgsComposerLabel( QgsComposition *composition /TransferThis/ );
    ~QgsComposerLabel();

    virtual int type() const;
%Docstring
Return correct graphics item type.
 :rtype: int
%End

    virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget );

%Docstring
 Reimplementation of QCanvasItem.paint
%End

    void adjustSizeToText();
%Docstring
Resizes the widget such that the text fits to the item. Keeps top left point
%End

    QString text();
%Docstring
 :rtype: str
%End
    void setText( const QString &text );

    int htmlState();
%Docstring
 :rtype: int
%End
    void setHtmlState( int state );

    QString displayText() const;
%Docstring
Returns the text as it appears on screen (with replaced data field)
 :rtype: str
%End

    QFont font() const;
%Docstring
 :rtype: QFont
%End
    void setFont( const QFont &f );

    Qt::AlignmentFlag vAlign() const;
%Docstring
 Accessor for the vertical alignment of the label
 :return: Qt.AlignmentFlag
 :rtype: Qt.AlignmentFlag
%End

    Qt::AlignmentFlag hAlign() const;
%Docstring
 Accessor for the horizontal alignment of the label
 :return: Qt.AlignmentFlag
 :rtype: Qt.AlignmentFlag
%End

    void setHAlign( Qt::AlignmentFlag a );
%Docstring
 Mutator for the horizontal alignment of the label
 \param a alignment
 :return: void
%End

    void setVAlign( Qt::AlignmentFlag a );
%Docstring
 Mutator for the vertical alignment of the label
 \param a alignment
 :return: void
%End

    double marginX() const;
%Docstring
 Returns the horizontal margin between the edge of the frame and the label
 contents.
 :return: horizontal margin in mm
.. versionadded:: 2.7
 :rtype: float
%End

    double marginY() const;
%Docstring
 Returns the vertical margin between the edge of the frame and the label
 contents.
 :return: vertical margin in mm
.. versionadded:: 2.7
 :rtype: float
%End

    void setMargin( const double m );
%Docstring
 Sets the margin between the edge of the frame and the label contents.
 This method sets both the horizontal and vertical margins to the same
 value. The margins can be individually controlled using the setMarginX
 and setMarginY methods.
 \param m margin in mm
.. seealso:: setMarginX
.. seealso:: setMarginY
%End

    void setMarginX( const double margin );
%Docstring
 Sets the horizontal margin between the edge of the frame and the label
 contents.
 \param margin horizontal margin in mm
.. seealso:: setMargin
.. seealso:: setMarginY
.. versionadded:: 2.7
%End

    void setMarginY( const double margin );
%Docstring
 Sets the vertical margin between the edge of the frame and the label
 contents.
 \param margin vertical margin in mm
.. seealso:: setMargin
.. seealso:: setMarginX
.. versionadded:: 2.7
%End

    void setFontColor( const QColor &c );
%Docstring
Sets text color
%End
    QColor fontColor() const;
%Docstring
Get font color
 :rtype: QColor
%End

    virtual bool writeXml( QDomElement &elem, QDomDocument &doc ) const;

%Docstring
 Stores state in Dom element
 \param elem is Dom element corresponding to 'Composer' tag
 \param doc document
 :rtype: bool
%End

    virtual bool readXml( const QDomElement &itemElem, const QDomDocument &doc );

%Docstring
 Sets state from Dom document
 \param itemElem is Dom element corresponding to 'ComposerLabel' tag
 \param doc document
 :rtype: bool
%End

    virtual QString displayName() const;

    virtual QRectF boundingRect() const;

%Docstring
 In case of negative margins, the bounding rect may be larger than the
 label's frame
 :rtype: QRectF
%End

    virtual void setFrameEnabled( const bool drawFrame );
%Docstring
 Reimplemented to call prepareGeometryChange after toggling frame
%End

    virtual void setFrameStrokeWidth( const double strokeWidth );
%Docstring
 Reimplemented to call prepareGeometryChange after changing stroke width
%End

  public slots:

    void refreshExpressionContext();


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposerlabel.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
