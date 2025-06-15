#ifndef QGSATTRIBUTESFORMTREEVIEWINDICATORPROVIDER_H
#define QGSATTRIBUTESFORMTREEVIEWINDICATORPROVIDER_H

#include "qgsattributesformview.h"
#include "qgsattributesformtreeviewindicator.h"

#include <QObject>

/**
 * The QgsAttributesFormTreeViewIndicatorProvider class provides an interface for
 * layer tree indicator providers.
 *
 * Subclasses must override:
 *
 * - iconName()
 * - tooltipText()
 * - acceptLayer() filter function to determine whether the indicator must be added for the layer
 *
 * Subclasses may override:
 *
 * - onIndicatorClicked() default implementation does nothing
 * - connectSignals() default implementation connects layers to dataSourceChanged()
 * - disconnectSignals() default implementation disconnects layers from dataSourceChanged()
 */
class QgsAttributesFormTreeViewIndicatorProvider : public QObject
{
    Q_OBJECT
  public:
    explicit QgsAttributesFormTreeViewIndicatorProvider( QgsAttributesFormBaseView *view );

  public slots:
    /**
     * Updates the state of a the indicator for the given \a item.
     */
    void updateItemIndicator( QgsAttributesFormItem *item );

  protected:
    // Subclasses MAY override:
    // //! Connect signals, default implementation connects layers to dataSourceChanged()
    // virtual void connectSignals( QgsMapLayer *layer );
    // //! Disconnect signals, default implementation disconnects layers from dataSourceChanged()
    // virtual void disconnectSignals( QgsMapLayer *layer );

  protected slots:

    // //! Action on indicator clicked, default implementation does nothing
    // virtual void onIndicatorClicked( const QModelIndex &index ) { Q_UNUSED( index ) }
    // End MAY overrides

    //! Connects to signals of layers newly added to the tree
    void onAddedChildren( QgsAttributesFormItem *item, int indexFrom, int indexTo );
    //! Disconnects from layers about to be removed from the tree
    //void onWillRemoveChildren( QgsAttributesFormItem *item, int indexFrom, int indexTo );
    //void onLayerLoaded();
    ////! Adds/removes indicator of a layer
    //void onLayerChanged();

  private:
    // Subclasses MUST override:
    //! Layer filter: layers that pass the test will get the indicator
    virtual bool acceptsItem( QgsAttributesFormItem *item ) = 0;
    //! Returns the icon name for the given \a layer, icon name is passed to QgsApplication::getThemeIcon()
    virtual QString iconName( QgsAttributesFormItem *item ) = 0;
    //! Returns the tooltip text for the given \a layer
    virtual QString tooltipText( QgsAttributesFormItem *item ) = 0;
    // End MUST overrides

    //! Indicator factory
    std::unique_ptr<QgsAttributesFormTreeViewIndicator> newIndicator( QgsAttributesFormItem *item );

  protected:
    QgsAttributesFormBaseView *mAttributesFormTreeView = nullptr;
    QSet<QgsAttributesFormTreeViewIndicator *> mIndicators;
};


class QgsFieldConstraintIndicatorProvider : public QgsAttributesFormTreeViewIndicatorProvider
{
    Q_OBJECT

  public:
    explicit QgsFieldConstraintIndicatorProvider( QgsAttributesFormBaseView *view );

  private:
    bool acceptsItem( QgsAttributesFormItem *item ) override;
    QString iconName( QgsAttributesFormItem *item ) override;
    QString tooltipText( QgsAttributesFormItem *item ) override;
};


class QgsFieldDefaultValueIndicatorProvider : public QgsAttributesFormTreeViewIndicatorProvider
{
    Q_OBJECT

  public:
    explicit QgsFieldDefaultValueIndicatorProvider( QgsAttributesFormBaseView *view );

  private:
    bool acceptsItem( QgsAttributesFormItem *item ) override;
    QString iconName( QgsAttributesFormItem *item ) override;
    QString tooltipText( QgsAttributesFormItem *item ) override;
};

#endif // QGSATTRIBUTESFORMTREEVIEWINDICATORPROVIDER_H
