class QgsScrollArea : QScrollArea
{
%TypeHeaderCode
#include <qgsscrollarea.h>
%End

  public:
    explicit QgsScrollArea( QWidget *parent /TransferThis/ = 0 );
    void scrollOccurred();
    bool hasScrolled() const;

  protected:
    void wheelEvent( QWheelEvent *event );

};
