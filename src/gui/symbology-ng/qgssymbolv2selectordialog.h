
#ifndef QGSSYMBOLV2SELECTORDIALOG_H
#define QGSSYMBOLV2SELECTORDIALOG_H

#include <QDialog>

#include "ui_qgssymbolv2selectordialogbase.h"

class QgsStyleV2;
class QgsSymbolV2;

class GUI_EXPORT QgsSymbolV2SelectorDialog : public QDialog, private Ui::QgsSymbolV2SelectorDialogBase
{
    Q_OBJECT

  public:
    QgsSymbolV2SelectorDialog( QgsSymbolV2* symbol, QgsStyleV2* style, QWidget* parent = NULL, bool embedded = false );

  protected:
    void populateSymbolView();
    void updateSymbolPreview();
    void updateSymbolColor();
    void updateSymbolInfo();

    //! Reimplements dialog keyPress event so we can ignore it
    void keyPressEvent( QKeyEvent * event );

  private:
    /**Displays alpha value as transparency in mTransparencyLabel*/
    void displayTransparency( double alpha );

  public slots:
    void changeSymbolProperties();
    void setSymbolFromStyle( const QModelIndex & index );
    void setSymbolColor();
    void setMarkerAngle( double angle );
    void setMarkerSize( double size );
    void setLineWidth( double width );
    void addSymbolToStyle();
    void on_mSymbolUnitComboBox_currentIndexChanged( const QString & text );
    void on_mTransparencySlider_valueChanged( int value );

  signals:
    void symbolModified();

  protected:
    QgsStyleV2* mStyle;
    QgsSymbolV2* mSymbol;
};

#endif
