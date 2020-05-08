/*
 * Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     LiLinling <lilinling_cm@deepin.com>
 *
 * Maintainer: LiLinling <lilinling_cm@deepin.com>
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

#include "defappwidget.h"
#include "window/utils.h"
#include "widgets/multiselectlistview.h"

#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QTimer>
#include <QDebug>

using namespace DCC_NAMESPACE::defapp;

DWIDGET_USE_NAMESPACE

DefaultAppsWidget::DefaultAppsWidget(QWidget *parent)
    : QWidget(parent)
    , m_defAppCatView(new dcc::widgets::MultiSelectListView(this))
    , m_centralLayout(new QVBoxLayout(this))
{
    setObjectName("Defapp");
    m_defAppCatView->setAccessibleName("List_defaultappcatList");
    m_defAppCatView->setResizeMode(QListView::Adjust);
    m_defAppCatView->setMovement(QListView::Static);
    m_defAppCatView->setFrameShape(QFrame::NoFrame);
    m_defAppCatView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_defAppCatView->setViewportMargins(ScrollAreaMargins);

    QStringList titles;
    //~ contents_path /defapp/Webpage
    titles << tr("Webpage");
    //~ contents_path /defapp/Mail
    titles << tr("Mail");
    //~ contents_path /defapp/Text
    titles << tr("Text");
    //~ contents_path /defapp/Music
    titles << tr("Music");
    //~ contents_path /defapp/Video
    titles << tr("Video");
    //~ contents_path /defapp/Picture
    titles << tr("Picture");
    //~ contents_path /defapp/Terminal
    titles << tr("Terminal");

    QStringList icons;
    icons << "dcc_browser"
          << "dcc_mail"
          << "dcc_text"
          << "dcc_music"
          << "dcc_video"
          << "dcc_photo"
          << "dcc_terminal";

    //Initialize second page view and model
    QStandardItemModel *model = new QStandardItemModel(this);

    for (int i = 0; i < icons.size(); i++) {
        DStandardItem *item = new DStandardItem(QIcon::fromTheme(icons.at(i)), titles.at(i));

        item->setData(VListViewItemMargin, Dtk::MarginsRole);
        model->appendRow(item);
    }

    m_defAppCatView->setModel(model);
    //show default browser app
    m_defAppCatView->setCurrentIndex(model->indexFromItem(model->item(0)));

    m_centralLayout->setMargin(0);
    m_centralLayout->addWidget(m_defAppCatView);

    connect(m_defAppCatView, &QListView::clicked, this, &DefaultAppsWidget::onCategoryClicked);
    connect(m_defAppCatView, &DListView::activated, m_defAppCatView, &QListView::clicked);

    setAccessibleName(tr("Default Applications"));
}

void DefaultAppsWidget::setCurrentIndex(int row)
{
    if (row > m_defAppCatView->model()->rowCount())
        row = 0;

    m_defAppCatView->setCurrentIndex(qobject_cast<QStandardItemModel *>(m_defAppCatView->model())->index(row, 0));
}

void DefaultAppsWidget::onCategoryClicked(const QModelIndex &index) {
    qDebug() << "DefaultAppsWidget clicked row " << index.row();
    Q_EMIT requestCategoryClicked(dcc::defapp::DefAppWorker::DefaultAppsCategory(index.row()));
    m_defAppCatView->resetStatus(index);
}
