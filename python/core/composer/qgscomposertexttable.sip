
/** A text table item that reads text from string lists
 * @note added in QGIS 2.10
*/
class QgsComposerTextTableV2 : QgsComposerTableV2
{

%TypeHeaderCode
#include <qgscomposertexttable.h>
%End

  public:
    QgsComposerTextTableV2( QgsComposition* c /TransferThis/, bool createUndoCommands );
    ~QgsComposerTextTableV2();

    /** Adds a row to the table
     * @param row list of strings to use for each cell's value in the newly added row
     * @note If row is shorter than the number of columns in the table than blank cells
     * will be inserted at the end of the row. If row contains more strings then the number
     * of columns in the table then these extra strings will be ignored.
     * @note if adding many rows, @link setContents @endlink is much faster
     */
    void addRow( const QStringList& row );

    /** Sets the contents of the text table.
     * @param contents list of table rows
     * @see addRow
     */
    void setContents( const QList< QStringList >& contents );

    bool getTableContents( QgsComposerTableContents &contents );

    virtual void addFrame( QgsComposerFrame* frame, bool recalcFrameSizes = true );

};

