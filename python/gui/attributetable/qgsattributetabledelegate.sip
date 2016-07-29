class QgsAttributeTableDelegate : QItemDelegate
{
%TypeHeaderCode
#include <qgsattributetabledelegate.h>
%End
    QgsVectorLayer *layer( const QAbstractItemModel *model ) const;

  public:
    /** Constructor
     * @param parent parent object
     */
    QgsAttributeTableDelegate( QObject* parent /TransferThis/ = 0 );
    /** Used to create an editor for when the user tries to
     * change the contents of a cell */
    QWidget * createEditor(
      QWidget *parent,
      const QStyleOptionViewItem &option,
      const QModelIndex &index ) const;

    /** Overloads the paint method form the QItemDelegate bas class */
    void paint(
      QPainter * painter,
      const QStyleOptionViewItem & option,
      const QModelIndex & index ) const;

    /**
     * Sets data from editor back to model. Overloads default method
     * @param editor editor which was created by create editor function in this class
     * @param model model where data should be updated
     * @param index index of field which is to be modified
     */
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const;

    /**
     * Sets data from model into the editor. Overloads default method
     * @param editor editor which was created by create editor function in this class
     * @param index index of field which is to be retrieved
     */
    void setEditorData( QWidget *editor, const QModelIndex &index ) const;

    void setFeatureSelectionModel( QgsFeatureSelectionModel* featureSelectionModel );

  signals:
    /**
     * Is emitted when an action column item is painted.
     * The consumer of this signal can initialize the index widget.
     *
     * @note This signal is emitted repeatedly whenever the item is being painted.
     *       It is the consumers responsibility to check if initialization has already
     *       happened before.
     */
    void actionColumnItemPainted( const QModelIndex& index ) const;
};
