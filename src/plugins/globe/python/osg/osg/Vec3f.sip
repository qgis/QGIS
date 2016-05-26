/** General purpose float triple for use as vertices, vectors and normals.
  * Provides general math operations from addition through to cross products.
  * No support yet added for float * Vec3f - is it necessary?
  * Need to define a non-member non-friend operator*  etc.
  * Vec3f * float is okay
*/
class Vec3f
{
%TypeHeaderCode
#include <osg/Vec3f>
using namespace osg;
%End
    public:

        /** Data type of vector components.*/
        typedef float value_type;

        /** Number of vector components. */
        enum { num_components = 3 };

        /** Constructor that sets all components of the vector to zero */
        Vec3f();
        Vec3f(value_type x,value_type y,value_type z);
        Vec3f(const Vec2f& v2,value_type zz);

        bool operator == (const Vec3f& v) const;

        bool operator != (const Vec3f& v) const;

        bool operator <  (const Vec3f& v) const;

        void set( value_type x, value_type y, value_type z);

        void set( const Vec3f& rhs);

        value_type operator [] (int i);

        value_type x() const;
        value_type y() const;
        value_type z() const;

        /** Returns true if all components have values that are not NaN. */
        bool valid() const;
        /** Returns true if at least one component has value NaN. */
        bool isNaN() const;

        /** Dot product. */
        value_type operator * (const Vec3f& rhs) const;

        /** Cross product. */
        const Vec3f operator ^ (const Vec3f& rhs) const;

        /** Multiply by scalar. */
        const Vec3f operator * (value_type rhs) const;

        /** Unary multiply by scalar. */
        Vec3f& operator *= (value_type rhs);

        /** Divide by scalar. */
        const Vec3f operator / (value_type rhs) const;

        /** Unary divide by scalar. */
        Vec3f& operator /= (value_type rhs);

        /** Binary vector add. */
        const Vec3f operator + (const Vec3f& rhs) const;

        /** Unary vector add. Slightly more efficient because no temporary
          * intermediate object.
        */
        Vec3f& operator += (const Vec3f& rhs);

        /** Binary vector subtract. */
        const Vec3f operator - (const Vec3f& rhs) const;

        /** Unary vector subtract. */
        Vec3f& operator -= (const Vec3f& rhs);

        /** Negation operator. Returns the negative of the Vec3f. */
        const Vec3f operator - () const;

        /** Length of the vector = sqrt( vec . vec ) */
        value_type length() const;

        /** Length squared of the vector = vec . vec */
        value_type length2() const;

        /** Normalize the vector so that it has length unity.
          * Returns the previous length of the vector.
        */
        value_type normalize();

};    // end of class Vec3f
