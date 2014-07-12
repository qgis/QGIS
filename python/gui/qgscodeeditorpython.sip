class QgsCodeEditorPython: QgsCodeEditor
{
%TypeHeaderCode
#include <qgscodeeditorpython.h>
%End

  public:
    QgsCodeEditorPython( QWidget *parent /TransferThis/ = 0, const QList<QString> &filenames = QList<QString>() );
    ~QgsCodeEditorPython();

    void setTitle( QString );

    void loadAPIs(const QList<QString> &filenames );

    void loadScript( const QString &script );

};
