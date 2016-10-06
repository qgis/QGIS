class QgsDashSpaceDialog : QDialog
{
%TypeHeaderCode
#include <qgsdashspacedialog.h>
%End
  public:
    QgsDashSpaceDialog( const QVector<qreal>& v, QWidget * parent /TransferThis/ = 0, const Qt::WindowFlags& f = 0 );
    ~QgsDashSpaceDialog();

    QVector<qreal> dashDotVector() const;
};
