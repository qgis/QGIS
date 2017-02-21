class QgsCurveEditorWidget : QWidget
{
%TypeHeaderCode
#include <qgscurveeditorwidget.h>
%End

  public:

    QgsCurveEditorWidget( QWidget* parent /TransferThis/ = 0, const QgsCurveTransform& curve = QgsCurveTransform() );

    QgsCurveTransform curve() const;

    void setCurve( const QgsCurveTransform& curve );

  signals:

    void changed();

  protected:

    virtual void keyPressEvent( QKeyEvent *event );

};
