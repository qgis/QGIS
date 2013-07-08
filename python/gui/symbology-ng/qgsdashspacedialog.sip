class QgsDashSpaceDialog : QDialog
{
%TypeHeaderCode
#include <qgsdashspacedialog.h>
%End
  public:
    QgsDashSpaceDialog( const QVector<qreal>& v, QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~QgsDashSpaceDialog();

    QVector<qreal> dashDotVector() const;
};
