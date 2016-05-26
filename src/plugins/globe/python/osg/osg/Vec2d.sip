/** General purpose double pair, uses include representation of
  * texture coordinates.
  * No support yet added for double * Vec2d - is it necessary?
  * Need to define a non-member non-friend operator* etc.
  * BTW: Vec2d * double is okay
*/

class Vec2d
{
%TypeHeaderCode
#include <osg/Vec2d>
using namespace osg;
%End
    public:

        /** Data type of vector components.*/
        typedef double value_type;

        /** Number of vector components. */
        enum { num_components = 2 };

        /** Constructor that sets all components of the vector to zero */
        Vec2d();

        Vec2d(value_type x,value_type y);

        Vec2d(const Vec2f& vec);

        operator Vec2f() const;


        bool operator == (const Vec2d& v) const;

        bool operator != (const Vec2d& v) const;

        bool operator <  (const Vec2d& v) const;

        void set( value_type x, value_type y );

        value_type operator [] (int i) const;

        value_type x() const;
        value_type y() const;

        /** Returns true if all components have values that are not NaN. */
        bool valid() const;
        /** Returns true if at least one component has value NaN. */
        bool isNaN() const;

        /** Dot product. */
        value_type operator * (const Vec2d& rhs) const;

        /** Multiply by scalar. */
        const Vec2d operator * (value_type rhs) const;

        /** Unary multiply by scalar. */
        Vec2d& operator *= (value_type rhs);

        /** Divide by scalar. */
        const Vec2d operator / (value_type rhs) const;

        /** Unary divide by scalar. */
        Vec2d& operator /= (value_type rhs);

        /** Binary vector add. */
        const Vec2d operator + (const Vec2d& rhs) const;

        /** Unary vector add. Slightly more efficient because no temporary
          * intermediate object.
        */
        Vec2d& operator += (const Vec2d& rhs);

        /** Binary vector subtract. */
        const Vec2d operator - (const Vec2d& rhs) const;

        /** Unary vector subtract. */
        Vec2d& operator -= (const Vec2d& rhs);

        /** Negation operator. Returns the negative of the Vec2d. */
        const Vec2d operator - () const;

        /** Length of the vector = sqrt( vec . vec ) */
        value_type length() const;

        /** Length squared of the vector = vec . vec */
        value_type length2( ) const;

        /** Normalize the vector so that it has length unity.
          * Returns the previous length of the vector.
        */
        value_type normalize();

};    // end of class Vec2d


/** Multiply by vector components. */
Vec2d componentMultiply(const Vec2d& lhs, const Vec2d& rhs);

/** Divide rhs components by rhs vector components. */
Vec2d componentDivide(const Vec2d& lhs, const Vec2d& rhs);
