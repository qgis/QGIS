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
    void paint( QPainter * thePainter,
                const QStyleOptionViewItem & theOption,
                const QModelIndex & theIndex ) const;
    /** Reimplement for parent class */
    QSize sizeHint( const QStyleOptionViewItem & theOption,
                    const QModelIndex & theIndex ) const;

    void setVerticalSpacing( int theValue );

    int verticalSpacing() const;

    void setHorizontalSpacing( int theValue );

    int horizontalSpacing() const;
};
