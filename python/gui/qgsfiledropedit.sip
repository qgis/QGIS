/** \ingroup gui
 * A line edit for capturing file names that can have files dropped onto
 * it via drag & drop.
 */
class QgsFileDropEdit: QLineEdit
{
%TypeHeaderCode
#include <qgsfiledropedit.h>
%End
  public:
    QgsFileDropEdit( QWidget *parent /TransferThis/ = 0 );
    virtual ~QgsFileDropEdit();

    bool isDirOnly() const;
    void setDirOnly( bool isDirOnly );

    bool isFileOnly() const;
    void setFileOnly( bool isFileOnly );

    const QString& suffixFilter() const;
    void setSuffixFilter( const QString& suffix );

  protected:

    virtual void dragEnterEvent( QDragEnterEvent *event );
    virtual void dragLeaveEvent( QDragLeaveEvent *event );
    virtual void dropEvent( QDropEvent *event );
    virtual void paintEvent( QPaintEvent *e );
};
