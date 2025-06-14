#ifndef QGSATTRIBUTESFORMTREEVIEWINDICATOR_H
#define QGSATTRIBUTESFORMTREEVIEWINDICATOR_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "qgis_gui.h"

#include <QIcon>
#include <QObject>


/**
 * \brief Indicator that can be used in an Attributes Form tree view to display icons next to field items.
 *
 * They add extra context to the field item, making it easier to get an overview of constraints and default values.
 *
 * \warning Not part of stable API and may change in future QGIS releases.
 * \ingroup gui
 * \since QGIS 3.44
 */
class GUI_EXPORT QgsAttributesFormTreeViewIndicator : public QObject
{
    Q_OBJECT
  public:
    //! Constructs an indicator, optionally transferring ownership to a parent QObject
    explicit QgsAttributesFormTreeViewIndicator( QObject *parent = nullptr );

    //! Indicator icon that will be displayed in the Attributes Form tree view
    QIcon icon() const { return mIcon; }
    //! Sets indicator icon that will be displayed in the Attributes Form tree view
    void setIcon( const QIcon &icon )
    {
      mIcon = icon;
      emit changed();
    }

    //! Returns tool tip text that will be shown when user hovers mouse over the indicator
    QString toolTip() const { return mToolTip; }
    //! Sets tool tip text
    void setToolTip( const QString &tip ) { mToolTip = tip; }

  signals:
    /**
     * Emitted when the indicator changes state (e.g. icon).
     */
    void changed();

  private:
    QIcon mIcon;
    QString mToolTip;
};

#endif // QGSATTRIBUTESFORMTREEVIEWINDICATOR_H
