
#include <qlistview.h>
#include "qgsspitbase.h"

class QgsSpit :public QgsSpitBase{
  public:
  QgsSpit(QWidget *parent=0, const char *name=0);
  ~QgsSpit();
	void populateConnectionList();
  void dbConnect();
  QStringList selectedTables();
  QString connInfo();
  void newConnection();
  void editConnection();
	void removeConnection();
  void addFile();
  void removeFile();

  private:
  QCheckListItem *lvi;
};
