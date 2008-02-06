/** QgsQuickPrint is a convenience class to quickly print a 
    preformatted map to pdf.
    */
class QgsQuickPrint : QObject
{
%TypeHeaderCode
#include <qgsquickprint.h>
%End
public:      
  QgsQuickPrint();
  //! Destructor
  virtual ~QgsQuickPrint();
  static QString pageSizeToString(QPrinter::PageSize theSize);
  static QPrinter::PageSize stringToPageSize(QString theSize);

public slots:
  void printMap(); 
  void setTitle(QString theText);
  void setName(QString theText);
  void setCopyright(QString theText);
  void setNorthArrow(QString theFileName);
  void setLogo1(QString theFileName);
  void setLogo2(QString theFileName);
  void setOutputPdf(QString theFileName);
  void setMapCanvas(QgsMapCanvas * thepMapCanvas);
  void setPageSize(QPrinter::PageSize theSize);
};

