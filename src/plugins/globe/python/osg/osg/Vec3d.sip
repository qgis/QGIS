/** General purpose double triple for use as vertices, vectors and normals.
  * Provides general math operations from addition through to cross products.
  * No support yet added for double * Vec3d - is it necessary?
  * Need to define a non-member non-friend operator*  etc.
  *    Vec3d * double is okay
*/
namespace osg
{
class Vec3d
{
%TypeHeaderCode
#include <osg/Vec3d>
%End

    public:

        /** Data type of vector components.*/
        typedef double value_type;

        /** Number of vector components. */
        enum { num_components = 3 };

        /** Constructor that sets all components of the vector to zero */
        Vec3d();

        Vec3d(const Vec3f& vec);

        operator Vec3f() const;

        Vec3d(value_type x,value_type y,value_type z);
        Vec3d(const Vec2d& v2,value_type zz);

        bool operator == (const osg::Vec3d& v) const;

        bool operator != (const osg::Vec3d& v) const;

        bool operator <  (const osg::Vec3d& v) const;

        void set( value_type x, value_type y, value_type z);

        void set( const osg::Vec3d& rhs);

        value_type operator [] (int i) const;

        value_type x() const;
        value_type y() const;
        value_type z() const;

        /** Returns true if all components have values that are not NaN. */
        bool valid() const;
        /** Returns true if at least one component has value NaN. */
        bool isNaN() const;

        /** Dot product. */
        value_type operator * (const osg::Vec3d& rhs) const;

        /** Cross product. */
        const osg::Vec3d operator ^ (const osg::Vec3d& rhs) const;

        /** Multiply by scalar. */
        const osg::Vec3d operator * (value_type rhs) const;

        /** Unary multiply by scalar. */
        osg::Vec3d& operator *= (value_type rhs);

        /** Divide by scalar. */
        const osg::Vec3d operator / (value_type rhs) const;

        /** Unary divide by scalar. */
        osg::Vec3d& operator /= (value_type rhs);

        /** Binary vector add. */
        const osg::Vec3d operator + (const osg::Vec3d& rhs) const;

        /** Unary vector add. Slightly more efficient because no temporary
          * intermediate object.
        */
        osg::Vec3d& operator += (const osg::Vec3d& rhs);

        /** Binary vector subtract. */
        const osg::Vec3d operator - (const osg::Vec3d& rhs) const;

        /** Unary vector subtract. */
        osg::Vec3d& operator -= (const osg::Vec3d& rhs);

        /** Negation operator. Returns the negative of the Vec3d. */
        const osg::Vec3d operator - () const;

        /** Length of the vector = sqrt( vec . vec ) */
        value_type length() const;

        /** Length squared of the vector = vec . vec */
        value_type length2() const;

        /** Normalize the vector so that it has length unity.
          * Returns the previous length of the vector.
          * If the vector is zero length, it is left unchanged and zero is returned.
        */
        value_type normalize();
};    // end of class Vec3d
};
