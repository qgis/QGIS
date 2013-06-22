class QgsDataDefinedSymbolDialog : QDialog
{
%TypeHeaderCode
#include <qgsdatadefinedsymboldialog.h>
%End
  public:
    class DataDefinedSymbolEntry
    {
    public:
      DataDefinedSymbolEntry( const QString& p, const QString& t, const QString& v, const QString& h );
    };

    QgsDataDefinedSymbolDialog( const QList< QgsDataDefinedSymbolDialog::DataDefinedSymbolEntry >& entries, const QgsVectorLayer* vl, QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~QgsDataDefinedSymbolDialog();
    QMap< QString, QString > dataDefinedProperties() const;

    //common help texts
    static QString doubleHelpText();
    static QString colorHelpText();
    static QString offsetHelpText();
    static QString fileNameHelpText();
};
