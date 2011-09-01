
#ifndef QGSSYMBOLV2SELECTORDIALOG_H
#define QGSSYMBOLV2SELECTORDIALOG_H

#include <QDialog>

#include "ui_qgssymbolv2selectordialogbase.h"

class QgsStyleV2;
class QgsSymbolV2;
class QgsVectorLayer;

class QMenu;

class GUI_EXPORT QgsSymbolV2SelectorDialog : public QDialog, private Ui::QgsSymbolV2SelectorDialogBase
{
    Q_OBJECT

  public:
    QgsSymbolV2SelectorDialog( QgsSymbolV2* symbol, QgsStyleV2* style, const QgsVectorLayer* vl, QWidget* parent = NULL, bool embedded = false );
    //constructor for multi symbol editing
    QgsSymbolV2SelectorDialog( QList<QgsSymbolV2*> symbols, QgsStyleV2* style, const QgsVectorLayer* vl, QWidget* parent = NULL, bool embedded = false );

    //! return menu for "advanced" button - create it if doesn't exist and show the advanced button
    QMenu* advancedMenu();

  protected:
    void populateSymbolView();
    void updateSymbolPreview();
    void updateSymbolColor();
    void updateSymbolInfo();

    //! Reimplements dialog keyPress event so we can ignore it
    void keyPressEvent( QKeyEvent * event );

  private:
    void init( bool embedded );
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

    void openStyleManager();

  signals:
    void symbolModified();

  protected:
    QgsStyleV2* mStyle;
    QList<QgsSymbolV2*> mSymbols;
    QMenu* mAdvancedMenu;
    const QgsVectorLayer* mVectorLayer;
};

#endif
