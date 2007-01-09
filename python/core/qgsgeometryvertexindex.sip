
 /**
  *  A QgsVertexIndex identifies a particular vertex in a given OGC Geometry.
  *  The left hand int (counting from 0) refers to the vertex number in
  *  the "innermost" line-string or linear-ring.
  *  The next int is the n'th line-string or linear-ring in a multi-line-string
  *  or polygon, and so on to the multi-geometry level if applicable
  *
  * @author Brendan Morley
  */

class QgsGeometryVertexIndex
{
%TypeHeaderCode
#include <qgsgeometryvertexindex.h>
%End

  public:
  
    //! Constructor
    QgsGeometryVertexIndex();
    
    /** copy constructor will prompt a deep copy of the object */
    QgsGeometryVertexIndex( const QgsGeometryVertexIndex & rhs );
    
    //! Destructor
    ~QgsGeometryVertexIndex();
    
    /** Pushes an int onto the last (rightmost) element of the index */
    void push_back(int& i);
    
    /** Gets the last (rightmost) element of the index */
    int back();
    
    /** Gets the i'th element of the index.
        i=0 refers to the "innermost" line-string or linear-ring.
     */
    int get_at(int i) const;

    /** Resets the index */
    void clear();

    /** Increments the last (rightmost) element of the index */
    void increment_back();

    /** Decrements the last (rightmost) element of the index */
    void decrement_back();

    /** assign i to the last (rightmost) element of the index */
    void assign_back(int& i);

    /** Returns this index as a string - useful for "printf debugging" */
    QString toString();


}; // class QgsGeometryVertexIndex

