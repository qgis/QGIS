class Node
{
%TypeHeaderCode
#include <Node.h>
%End

  public:
    Node();
    Node( const Node &n );
    ~Node();
    // Node &operator=( const Node &n );
    /** Returns a pointer to the next element in the linked list*/
    Node *getNext() const;
    /** Returns a pointer to the QgsPoint object associated with the node*/
    QgsPoint *getPoint() const;
    /** Sets the pointer to the next node*/
    void setNext( Node *n );
    /** Sets a new pointer to an associated QgsPoint object*/
    void setPoint( QgsPoint *p );
};
