class QgsCodeEditor: QsciScintilla
{
%TypeHeaderCode
#include <qgscodeeditor.h>
%End

  public:
    QgsCodeEditor( QWidget *parent /TransferThis/ = 0, QString title = "" , bool folding = false, bool margin = false );
    ~QgsCodeEditor();

    /** Set the widget title
     * @param title widget title
    */
    void setTitle( const QString title );
    
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
    
    /** Insert text at cursor position, or replace any selected text if user has
     * made a selection.
     * @param theText The text to be inserted
     */
    void insertText( const QString theText );

  protected:

    bool isFixedPitch( const QFont& font );

    QFont getMonospaceFont();
};
