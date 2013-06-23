/** \ingroup MapComposer
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

    /** return correct graphics item type. Added in v1.7 */
    virtual int type() const;

    /** \brief Reimplementation of QCanvasItem::paint*/
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    /**resizes the widget such that the text fits to the item. Keeps top left point*/
    void adjustSizeToText();

    QString text();
    void setText( const QString& text );

    int htmlState();
    void setHtmlState( int state );

    /**Returns the text as it appears on screen (with replaced data field)
      @note this function was added in version 1.2*/
    QString displayText() const;

    /** Sets the current feature, the current layer and a list of local variable substitutions for evaluating expressions */
    void setExpressionContext( QgsFeature* feature, QgsVectorLayer* layer, QMap<QString, QVariant> substitutions = QMap<QString, QVariant>() );

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
    //!brief Accessor for the margin of the label
    double margin();
    //!brief Mutator for the margin of the label
    void setMargin( double m );

    /**Sets text color
        @note: this function was added in version 1.4*/
    void setFontColor( const QColor& c );
    /**Get font color
        @note: this function was added in version 1.4*/
    QColor fontColor() const;

    void setSceneRect( const QRectF& rectangle );

    /** stores state in Dom element
       * @param elem is Dom element corresponding to 'Composer' tag
       * @param doc document
       */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** sets state from Dom document
       * @param itemElem is Dom element corresponding to 'ComposerLabel' tag
       * @param doc document
       */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

  public slots:
    virtual void setRotation( double r );
};
