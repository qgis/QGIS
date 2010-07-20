#ifndef QGSRENDERERV2WIDGET_H
#define QGSRENDERERV2WIDGET_H

#include <QWidget>

class QgsVectorLayer;
class QgsStyleV2;
class QgsSymbolV2;
class QgsFeatureRendererV2;
class QgsSymbolV2SelectorDialog;


/**
  Base class for renderer settings widgets

WORKFLOW:
- open renderer dialog with some RENDERER  (never null!)
- find out which widget to use
- instantiate it and set in stacked widget
- on any change of renderer type, create some default (dummy?) version and change the stacked widget
- when clicked ok/apply, get the renderer from active widget and clone it for the layer
*/
class GUI_EXPORT QgsRendererV2Widget : public QWidget
{
  public:
    QgsRendererV2Widget( QgsVectorLayer* layer, QgsStyleV2* style );

    virtual ~QgsRendererV2Widget() {}

    //! return pointer to the renderer (no transfer of ownership)
    virtual QgsFeatureRendererV2* renderer() = 0;

  protected:
    QgsVectorLayer* mLayer;
    QgsStyleV2* mStyle;
};


////////////

#include <QObject>

class QMenu;
class QgsField;

typedef QMap<int, QgsField> QgsFieldMap;

/**
Utility class for prividing GUI for data-defined rendering.
*/
class QgsRendererV2DataDefinedMenus : public QObject
{
    Q_OBJECT

  public:

    QgsRendererV2DataDefinedMenus( QMenu* menu, const QgsFieldMap& flds, QString rotationField, QString sizeScaleField );

    void populateMenu( QMenu* menu, const char* slot, QString fieldName );
    void updateMenu( QMenu* menu, QString fieldName );

  public slots:

    void rotationFieldSelected();
    void sizeScaleFieldSelected();

  signals:

    void rotationFieldChanged( QString fldName );
    void sizeScaleFieldChanged( QString fldName );

  protected:
    QMenu* mRotationMenu;
    QMenu* mSizeScaleMenu;
    const QgsFieldMap& mFlds;
};

#endif // QGSRENDERERV2WIDGET_H
