
/**A file dialog which lets the user select the preferred encoding type for a data provider*/
class QgsEncodingFileDialog : QFileDialog
{
%TypeHeaderCode
#include <qgsencodingfiledialog.h>
%End

 public:
    QgsEncodingFileDialog(QWidget * parent = 0,
        const QString & caption = QString(), const QString & directory = QString(),
        const QString & filter = QString(), const QString & encoding = QString());
    ~QgsEncodingFileDialog();
    /**Returns a string describing the chosen encoding*/
    QString encoding() const;
};

