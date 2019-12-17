#ifndef QGSGEOMETRYCHECKFIX_H
#define QGSGEOMETRYCHECKFIX_H

#include <QString>

class QgsGeometryCheckFix
{
  public:
    QgsGeometryCheckFix( int id, const QString &name, const QString &description, bool isStable = true );

    int id() const;

    bool isStable() const;

    QString name() const;

    QString description() const;

  private:
    int mId = -1;
    bool mIsStable = false;
    QString mName;
    QString mDescription;
};

#endif // QGSGEOMETRYCHECKFIX_H
