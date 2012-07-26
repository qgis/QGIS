#ifndef QGSCOMPOSERHTMLWIDGET_H
#define QGSCOMPOSERHTMLWIDGET_H

#include "ui_qgscomposerhtmlwidgetbase.h"

class QgsComposerHtml;

class QgsComposerHtmlWidget: public QWidget, private Ui::QgsComposerHtmlWidgetBase
{
    Q_OBJECT
  public:
    QgsComposerHtmlWidget( QgsComposerHtml* html );
    ~QgsComposerHtmlWidget();

  private slots:
    void on_mUrlLineEdit_editingFinished();
    void on_mFileToolButton_clicked();
    void on_mResizeModeComboBox_currentIndexChanged( int index );

    /**Sets the GUI elements to the values of mHtmlItem*/
    void setGuiElementValues();

  private:
    QgsComposerHtmlWidget();
    void blockSignals( bool block );

    QgsComposerHtml* mHtml;
};

#endif // QGSCOMPOSERHTMLWIDGET_H
