//    Copyright (C) 2020-2022 Jakub Melka
//
//    This file is part of PDF4QT.
//
//    PDF4QT is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    with the written consent of the copyright owner, any later version.
//
//    PDF4QT is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public License
//    along with PDF4QT.  If not, see <https://www.gnu.org/licenses/>.

#include "pdfplugin.h"
#include "pdfdbgheap.h"

namespace pdf
{

PDFPlugin::PDFPlugin(QObject* parent) :
    QObject(parent),
    m_dataExchangeInterface(nullptr),
    m_widget(nullptr),
    m_cmsManager(nullptr),
    m_document(nullptr)
{

}

void PDFPlugin::setDataExchangeInterface(IPluginDataExchange* dataExchangeInterface)
{
    m_dataExchangeInterface = dataExchangeInterface;
}

void PDFPlugin::setWidget(PDFWidget* widget)
{
    m_widget = widget;
}

void PDFPlugin::setCMSManager(PDFCMSManager* manager)
{
    m_cmsManager = manager;
}

void PDFPlugin::setDocument(const PDFModifiedDocument& document)
{
    m_document = document;
}

std::vector<QAction*> PDFPlugin::getActions() const
{
    return std::vector<QAction*>();
}

PDFPluginInfo PDFPluginInfo::loadFromJson(const QJsonObject* json)
{
    PDFPluginInfo result;

    QJsonObject metadata = json->value("MetaData").toObject();
    result.name = metadata.value(QLatin1String("Name")).toString();
    result.author = metadata.value(QLatin1String("Author")).toString();
    result.version = metadata.value(QLatin1String("Version")).toString();
    result.license = metadata.value(QLatin1String("License")).toString();
    result.description = metadata.value(QLatin1String("Description")).toString();

    return result;
}

}   // namespace pdf
