class QgsSublayersDialog : QDialog
{
%TypeHeaderCode
#include <qgssublayersdialog.h>
%End
  public:
    enum ProviderType
    {
      Ogr,
      Gdal,
      Vsifile
    };

    QgsSublayersDialog( ProviderType providerType, QString name, QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~QgsSublayersDialog();

    void populateLayerTable( QStringList theList, QString delim = ":" );
    QStringList selectionNames();
    QList<int> selectionIndexes();

  public slots:
    void on_buttonBox_helpRequested();
    int exec();
};
