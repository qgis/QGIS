#ifndef QGSHELPVIEWER_H
#define QGSHELPVIEWER_H
# include "qgshelpviewerbase.h"
class QString;
class sqlite3;
class QgsHelpViewer : public QgsHelpViewerBase
{
  Q_OBJECT
  public:
    QgsHelpViewer(const QString &contextId=QString::null, QWidget *parent=0, const char *name=0);
    ~QgsHelpViewer();
    void setContext(const QString &contextId);
    void loadContext(const QString &contextId);
public slots:
    void fileExit();
private:
  int connectDb(const QString &helpDbPath);
 sqlite3 *db;
};
#endif // QGSHELPVIEWER_H
