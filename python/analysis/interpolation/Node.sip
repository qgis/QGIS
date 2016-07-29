class Node
{
%TypeHeaderCode
#include <Node.h>
%End

  public:
    Node();
    Node( const Node& n );
    ~Node();
    // Node& operator=( const Node& n );
    /** Returns a pointer to the next element in the linked list*/
    Node* getNext() const;
    /** Returns a pointer to the Point3D object associated with the node*/
    Point3D* getPoint() const;
    /** Sets the pointer to the next node*/
    void setNext( Node* n );
    /** Sets a new pointer to an associated Point3D object*/
    void setPoint( Point3D* p );
};
