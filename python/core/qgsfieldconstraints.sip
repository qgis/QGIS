/**
 * \class QgsFieldConstraints
 * \ingroup core
 * Stores information about constraints which may be present on a field.
 * \note added in QGIS 3.0
 */


class QgsFieldConstraints
{

%TypeHeaderCode
#include <qgsfieldconstraints.h>
%End

  public:

    /**
     * Constraints which may be present on a field.
     */
    enum Constraint
    {
      ConstraintNotNull, //!< Field may not be null
      ConstraintUnique, //!< Field must have a unique value
      ConstraintExpression, //!< Field has an expression constraint set. See constraintExpression().
    };
    typedef QFlags<QgsFieldConstraints::Constraint> Constraints;

    /**
     * Origin of constraints.
     */
    enum ConstraintOrigin
    {
      ConstraintOriginNotSet, //!< Constraint is not set
      ConstraintOriginProvider, //!< Constraint was set at data provider
      ConstraintOriginLayer, //!< Constraint was set by layer
    };

    /**
     * Strength of constraints.
     */
    enum ConstraintStrength
    {
      ConstraintHard, //!< Constraint must be honored before feature can be accepted
      ConstraintSoft, //!< User is warned if constraint is violated but feature can still be accepted
    };

    /**
     * Constructor for QgsFieldConstraints.
     */
    QgsFieldConstraints();

    /**
     * Returns any constraints which are present for the field.
     * @see setConstraints()
     * @see constraintOrigin()
     */
    Constraints constraints() const;

    /**
     * Returns the origin of a field constraint, or ConstraintOriginNotSet if the constraint
     * is not present on this field.
     * @see constraints()
     */
    ConstraintOrigin constraintOrigin( Constraint constraint ) const;

    /**
     * Sets a constraint on the field.
     * @see constraints()
     * @see removeConstraint()
     */
    void setConstraint( Constraint constraint, ConstraintOrigin origin = ConstraintOriginLayer );

    /**
     * Removes a constraint from the field.
     * @see setConstraint()
     * @see constraints()
     */
    void removeConstraint( Constraint constraint );

    /**
     * Returns the constraint expression for the field, if set.
     * @see constraints()
     * @see constraintDescription()
     * @see setConstraintExpression()
     */
    QString constraintExpression() const;

    /**
     * Returns the descriptive name for the constraint expression.
     * @see constraints()
     * @see constraintExpression()
     * @see setConstraintExpression()
     */
    QString constraintDescription() const;

    /**
     * Set the constraint expression for the field. An optional descriptive name for the constraint
     * can also be set. Setting an empty expression will clear any existing expression constraint.
     * @see constraintExpression()
     * @see constraintDescription()
     * @see constraints()
     */
    void setConstraintExpression( const QString& expression, const QString& description = QString() );

    bool operator==( const QgsFieldConstraints& other ) const;

};

QFlags<QgsFieldConstraints::Constraint> operator|(QgsFieldConstraints::Constraint f1, QFlags<QgsFieldConstraints::Constraint> f2);
