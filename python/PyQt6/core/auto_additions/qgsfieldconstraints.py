# The following has been generated automatically from src/core/qgsfieldconstraints.h
QgsFieldConstraints.ConstraintNotNull = QgsFieldConstraints.Constraint.ConstraintNotNull
QgsFieldConstraints.ConstraintUnique = QgsFieldConstraints.Constraint.ConstraintUnique
QgsFieldConstraints.ConstraintExpression = QgsFieldConstraints.Constraint.ConstraintExpression
QgsFieldConstraints.Constraints = lambda flags=0: QgsFieldConstraints.Constraint(flags)
QgsFieldConstraints.ConstraintOriginNotSet = QgsFieldConstraints.ConstraintOrigin.ConstraintOriginNotSet
QgsFieldConstraints.ConstraintOriginProvider = QgsFieldConstraints.ConstraintOrigin.ConstraintOriginProvider
QgsFieldConstraints.ConstraintOriginLayer = QgsFieldConstraints.ConstraintOrigin.ConstraintOriginLayer
QgsFieldConstraints.ConstraintStrengthNotSet = QgsFieldConstraints.ConstraintStrength.ConstraintStrengthNotSet
QgsFieldConstraints.ConstraintStrengthHard = QgsFieldConstraints.ConstraintStrength.ConstraintStrengthHard
QgsFieldConstraints.ConstraintStrengthSoft = QgsFieldConstraints.ConstraintStrength.ConstraintStrengthSoft
from enum import Enum


def _force_int(v): return int(v.value) if isinstance(v, Enum) else v


QgsFieldConstraints.Constraint.__bool__ = lambda flag: bool(_force_int(flag))
QgsFieldConstraints.Constraint.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsFieldConstraints.Constraint.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsFieldConstraints.Constraint.__or__ = lambda flag1, flag2: QgsFieldConstraints.Constraint(_force_int(flag1) | _force_int(flag2))
