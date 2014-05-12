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

    QgsSublayersDialog( ProviderType providerType, QString name, QWidget* parent = 0, Qt::WindowFlags fl = 0 );
    ~QgsSublayersDialog();

    void populateLayerTable( QStringList theList, QString delim = ":" );
    // Returns list of selected layers, if there are more layers with the same name,
    // geometry type is appended separated by semicolon, example: <layer>:<geometryType>
    QStringList selectionNames();
    QList<int> selectionIndexes();

  public slots:
    void on_buttonBox_helpRequested();
    int exec();
};
