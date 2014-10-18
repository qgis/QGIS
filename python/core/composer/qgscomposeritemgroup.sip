class QgsComposerItemGroup: QgsComposerItem
{
%TypeHeaderCode
#include <qgscomposeritemgroup.h>
%End
  public:
    QgsComposerItemGroup( QgsComposition* c /TransferThis/ );
    ~QgsComposerItemGroup();

    /** return correct graphics item type. Added in v1.7 */
    virtual int type() const;

    /**Adds an item to the group. All the group members are deleted
     if the group is deleted*/
    void addItem( QgsComposerItem* item );
    /**Removes the items but does not delete them*/
    void removeItems();
    /**Draw outline and ev. selection handles*/
    void paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );
    /**Sets this items bound in scene coordinates such that 1 item size units
       corresponds to 1 scene size unit*/
    void setSceneRect( const QRectF& rectangle );

    //overridden to also hide grouped items
    virtual void setVisibility( const bool visible );

    /** stores state in Dom node
       * @param elem is Dom element corresponding to 'Composer' tag
       * @param doc is the Dom document
       */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** sets state from Dom document
       * @param itemElem is Dom node corresponding to item tag
       * @param doc is the Dom document
       */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    QSet<QgsComposerItem*> items();

  signals:
    void childItemDeleted( QgsComposerItem* item );

  public slots:
    void itemDestroyed();

  protected:
    void drawFrame( QPainter* p );
};
