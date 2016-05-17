class QgsNumericSortTreeWidgetItem : QTreeWidgetItem
{
%TypeHeaderCode
#include <qgsnumericsortlistviewitem.h>
%End
  public:
    /**
     * Constructor.
     */
    QgsNumericSortTreeWidgetItem( QTreeWidget * parent );
    QgsNumericSortTreeWidgetItem( QTreeWidgetItem * parent );

    //! Destructor
    virtual ~QgsNumericSortTreeWidgetItem();

    virtual bool operator<( const QTreeWidgetItem &other ) const;

};
