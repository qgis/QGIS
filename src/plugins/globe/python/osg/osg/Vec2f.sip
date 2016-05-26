/** General purpose float pair. Uses include representation of
  * texture coordinates.
  * No support yet added for float * Vec2f - is it necessary?
  * Need to define a non-member non-friend operator* etc.
  * BTW: Vec2f * float is okay
*/

class Vec2f
{
%TypeHeaderCode
#include <osg/Vec2f>
using namespace osg;
%End
    public:

        /** Data type of vector components.*/
        typedef float value_type;

        /** Number of vector components. */
        enum { num_components = 2 };

        /** Constructor that sets all components of the vector to zero */
        Vec2f();
        Vec2f(value_type x,value_type y);


        bool operator == (const Vec2f& v);

        bool operator != (const Vec2f& v);

        bool operator <  (const Vec2f& v);

        void set( value_type x, value_type y );

        value_type operator [] (int i) const;

        value_type x();
        value_type y();

        /** Returns true if all components have values that are not NaN. */
        bool valid() const;
        /** Returns true if at least one component has value NaN. */
        bool isNaN() const;

        /** Dot product. */
        value_type operator * (const Vec2f& rhs) const;

        /** Multiply by scalar. */
        const Vec2f operator * (value_type rhs) const;

        /** Unary multiply by scalar. */
        Vec2f& operator *= (value_type rhs);

        /** Divide by scalar. */
        const Vec2f operator / (value_type rhs) const;

        /** Unary divide by scalar. */
        Vec2f& operator /= (value_type rhs);

        /** Binary vector add. */
        const Vec2f operator + (const Vec2f& rhs) const;

        /** Unary vector add. Slightly more efficient because no temporary
          * intermediate object.
        */
        Vec2f& operator += (const Vec2f& rhs);

        /** Binary vector subtract. */
        const Vec2f operator - (const Vec2f& rhs) const;

        /** Unary vector subtract. */
        Vec2f& operator -= (const Vec2f& rhs);

        /** Negation operator. Returns the negative of the Vec2f. */
        const Vec2f operator - () const;

        /** Length of the vector = sqrt( vec . vec ) */
        value_type length() const;

        /** Length squared of the vector = vec . vec */
        value_type length2( ) const;

        /** Normalize the vector so that it has length unity.
          * Returns the previous length of the vector.
        */
        value_type normalize();

};    // end of class Vec2f

/** Multiply by vector components. */
Vec2f componentMultiply(const Vec2f& lhs, const Vec2f& rhs);

/** Divide rhs components by rhs vector components. */
Vec2f componentDivide(const Vec2f& lhs, const Vec2f& rhs);
