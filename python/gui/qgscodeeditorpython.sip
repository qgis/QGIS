class QgsCodeEditorPython: QgsCodeEditor
{
%TypeHeaderCode
#include <qgscodeeditorpython.h>
%End

  public:
    QgsCodeEditorPython( QWidget *parent /TransferThis/ = 0, const QList<QString> &filenames = QList<QString>() );
    ~QgsCodeEditorPython();

    void loadAPIs(const QList<QString> &filenames );

    bool loadScript( const QString &script );

};
