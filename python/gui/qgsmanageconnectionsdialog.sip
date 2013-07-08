class QgsManageConnectionsDialog : QDialog
{
%TypeHeaderCode
#include <qgsmanageconnectionsdialog.h>
%End

  public:
    enum Mode
    {
      Export,
      Import
    };

    enum Type
    {
      WMS,
      PostGIS,
      WFS,
      MSSQL,
      WCS,
      Oracle,
    };

    // constructor
    // mode argument must be 0 for export and 1 for import
    // type argument must be 0 for WMS and 1 for PostGIS
    QgsManageConnectionsDialog( QWidget *parent = NULL, Mode mode = Export, Type type = WMS, QString fileName = "" );

  public slots:
    void doExportImport();
    void selectAll();
    void clearSelection();
};
