class QgsCodeEditorPython: QgsCodeEditor
{
%TypeHeaderCode
#include <qgscodeeditorpython.h>
%End

  public:
    /**
     * Construct a new Python editor.
     *
     * @param parent The parent QWidget
     * @param filenames The list of apis files to load for the python lexer
     * @note added in 2.6
     */
    QgsCodeEditorPython( QWidget *parent /TransferThis/ = 0, const QList<QString> &filenames = QList<QString>() );
    ~QgsCodeEditorPython();

    /** Load APIs from one or more files
     * @param filenames The list of apis files to load for the python lexer
     */
    void loadAPIs( const QList<QString> &filenames );

    /** Load a script file
     * @param script The script file to load
     */
    bool loadScript( const QString &script );

};
