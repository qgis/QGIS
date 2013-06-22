class QgsStyleV2ExportImportDialog : QDialog
{
%TypeHeaderCode
#include <qgsstylev2exportimportdialog.h>
%End
  public:
    enum Mode
    {
      Export,
      Import
    };

    // constructor
    // mode argument must be 0 for saving and 1 for loading
    QgsStyleV2ExportImportDialog( QgsStyleV2* style, QWidget *parent = NULL, Mode mode = Export );
    ~QgsStyleV2ExportImportDialog();

  public slots:
    void doExportImport();
    void selectAll();
    void clearSelection();

    void importTypeChanged( int );
    void browse();
};
