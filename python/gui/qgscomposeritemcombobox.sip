/**
 * /class QgsComposerItemComboBox
 * /ingroup gui
 * /brief The QgsComposerItemComboBox class is a combo box which displays items of
 * a matching type from a composition.
 * /note added in 2.16
 */
class QgsComposerItemComboBox : QComboBox
{
%TypeHeaderCode
#include "qgscomposeritemcombobox.h"
%End

  public:
    /**
     * QgsComposerItemComboBox creates a combo box to display a list of items in a
     * composition. The items can optionally be filtered by type.
     * @param parent parent widget
     * @param composition composition to show items from. If not set, no items will be shown
     * until setComposition() is called
     */
    explicit QgsComposerItemComboBox( QWidget* parent /TransferThis/ = nullptr, QgsComposition* composition = nullptr );

    /** Sets the composition containing the items to list in the combo box.
     */
    void setComposition( QgsComposition* composition );

    /** Sets a filter for the item type to show in the combo box.
     * @param itemType type of items to show. Set to QgsComposerItem::ComposerItem to
     * show all items.
     * @see itemType()
     */
    void setItemType( QgsComposerItem::ItemType itemType );

    /** Returns the filter for the item types to show in the combo box.
     * @see setItemType()
     */
    QgsComposerItem::ItemType itemType() const;

    /** Sets a list of specific items to exclude from the combo box.
     * @param exceptList list of items to exclude
     * @see exceptedItemList()
     */
    void setExceptedItemList( const QList< QgsComposerItem* >& exceptList );

    /** Returns the list of specific items excluded from the combo box.
     * @see setExceptedItemList()
     */
    QList< QgsComposerItem* > exceptedItemList() const;

    /** Return the item currently shown at the specified index within the combo box.
     * @param index position of item to return
     * @see currentItem()
     */
    const QgsComposerItem* item( int index ) const;

    /** Returns the item currently selected in the combo box.
     */
    const QgsComposerItem* currentItem() const;

  public slots:
    /** Sets the currently selected item in the combo box.
     * @param item selected item
     */
    void setItem( const QgsComposerItem* item );

  signals:

    //! Emitted whenever the currently selected item changes
    void itemChanged( QgsComposerItem* item );

};
