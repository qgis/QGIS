
/**A file dialog which lets the user select the preferred encoding type for a data provider*/
class QgsEncodingFileDialog : QFileDialog
{
%TypeHeaderCode
#include <qgsencodingfiledialog.h>
%End

  public:
    QgsEncodingFileDialog( QWidget * parent /TransferThis/ = 0,
                           const QString & caption = QString(), const QString & directory = QString(),
                           const QString & filter = QString(), const QString & encoding = QString() );
    ~QgsEncodingFileDialog();
    /**Returns a string describing the chosen encoding*/
    QString encoding() const;
    /** Adds a 'Cancel All' button for the user to click */
    void addCancelAll();
    /** Returns true if the user clicked 'Cancel All' */
    bool cancelAll();

  public slots:
    void saveUsedEncoding();

    void pbnCancelAll_clicked();
};
