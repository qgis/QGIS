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

    void setHeaderLabels( const QStringList& l );
    void addRow( const QStringList& row );

    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

  protected:
    //! @note not available in python bindings
    // bool getFeatureAttributes( QList< QMap<int, QVariant> >& attributes );
    QMap<int, QString> getHeaderLabels() const;
};
