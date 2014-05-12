class QgsComposerTextTable: QgsComposerTable
{
%TypeHeaderCode
#include <qgscomposertexttable.h>
%End
  public:
    QgsComposerTextTable( QgsComposition* c );
    ~QgsComposerTextTable();

    /** return correct graphics item type. Added in v1.7 */
    virtual int type() const;
    
    /**Sets the text to use for the header row for the table
     * @param labels list of strings to use for each column's header row
     * @see headerLabels
    */
    void setHeaderLabels( const QStringList& labels );
    
    /**Adds a row to the table
     * @param row list of strings to use for each cell's value in the newly added row
     * @note If row is shorter than the number of columns in the table than blank cells
     * will be inserted at the end of the row. If row contains more strings then the number
     * of columns in the table then these extra strings will be ignored.
    */    
    void addRow( const QStringList& row );

    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

};
