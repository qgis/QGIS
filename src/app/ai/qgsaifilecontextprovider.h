#ifndef QGSAIFILECONTEXTPROVIDER_H
#define QGSAIFILECONTEXTPROVIDER_H

#include "qgis_app.h"

#include <QObject>
#include <QString>
#include <QStringList>

struct APP_EXPORT QgsAiFileContext
{
  QString filePath;
  QString selectedText;
  QString fileSnippet;
  bool truncated = false;

  bool isValid() const { return !filePath.isEmpty() || !selectedText.isEmpty(); }
};

class APP_EXPORT QgsAiFileContextProvider : public QObject
{
    Q_OBJECT

  public:
    explicit QgsAiFileContextProvider( const QString &workspaceRoot, QObject *parent = nullptr );

    QgsAiFileContext buildContext( const QString &filePath, const QString &selectedText = QString(), int maxBytes = 16384 ) const;
    QStringList searchInFile( const QString &filePath, const QString &needle, int maxMatches = 25 ) const;
    QString diffPreview( const QString &beforeText, const QString &afterText ) const;
    QString workspaceRoot() const { return mWorkspaceRoot; }

  private:
    QString normalizePath( const QString &filePath ) const;
    QString mWorkspaceRoot;
};

#endif // QGSAIFILECONTEXTPROVIDER_H
