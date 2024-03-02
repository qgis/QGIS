/***************************************************************************
    qgsdigitizingguidemodel.h
    ----------------------
    begin                : December 2023
    copyright            : (C) Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDIGITIZINGGUIDEMODEL_H
#define QGSDIGITIZINGGUIDEMODEL_H

#include <QAbstractTableModel>
#include <QDateTime>

#include "qgis_core.h"
#include "qgsdigitizingguidelayer.h"

class QItemSelection;

/**
 * \ingroup core
 * \brief Model to hold digitizing guides data
 *
 * \see QgsDigitizingGuideLayer
 *
 * \since QGIS 3.36
 */
class CORE_EXPORT QgsDigitizingGuideModel : public QAbstractTableModel
{
    Q_OBJECT

  public:

    //! Constructor
    explicit QgsDigitizingGuideModel( QgsDigitizingGuideLayer *guideLayer );

    /**
     * Adds a point guide to the model
     * \arg point the point guide added
     * \arg title an optional title for the point
     * \arg details detail items displaying how the guide was constructed
     * \arg creation the creation date time, now by default
     */
    void addPointGuide( const QString &guideItemId,
                        const QString &title = QString(),
                        const QString &titleItemId = QString(),
                        QStringList details = QStringList(),
                        const QDateTime &creation = QDateTime::currentDateTime() );

    /**
     * Adds a line guide to the model
     * \arg curve the curve/line guide added
     * \arg title an optional title for the point
     * \arg details detail items displaying how the guide was constructed
     * \arg creation the creation date time, now by default
     */
    void addLineGuide( const QString &guideItemId,
                       const QString &title = QString(),
                       const QString &titleItemId = QString(),
                       QStringList details = QStringList(),
                       const QDateTime &creation = QDateTime::currentDateTime() );

    //! Clears the model
    void clear();

    //! Removes guides at the given ids
    bool removeGuides( const QModelIndexList &indexList );

    int rowCount( const QModelIndex &parent ) const override;
    int columnCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;

    //! Handles selection changes to highlight guides on the map
    void selectionChanged( const QItemSelection &selected, const QItemSelection &deselected );

  private:
    friend class QgsDigitizingGuideLayer;

    class GuideInfo
    {
      public:
        GuideInfo( const QString &guideId, const QDateTime &creation = QDateTime::currentDateTime() )
          : mGuideItemId( guideId )
          , mCreation( creation )
        {}

        QString mType;
        QString mGuideItemId;
        QString mTitle;
        QString mTitleItemId;
        bool mEnabled = true;
        QStringList mDetails;
        QDateTime mCreation;
    };

    QgsDigitizingGuideLayer *mGuideLayer = nullptr;
    QList<GuideInfo> mGuides;

};

#endif // QGSADVANCEDDIGITIZINGGUIDMODELE_H
