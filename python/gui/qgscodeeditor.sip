class QgsCodeEditor: QsciScintilla
{
%TypeHeaderCode
#include <qgscodeeditor.h>
%End

  public:
    QgsCodeEditor( QWidget *parent /TransferThis/ = 0, QString title = "" , bool folding = false, bool margin = false );
    ~QgsCodeEditor();

    bool enableMargin( bool margin );

    void enableFolding( bool folding);
};
