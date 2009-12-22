
class QgsSearchString
{
%TypeHeaderCode
#include "qgssearchstring.h"
%End

  public:
    //! constructor
    QgsSearchString();

    //! copy constructor - makes also copy of search tree
    QgsSearchString( const QgsSearchString& str );

    //! destructor - deletes node tree
    ~QgsSearchString();

    //! assignment operator takes care to copy search tree correctly
    // unable to wrap QgsSearchString& operator=( const QgsSearchString& str );

    /** sets search string and parses search tree
        on success returns true and sets member variables to the new values */
    bool setString( QString str );

    /** copies tree and makes search string for it
       on success returns true and sets member variables to the new values */
    bool setTree( QgsSearchTreeNode* tree );

    //! getter functions
    QgsSearchTreeNode* tree();
    QString string();

    //! returns parser error message - valid only after unsuccessfull parsing
    const QString& parserErrorMsg();

    //! returns true if no string is set
    bool isEmpty();

    //! clear search string
    void clear();

};
