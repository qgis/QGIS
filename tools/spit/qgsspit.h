
#include <qlistview.h>
#include <vector>
#include "qgsspitbase.h"
#include "qgsshapefile.h"

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
  void removeAllFiles();
  void useDefault();
  void changeEditAndRemove(int);

  private:
  int default_value;
  std::vector <QgsShapeFile> fileList;
};
