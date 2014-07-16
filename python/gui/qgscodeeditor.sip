class QgsCodeEditor: QsciScintilla
{
%TypeHeaderCode
#include <qgscodeeditor.h>
%End

  public:
    QgsCodeEditor( QWidget *parent /TransferThis/ = 0, QString title = "" , bool folding = false, bool margin = false );
    ~QgsCodeEditor();

    /** Set margin visible state
     *  @param margin Set margin in the editor
     */
    void setMarginVisible( bool margin );
    bool marginVisible();

    /** Set folding visible state
     *  @param folding Set folding in the editor
     */
    void setFoldingVisible( bool folding);
    bool foldingVisible();
};
