// qgsunaccentrules.h
#ifndef QGSUNACCENTRULES_H
#define QGSUNACCENTRULES_H

#include <QString>
#include <QHash>
#include <QVector>
#include <QPair>

/**
 * \class QgsUnaccentRules
 * \ingroup core
 * \since QGIS 3.38
 * \brief Utilities for removing accents/diacritics from strings.
 *
 * The rules are loaded from the packaged resource file
 * "resources/unaccent.rules" and applied to input text to produce
 * an unaccented form.
 */

class QgsUnaccentRules
{
  public:
    static QgsUnaccentRules &instance();
    QString transform( const QString &in );

  private:
    QgsUnaccentRules();
    void ensureLoaded();
    bool m_loaded = false;
    int m_maxLen = 0;
    QHash<int, QVector<QPair<QString, QString>>> m_rulesByLen;
};

#endif // QGSUNACCENTRULES_H
