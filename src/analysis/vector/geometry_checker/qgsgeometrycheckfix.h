// LICENSE HEADER TODO
#ifndef QGSGEOMETRYCHECKFIX_H
#define QGSGEOMETRYCHECKFIX_H

#include <QString>
#include "qgis_analysis.h"

/**
 * This class implements a fix for problems detected in geometry checks.
 *
 * \since QGIS 3.12
 */
class ANALYSIS_EXPORT QgsGeometryCheckFix
{
  public:
    QgsGeometryCheckFix( int id, const QString &name, const QString &description, bool isStable = true );

    int id() const;

    /**
     * If this fix is stable enough to be listed by default.
     */
    bool isStable() const;

    /**
     * A human readable and translated name for this fix.
     */
    QString name() const;

    /**
     * A human readable and translated description for this fix.
     */
    QString description() const;

  private:
    int mId = -1;
    bool mIsStable = false;
    QString mName;
    QString mDescription;
};

#endif // QGSGEOMETRYCHECKFIX_H
