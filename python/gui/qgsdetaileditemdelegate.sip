/** \ingroup gui
 * A custom model/view delegate that can display an icon, heading
 * and detail sections.
 * @see also QgsDetailedItemData
 */
class QgsDetailedItemDelegate : QAbstractItemDelegate
{
%TypeHeaderCode
#include <qgsdetaileditemdelegate.h>
%End

  public:
    QgsDetailedItemDelegate( QObject * parent /TransferThis/ = 0 );
    ~QgsDetailedItemDelegate();
    /** Reimplement for parent class */
    void paint( QPainter * painter,
                const QStyleOptionViewItem & option,
                const QModelIndex & index ) const;
    /** Reimplement for parent class */
    QSize sizeHint( const QStyleOptionViewItem & option,
                    const QModelIndex & index ) const;

    void setVerticalSpacing( int value );

    int verticalSpacing() const;

    void setHorizontalSpacing( int value );

    int horizontalSpacing() const;
};
