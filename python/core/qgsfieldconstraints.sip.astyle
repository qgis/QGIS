/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfieldconstraints.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsFieldConstraints
{
%Docstring
 Stores information about constraints which may be present on a field.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsfieldconstraints.h"
%End
  public:

    enum Constraint
    {
      ConstraintNotNull,
      ConstraintUnique,
      ConstraintExpression,
    };
    typedef QFlags<QgsFieldConstraints::Constraint> Constraints;


    enum ConstraintOrigin
    {
      ConstraintOriginNotSet,
      ConstraintOriginProvider,
      ConstraintOriginLayer,
    };

    enum ConstraintStrength
    {
      ConstraintStrengthNotSet,
      ConstraintStrengthHard,
      ConstraintStrengthSoft,
    };

    QgsFieldConstraints();
%Docstring
 Constructor for QgsFieldConstraints.
%End

    Constraints constraints() const;
%Docstring
 Returns any constraints which are present for the field.
.. seealso:: setConstraints()
.. seealso:: constraintOrigin()
 :rtype: Constraints
%End

    ConstraintOrigin constraintOrigin( Constraint constraint ) const;
%Docstring
 Returns the origin of a field constraint, or ConstraintOriginNotSet if the constraint
 is not present on this field.
.. seealso:: constraints()
 :rtype: ConstraintOrigin
%End

    ConstraintStrength constraintStrength( Constraint constraint ) const;
%Docstring
 Returns the strength of a field constraint, or ConstraintStrengthNotSet if the constraint
 is not present on this field.
.. seealso:: constraints()
.. seealso:: setConstraintStrength()
 :rtype: ConstraintStrength
%End

    void setConstraintStrength( Constraint constraint, ConstraintStrength strength );
%Docstring
 Sets the strength of a constraint. Note that the strength of constraints which originate
 from a provider cannot be changed. Constraints default to ConstraintStrengthHard unless
 explicitly changed.
.. seealso:: constraintStrength()
%End

    void setConstraint( Constraint constraint, ConstraintOrigin origin = ConstraintOriginLayer );
%Docstring
 Sets a constraint on the field.
.. seealso:: constraints()
.. seealso:: removeConstraint()
%End

    void removeConstraint( Constraint constraint );
%Docstring
 Removes a constraint from the field.
.. seealso:: setConstraint()
.. seealso:: constraints()
%End

    QString constraintExpression() const;
%Docstring
 Returns the constraint expression for the field, if set.
.. seealso:: constraints()
.. seealso:: constraintDescription()
.. seealso:: setConstraintExpression()
 :rtype: str
%End

    QString constraintDescription() const;
%Docstring
 Returns the descriptive name for the constraint expression.
.. seealso:: constraints()
.. seealso:: constraintExpression()
.. seealso:: setConstraintExpression()
 :rtype: str
%End

    void setConstraintExpression( const QString &expression, const QString &description = QString() );
%Docstring
 Set the constraint expression for the field. An optional descriptive name for the constraint
 can also be set. Setting an empty expression will clear any existing expression constraint.
.. seealso:: constraintExpression()
.. seealso:: constraintDescription()
.. seealso:: constraints()
%End

    bool operator==( const QgsFieldConstraints &other ) const;

};

QFlags<QgsFieldConstraints::Constraint> operator|(QgsFieldConstraints::Constraint f1, QFlags<QgsFieldConstraints::Constraint> f2);


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfieldconstraints.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
