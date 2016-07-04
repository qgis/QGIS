/** \ingroup core
 * A label that can be placed onto a map composition.
 */
class QgsComposerLabel : QgsComposerItem
{
%TypeHeaderCode
#include "qgscomposerlabel.h"
%End
  public:
    QgsComposerLabel( QgsComposition *composition /TransferThis/);
    ~QgsComposerLabel();

    /** Return correct graphics item type. */
    virtual int type() const;

    /** \brief Reimplementation of QCanvasItem::paint*/
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    /** Resizes the widget such that the text fits to the item. Keeps top left point*/
    void adjustSizeToText();

    QString text();
    void setText( const QString& text );

    int htmlState();
    void setHtmlState( int state );

    /** Returns the text as it appears on screen (with replaced data field) */
    QString displayText() const;

    /** Sets the current feature, the current layer and a list of local variable substitutions for evaluating expressions.
      * @deprecated use atlas features and setSubstitutions() instead
      */
    void setExpressionContext( QgsFeature* feature, QgsVectorLayer* layer, const QMap<QString, QVariant>& substitutions = QMap<QString, QVariant>() ) /Deprecated/;

    /** Sets the list of local variable substitutions for evaluating expressions in label text.
     * @note added in QGIS 2.12
     */
    void setSubstitutions( const QMap<QString, QVariant>& substitutions = QMap<QString, QVariant>() );

    QFont font() const;
    void setFont( const QFont& f );
    /** Accessor for the vertical alignment of the label
     * @returns Qt::AlignmentFlag
     */
    Qt::AlignmentFlag vAlign() const;
    /** Accessor for the horizontal alignment of the label
     * @returns Qt::AlignmentFlag
     */
    Qt::AlignmentFlag hAlign() const;
    /** Mutator for the horizontal alignment of the label
     * @param a alignment
     * @returns void
     */
    void setHAlign( Qt::AlignmentFlag a );
    /** Mutator for the vertical alignment of the label
     * @param a alignment
     * @returns void
     */
    void setVAlign( Qt::AlignmentFlag a );

    /** Returns the margin between the edge of the frame and the label contents
     * @returns margin in mm
     * @deprecated use marginX and marginY instead
     */
    double margin() /Deprecated/;

    /** Returns the horizontal margin between the edge of the frame and the label
     * contents.
     * @returns horizontal margin in mm
     * @note added in QGIS 2.7
     */
    double marginX() const;

    /** Returns the vertical margin between the edge of the frame and the label
     * contents.
     * @returns vertical margin in mm
     * @note added in QGIS 2.7
     */
    double marginY() const;

    /** Sets the margin between the edge of the frame and the label contents.
     * This method sets both the horizontal and vertical margins to the same
     * value. The margins can be individually controlled using the setMarginX
     * and setMarginY methods.
     * @param m margin in mm
     * @see setMarginX
     * @see setMarginY
     */
    void setMargin( const double m );

    /** Sets the horizontal margin between the edge of the frame and the label
     * contents.
     * @param margin horizontal margin in mm
     * @see setMargin
     * @see setMarginY
     * @note added in QGIS 2.7
     */
    void setMarginX( const double margin );

    /** Sets the vertical margin between the edge of the frame and the label
     * contents.
     * @param margin vertical margin in mm
     * @see setMargin
     * @see setMarginX
     * @note added in QGIS 2.7
     */
    void setMarginY( const double margin );

    /** Sets text color */
    void setFontColor( const QColor& c );
    /** Get font color */
    QColor fontColor() const;

    /** Stores state in Dom element
     * @param elem is Dom element corresponding to 'Composer' tag
     * @param doc document
     */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** Sets state from Dom document
     * @param itemElem is Dom element corresponding to 'ComposerLabel' tag
     * @param doc document
     */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    //Overridden to contain part of label's text
    virtual QString displayName() const;

    /** In case of negative margins, the bounding rect may be larger than the
     * label's frame
     */
    QRectF boundingRect() const;

    /** Reimplemented to call prepareGeometryChange after toggling frame
     */
    virtual void setFrameEnabled( const bool drawFrame );

    /** Reimplemented to call prepareGeometryChange after changing outline width
     */
    virtual void setFrameOutlineWidth( const double outlineWidth );

  public slots:
    void refreshExpressionContext();
};
