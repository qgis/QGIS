/***************************************************************************
                     qgsfieldconstraints.h
                     ---------------------
               Date                 : November 2016
               Copyright            : (C) 2016 by Nyall Dawson
               email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFIELDCONSTRAINTS_H
#define QGSFIELDCONSTRAINTS_H

#include <QString>
#include <QHash>
#include <QObject>

/**
 * \class QgsFieldConstraints
 * \ingroup core
 * Stores information about constraints which may be present on a field.
 * \note added in QGIS 3.0
 */

class CORE_EXPORT QgsFieldConstraints
{
    Q_GADGET

    Q_PROPERTY( Constraints constraints READ constraints )

  public:

    /**
     * Constraints which may be present on a field.
     */
    enum Constraint
    {
      ConstraintNotNull = 1, //!< Field may not be null
      ConstraintUnique = 1 << 1, //!< Field must have a unique value
      ConstraintExpression = 1 << 2, //!< Field has an expression constraint set. See constraintExpression().
    };
    Q_DECLARE_FLAGS( Constraints, Constraint )

    /**
     * Origin of constraints.
     */
    enum ConstraintOrigin
    {
      ConstraintOriginNotSet = 0, //!< Constraint is not set
      ConstraintOriginProvider, //!< Constraint was set at data provider
      ConstraintOriginLayer, //!< Constraint was set by layer
    };

    /**
     * Strength of constraints.
     */
    enum ConstraintStrength
    {
      ConstraintHard = 0, //!< Constraint must be honored before feature can be accepted
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
    Constraints constraints() const { return mConstraints; }

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
    void removeConstraint( Constraint constraint ) { mConstraints &= ~constraint; }

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
    QString constraintDescription() const { return mExpressionConstraintDescription; }

    /**
     * Set the constraint expression for the field. An optional descriptive name for the constraint
     * can also be set. Setting an empty expression will clear any existing expression constraint.
     * @see constraintExpression()
     * @see constraintDescription()
     * @see constraints()
     */
    void setConstraintExpression( const QString& expression, const QString& description = QString() );

    bool operator==( const QgsFieldConstraints& other ) const;

  private:

    //! Constraints
    Constraints mConstraints;

    //! Origin of field constraints
    QHash< Constraint, ConstraintOrigin > mConstraintOrigins;

    //! Expression constraint
    QString mExpressionConstraint;

    //! Expression constraint descriptive name
    QString mExpressionConstraintDescription;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsFieldConstraints::Constraints )

#endif //QGSFIELDCONSTRAINTS_H
