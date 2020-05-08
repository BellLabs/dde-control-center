/*
 * Copyright (C) 2011 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     yangyuyin <yangyuyin_cm@deepin.com>
 *
 * Maintainer: yangyuyin <yangyuyin@deepin.com>
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
#include "developermodedialog.h"
#include "commoninfomodel.h"
#include "modules/accounts/user.h"
#include "widgets/titlelabel.h"

#include <com_deepin_deepinid.h>

#include <DTipLabel>
#include <DTitlebar>
#include <DWindowCloseButton>
#include <DTextBrowser>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTimer>
#include <QDBusInterface>
#include <QDebug>
#include <QDBusReply>
#include <QFileDialog>
#include <QJsonArray>

using GrubDevelopMode = com::deepin::deepinid;
using namespace DCC_NAMESPACE::commoninfo;

DWIDGET_USE_NAMESPACE

DeveloperModeDialog::DeveloperModeDialog(DAbstractDialog *parent)
    : DAbstractDialog (parent)
{
    setMinimumSize(QSize(350, 380));
    //总布局
    QVBoxLayout *vBoxLayout = new QVBoxLayout(this);
    //图标和关闭按钮布局
    QHBoxLayout *titleHBoxLayout = new QHBoxLayout();
    DTitlebar *titleIcon = new DTitlebar();
    titleIcon->setFrameStyle(QFrame::NoFrame);//无边框
    titleIcon->setBackgroundTransparent(true);//透明
    titleIcon->setIcon(QIcon::fromTheme("preferences-system"));
    titleHBoxLayout->addWidget(titleIcon, Qt::AlignTop);
    titleIcon->setMenuVisible(false);
    titleIcon->setTitle("");
    //内容布局
    QVBoxLayout *contentVBoxLayout = new QVBoxLayout();
    auto chooseModeTip = new TitleLabel (tr("Request Root Access"), this);
    chooseModeTip->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    chooseModeTip->setWordWrap(true);

    contentVBoxLayout->setContentsMargins(40, 0, 40, 20);
    contentVBoxLayout->addWidget(chooseModeTip, 0, Qt::AlignTop);

    auto hw = new QWidget();
    QHBoxLayout *hBoxLayout = new QHBoxLayout();
    m_onlineBtn = new DRadioButton(tr("Online"));
    m_offlineBtn = new DRadioButton(tr("Offline"));
    m_onlineBtn->setChecked(true);
    hw->setLayout(hBoxLayout);

    hBoxLayout->setSpacing(20);
    hBoxLayout->addWidget(m_onlineBtn, 0, Qt::AlignCenter);
    hBoxLayout->addWidget(m_offlineBtn, 0, Qt::AlignCenter);
    contentVBoxLayout->addWidget(hw, 0, Qt::AlignTop | Qt::AlignHCenter);

    //在线激活模式提示
    auto chooseModeCommonts = new DTextBrowser();
    auto mpalette=this->palette();
    mpalette.setBrush(QPalette::Base, QBrush(Qt::NoBrush));
    chooseModeCommonts->setPalette(mpalette);
    chooseModeCommonts->setFrameStyle(QFrame::NoFrame);
    chooseModeCommonts->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    chooseModeCommonts->setText(tr("Please sign in to your cloud account first and continue"));

    contentVBoxLayout->addWidget(chooseModeCommonts, 1, Qt::AlignTop | Qt::AlignHCenter);

    //在线激活模式下一步按钮
    m_nextButton  = new DSuggestButton(tr("Next"));
    contentVBoxLayout->addWidget(m_nextButton, 0, Qt::AlignBottom);

    //离线激活模式导出机器信息和导入证书按钮
    auto exportBtn = new QPushButton(tr("Export PC Info"));
    auto importBtn = new DSuggestButton(tr("Import Certificate"));
    exportBtn->setVisible(false);
    importBtn->setVisible(false);

    contentVBoxLayout->addWidget(exportBtn, 0, Qt::AlignBottom);
    contentVBoxLayout->addWidget(importBtn, 0, Qt::AlignBottom);

    //加入布局
    vBoxLayout->addLayout(titleHBoxLayout);
    vBoxLayout->addLayout(contentVBoxLayout);
    vBoxLayout->setMargin(0);
    setLayout(vBoxLayout);

    //选择激活在线模式或离线模式
    connect(m_onlineBtn, &QAbstractButton::toggled, [=]{
        if(m_onlineBtn->isChecked()){
            exportBtn->setVisible(false);
            importBtn->setVisible(false);
            m_nextButton->setVisible(true);

            chooseModeCommonts->setText(tr("Please sign in to your cloud account first and continue"));
            this->update();
        }else{
            m_nextButton->setVisible(false);
            exportBtn->setVisible(true);
            importBtn->setVisible(true);
            chooseModeCommonts->setText(tr("1. Export your PC information") + '\n'
                                        + tr("2. Go to https://www.chinauos.com/developMode to download an offline certificate") + '\n'
                                        + tr("3. Import the certificate"));
        }
    });

    connect(m_nextButton, &QPushButton::clicked, this, &DeveloperModeDialog::setLogin);
    connect(this,&DeveloperModeDialog::requestSetNextBtnStatus, [this]{close();});

    connect(exportBtn, &QPushButton::clicked, [this]{
        auto inter = new GrubDevelopMode("com.deepin.deepinid", "/com/deepin/deepinid",
                                         QDBusConnection::sessionBus(), this);
        auto hardwareInfo = inter->GetHardware();

        // 以读写方式打开主目录下的1.json文件，若该文件不存在则会自动创建
        QString defaultPath = QDir::homePath() + "/Desktop/1.json";
        auto path = QFileDialog::getSaveFileName(this, "", defaultPath);
        QFile file(path);
        if(!file.open(QIODevice::ReadWrite)) {
            qDebug() << "File open error";
        }else{
            qDebug() <<"File open!";
        }

        // 使用QJsonObject对象插入键值对。
        QJsonObject jsonObject;
        auto hardwareInfoValue = hardwareInfo.value();
        auto hardwareDMIValue = hardwareInfo.value().dmi;
        jsonObject.insert("id", hardwareInfoValue.id);
        jsonObject.insert("hostname", hardwareInfoValue.hostName);
        jsonObject.insert("username", hardwareInfoValue.username);
        jsonObject.insert("cpu", hardwareInfoValue.cpu);
        jsonObject.insert("laptop", hardwareInfoValue.laptop);
        jsonObject.insert("memory", hardwareInfoValue.memory);
        jsonObject.insert("network_cards", hardwareInfoValue.networkCards);

        QJsonObject objectDMI;
        objectDMI.insert("bios_vendor", hardwareDMIValue.biosVendor);
        objectDMI.insert("bios_version", hardwareDMIValue.biosVersion);
        objectDMI.insert("bios_date", hardwareDMIValue.biosDate);
        objectDMI.insert("board_name", hardwareDMIValue.boardName);
        objectDMI.insert("board_serial", hardwareDMIValue.boardSerial);
        objectDMI.insert("board_vendor", hardwareDMIValue.boardVendor);
        objectDMI.insert("board_version", hardwareDMIValue.boardVersion);
        objectDMI.insert("product_name", hardwareDMIValue.productName);
        objectDMI.insert("product_family", hardwareDMIValue.productFamily);
        objectDMI.insert("product_serial", hardwareDMIValue.producctSerial);
        objectDMI.insert("product_uuid", hardwareDMIValue.productUUID);
        objectDMI.insert("product_version", hardwareDMIValue.productVersion);

        jsonObject.insert("dmi", objectDMI);
        //使用QJsonDocument设置该json对象
        QJsonDocument jsonDoc;
        jsonDoc.setObject(jsonObject);

        //将json以文本形式写入文件并关闭文件
        file.write(jsonDoc.toJson());
        file.close();

        inter->deleteLater();
    });

    //离线模式导入证书

    connect(importBtn, &QPushButton::clicked, [this]{
        QString defaultPath = QDir::homePath() + "/Downloads/";
        auto filePathName = QFileDialog::getOpenFileName(this, "", defaultPath);
        if(!filePathName.isEmpty()){
            Q_EMIT requestCommit(filePathName);
            close();
        }
    });
}

void DeveloperModeDialog::setModel(DCC_NAMESPACE::commoninfo::CommonInfoModel *model)
{
    m_model = model;
}

void DeveloperModeDialog::setLogin()
{
    auto model = m_model;
    auto btn = m_nextButton;
    Q_ASSERT(model);
    auto requestDev = [this,btn]{
        btn->clearFocus();
        btn->setEnabled(false);
        //防止出现弹窗时可以再次点击按钮
        QTimer::singleShot(100, this, [this]{
            Q_EMIT requestDeveloperMode(true);
        });
    };

    if (!model->isLogin()){
        m_enterDev = true;
        btn->clearFocus();
        Q_EMIT requestLogin();
        connect(model, &CommonInfoModel::isLoginChenged, this, [requestDev, this, btn](bool log){
            if (!log || !m_enterDev)
                return;
            requestDev();
            m_enterDev = false;
        });
    }else {
        requestDev();
    }
}
