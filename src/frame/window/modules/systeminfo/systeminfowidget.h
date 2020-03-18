/*
 * Copyright (C) 2011 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     liuhong <liuhong_cm@deepin.com>
 *
 * Maintainer: liuhong <liuhong_cm@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "window/namespace.h"

#include <DListView>

#include <QWidget>
#include <QMetaMethod>

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QListView;
class QStandardItemModel;
class QModelIndex;
QT_END_NAMESPACE

namespace DCC_NAMESPACE {
namespace systeminfo {

class SystemInfoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SystemInfoWidget(QWidget *parent = nullptr);
    void setCurrentIndex(int index);

private:
    void initWidget();
    void initData();

Q_SIGNALS:
    void requestShowAboutNative();
    void requestShowVersionProtocol();
    void requestShowEndUserLicenseAgreement();
    void requestShowRestore();

private:
    struct ListMethod {
        QString icon;
        QString text;
        QMetaMethod method;
    };

private:
    QVBoxLayout *m_mainContentLayout;
    DTK_WIDGET_NAMESPACE::DListView *m_listView;
    QStandardItemModel *m_itemModel;
    QList<ListMethod> m_itemList;
};

}
}
