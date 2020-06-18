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

#include "createaccountpage.h"
#include "widgets/titlelabel.h"
#include "window/utils.h"
#include "groupitem.h"

#include <DFontSizeManager>

#include <QtGlobal>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QList>
#include <QDebug>
#include <QSettings>
#include <QApplication>

DWIDGET_USE_NAMESPACE
using namespace dcc::accounts;
using namespace dcc::widgets;
using namespace DCC_NAMESPACE::accounts;

CreateAccountPage::CreateAccountPage(QWidget *parent)
    : QWidget(parent)
    , m_newUser{nullptr}
    , m_avatarListWidget(nullptr)
    , m_nameEdit(new DLineEdit)
    , m_fullnameEdit(new DLineEdit)
    , m_passwdEdit(new DPasswordEdit)
    , m_repeatpasswdEdit(new DPasswordEdit)
    , m_groupListView(nullptr)
    , m_groupItemModel(nullptr)
{
    m_isServerSystem = IsServerSystem;
    QVBoxLayout *mainContentLayout = new QVBoxLayout;
    mainContentLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    mainContentLayout->setMargin(0);
    setLayout(mainContentLayout);

    m_scrollArea = new QScrollArea;
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameStyle(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setContentsMargins(0, 0, 0, 0);
    mainContentLayout->addWidget(m_scrollArea);

    auto contentLayout = new QVBoxLayout();
    contentLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    m_tw = new QWidget();
    m_tw->setLayout(contentLayout);
    contentLayout->setSpacing(0);
    contentLayout->setMargin(0);
    m_scrollArea->setWidget(m_tw);

    initWidgets(contentLayout);
    if (m_isServerSystem) {
        contentLayout->addSpacing(List_Interval);
        initUsrGroup(contentLayout);
    }

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setMargin(0);

    QPushButton *cancleBtn = new QPushButton(tr("Cancel"));
    DSuggestButton *addBtn = new DSuggestButton(tr("Create"));
    btnLayout->addWidget(cancleBtn);
    btnLayout->addWidget(addBtn);
    mainContentLayout->addSpacing(0);
    mainContentLayout->addLayout(btnLayout);

    connect(cancleBtn, &QPushButton::clicked, this, [&] {
        Q_EMIT requestBack();
    });
    connect(addBtn, &DSuggestButton::clicked, this, &CreateAccountPage::createUser);

}

CreateAccountPage::~CreateAccountPage()
{
    m_repeatpasswdEdit->hideAlertMessage();
}

void CreateAccountPage::resizeEvent(QResizeEvent *e)
{
    if (m_tw) {
        m_tw->resize(m_scrollArea->size());
    }
}

void CreateAccountPage::initUsrGroup(QVBoxLayout *layout)
{
    m_groupListView = new DListView(this);
    m_groupItemModel = new QStandardItemModel(this);
    m_groupListView->setModel(m_groupItemModel);
    m_groupListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_groupListView->setBackgroundType(DStyledItemDelegate::BackgroundType::ClipCornerBackground);
    m_groupListView->setSelectionMode(QAbstractItemView::NoSelection);
    m_groupListView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    m_groupListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_groupListView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_groupListView->setSpacing(1);
    connect(m_groupListView, &DListView::clicked, this, [=](const QModelIndex &index) {
        QStandardItem *item = m_groupItemModel->item(index.row() ,index.column());
        Qt::CheckState state = item->checkState();
        if (state == Qt::Checked) {
            item->setCheckState(Qt::Unchecked);
        } else {
            item->setCheckState(Qt::Checked);
        }
        m_groupItemModel->sort(0);
    });
    QLabel *groupTip = new QLabel(tr("Group"));
    layout->addWidget(groupTip);
    layout->addSpacing(List_Interval);
    layout->addWidget(m_groupListView);
}

void CreateAccountPage::initWidgets(QVBoxLayout *layout)
{
    //~ contents_path /accounts/New Account
    TitleLabel *titleLabel = new TitleLabel(tr("New Account"));
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(0,0,10,0);
    layout->addSpacing(13);
    layout->addWidget(titleLabel);

    m_avatarListWidget = new AvatarListWidget(m_newUser, this);
    m_avatarListWidget->setAvatarSize(QSize(40, 40));
    m_avatarListWidget->setViewportMargins(0, 0, 0, 0);
    m_avatarListWidget->setSpacing(14);
    m_avatarListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    layout->addSpacing(7);
    layout->addWidget(m_avatarListWidget, 0, Qt::AlignTop);

    QLabel *nameLabel = new QLabel(tr("Username") + ':');
    layout->addWidget(nameLabel);
    layout->addWidget(m_nameEdit);
    layout->addSpacing(7);

    QLabel *fullnameLabel = new QLabel(tr("Full Name") + ':');
    layout->addWidget(fullnameLabel);
    layout->addWidget(m_fullnameEdit);
    layout->addSpacing(7);

    QLabel *passwdLabel = new QLabel(tr("Password") + ':');
    layout->addWidget(passwdLabel);
    layout->addWidget(m_passwdEdit);
    layout->addSpacing(7);

    QLabel *repeatpasswdLabel = new QLabel(tr("Repeat Password") + ':');
    layout->addWidget(repeatpasswdLabel);
    layout->addWidget(m_repeatpasswdEdit);
    layout->addSpacing(7);

    connect(m_avatarListWidget, &AvatarListWidget::requestSetAvatar,
            m_avatarListWidget, &AvatarListWidget::setCurrentAvatarChecked);
    connect(m_avatarListWidget, &AvatarListWidget::requestAddNewAvatar,
    this, [this](dcc::accounts::User *user, const QString &file) {
        Q_UNUSED(user)
        m_avatarListWidget->setCurrentAvatarChecked(file);
    });

    connect(m_nameEdit, &DLineEdit::textEdited, this, [ = ](const QString &str) {
        if (m_nameEdit->isAlert()) {
            m_nameEdit->hideAlertMessage();
            m_nameEdit->setAlert(false);
        }

        if (m_nameEdit->text().isEmpty())
            return;

        QString strText(m_nameEdit->text());
        QString strTemp;
        int idx;
        for (idx = 0; idx < strText.size(); ++idx) {
            if ((strText[idx] >= '0' && strText[idx] <= '9') ||
                (strText[idx] >= 'a' && strText[idx] <= 'z') ||
                (strText[idx] >= 'A' && strText[idx] <= 'Z') ||
                (strText[idx] == '-' || strText[idx] == '_')) {
                strTemp.append(strText[idx]);
            } else {
                break;
            }
        }

        m_nameEdit->lineEdit()->blockSignals(true);
        m_nameEdit->lineEdit()->setText(strTemp.toLower());
        m_nameEdit->lineEdit()->setCursorPosition(idx);
        m_nameEdit->lineEdit()->blockSignals(false);
    });

    connect(m_fullnameEdit, &DLineEdit::textEdited, this, [ = ] {
        if (m_fullnameEdit->isAlert()){
            m_fullnameEdit->hideAlertMessage();
            m_fullnameEdit->setAlert(false);
        }
    });

    connect(m_passwdEdit, &DPasswordEdit::textEdited, this, [ = ] {
        if (m_passwdEdit->isAlert()) {
            m_passwdEdit->hideAlertMessage();
            m_passwdEdit->setAlert(false);
        }
    });

    connect(m_repeatpasswdEdit, &DPasswordEdit::textEdited, this, [ = ] {
        if (m_repeatpasswdEdit->isAlert()) {
            m_repeatpasswdEdit->hideAlertMessage();
            m_repeatpasswdEdit->setAlert(false);
        }
    });

    m_nameEdit->lineEdit()->setPlaceholderText(tr("Required"));//必填
    m_fullnameEdit->lineEdit()->setPlaceholderText(tr("optional"));//选填
    m_passwdEdit->lineEdit()->setPlaceholderText(tr("Required"));//必填
    m_repeatpasswdEdit->lineEdit()->setPlaceholderText(tr("Required"));//必填
}

void CreateAccountPage::setModel(UserModel *userModel, User *user)
{
    m_newUser = user;
    Q_ASSERT(m_newUser);
    m_userModel = userModel;
    Q_ASSERT(m_userModel);

    if (!m_groupItemModel) {
        return;
    }
    m_groupItemModel->clear();
    for(QString item : m_userModel->getAllGroups()) {
        GroupItem *it = new GroupItem(item);
        it->setCheckable(false);
        m_groupItemModel->appendRow(it);
    }

    QStringList presetGroup = m_userModel->getPresetGroups();
    int row_count = m_groupItemModel->rowCount();
    for (int i = 0; i < row_count; ++i) {
        QStandardItem *item = m_groupItemModel->item(i, 0);
        if (item && presetGroup.contains(item->text())) {
            item->setCheckState(Qt::Checked);
        } else {
            item->setCheckState(Qt::Unchecked);
        }
    }
    m_groupItemModel->sort(0);
}

void CreateAccountPage::createUser()
{
    //校验输入的用户名和密码
    if (!onNameEditFinished(m_nameEdit)) {
        return;
    }
    if (!onPasswordEditFinished(m_passwdEdit)) {
        return;
    }
    if (!onFullNameEidtFinished(m_fullnameEdit)) {
        return;
    }
    if (!onPasswordEditFinished(m_repeatpasswdEdit)) {
        return;
    }

    //如果用户没有选图像
    auto avatarPaht = m_avatarListWidget->getAvatarPath();
    m_newUser->setCurrentAvatar(avatarPaht);
    m_newUser->setName(m_nameEdit->lineEdit()->text());
    m_newUser->setFullname(m_fullnameEdit->lineEdit()->text());
    m_newUser->setPassword(m_passwdEdit->lineEdit()->text());
    m_newUser->setRepeatPassword(m_repeatpasswdEdit->lineEdit()->text());

    if (m_isServerSystem) {
        QStringList usrGroups;
        int row_count = m_groupItemModel->rowCount();
        for (int i = 0; i < row_count; ++i) {
            QStandardItem *item = m_groupItemModel->item(i, 0);
            if (item->checkState() == Qt::Checked) {
                usrGroups << item->text();
            }
        }
        m_newUser->setGroups(usrGroups);
    }

    DaemonService *daemonservice = new DaemonService("com.deepin.defender.daemonservice",
                                                     "/com/deepin/defender/daemonservice",
                                                     QDBusConnection::sessionBus(), this);
    QString strPwd = m_passwdEdit->lineEdit()->text();
    if (strPwd.length() >= daemonservice->GetPwdLen() && m_newUser->charactertypes(strPwd) >= daemonservice->GetPwdTypeLen()) {
        Q_EMIT requestCreateUser(m_newUser);
    } else {
        DDialog dlg("", daemonservice->GetPwdError());
        dlg.setIcon(QIcon::fromTheme("preferences-system"));
        dlg.addButton(tr("Go to Settings"));
        dlg.addButton(tr("Cancel"), true, DDialog::ButtonWarning);
        connect(&dlg, &DDialog::buttonClicked, this, [this](int idx){
            if (idx == 0) {
                Defender *defender = new Defender("com.deepin.defender.hmiscreen",
                                                  "/com/deepin/defender/hmiscreen",
                                                  QDBusConnection::sessionBus(), this);
                defender->ShowModule("systemsafety");
            }
        });
        dlg.exec();
    }
}

bool CreateAccountPage::validatePassword(const QString &password)
{
    QString validate_policy = QString("1234567890") + QString("abcdefghijklmnopqrstuvwxyz") +
                              QString("ABCDEFGHIJKLMNOPQRSTUVWXYZ") + QString("~!@#$%^&*()[]{}\\|/?,.<>");

    return containsChar(password, validate_policy);
}

bool CreateAccountPage::containsChar(const QString &password, const QString &validate)
{
    for (const QChar &p : password) {
        if (!validate.contains(p)) {
            return false;
        }
    }

    return true;
}

void CreateAccountPage::setCreationResult(CreationResult *result)
{
    switch (result->type()) {
    case CreationResult::NoError:
        Q_EMIT requestBack(AccountsWidget::CreateUserSuccess);
        break;
    case CreationResult::UserNameError:
        m_nameEdit->setAlert(true);
        m_nameEdit->showAlertMessage(result->message(), -1);
        break;
    case CreationResult::PasswordError:
        m_passwdEdit->setAlert(true);
        m_passwdEdit->showAlertMessage(result->message(), -1);
        break;
    case CreationResult::PasswordMatchError:
        m_repeatpasswdEdit->setAlert(true);
        m_repeatpasswdEdit->showAlertMessage(result->message(), -1);
        break; // reserved for future server edition feature.
    case CreationResult::UnknownError:
        qWarning() << "error encountered creating user: " << result->message();
        break;
    }

    result->deleteLater();
}

bool CreateAccountPage::onPasswordEditFinished(DPasswordEdit *edit)
{
    if (edit == m_repeatpasswdEdit) {
        if (m_passwdEdit->lineEdit()->text() != m_repeatpasswdEdit->lineEdit()->text()) {
            m_repeatpasswdEdit->setAlert(true);
            m_repeatpasswdEdit->showAlertMessage(tr("Passwords do not match"), this->parentWidget(), -1);
            return false;
        }
        return true; //重复密码 只检验与第一个密码是否相同
    }

    const QString &userpassword = edit->lineEdit()->text();
    if (userpassword.isEmpty()) {
        edit->setAlert(true);
        return false;
    }

//    const int maxSize = 512;
//    if (userpassword.size() > maxSize) {
//        edit->setAlert(true);
//        edit->showAlertMessage(tr("Password must be no more than %1 characters").arg(maxSize), -1);
//        return false;
//    }

//    bool result = validatePassword(userpassword);
//    if (!result) {
//        edit->setAlert(true);
//        edit->showAlertMessage(tr("Password can only contain English letters (case-sensitive), numbers or special symbols (~!@#$%^&*()[]{}\\|/?,.<>)"), -1);
//        return false;
//    }
    auto res = dcc::accounts::UserModel::validatePassword(edit->lineEdit()->text());
    if (!res.isEmpty()) {
        edit->setAlert(true);
        edit->showAlertMessage(res);
        return false;
    }
    
    return true;
}

bool CreateAccountPage::validateUsername(const QString &username, QString &invalidReason)
{
    const int min_len = 3;
    const int max_len = 32;

    if (username.length() < min_len) {
        invalidReason = tr("Username must be between 3 and 32 characters");
        return false;
    }
    if (username.length() > max_len) {
        invalidReason = tr("Username must be between 3 and 32 characters");
        return false;
    }
    const uint first_char = username.at(0).unicode();
    if (first_char < 'a' || first_char > 'z') {
        invalidReason = tr("The first character must be a letter (lowercase)");
        return false;
    }
    const QRegExp reg("[a-z][a-z0-9_-]*");
    if (!reg.exactMatch(username)) {
        invalidReason = tr("Username can only contain English letters (lowercase), numbers or special symbols (_-)");
        return false;
    }
    return true;
}

bool CreateAccountPage::onNameEditFinished(DLineEdit *edit)
{
    const QString &username = edit->lineEdit()->text();

    QString invalidReason;
    if (!validateUsername(username, invalidReason)) {
        edit->setAlert(true);
        edit->showAlertMessage(invalidReason, -1);
        return false;
    }
    return true;
}

bool CreateAccountPage::onFullNameEidtFinished(DLineEdit *edit)
{
    auto userFullName = edit->lineEdit()->text();
    auto userList = m_userModel->userList();
    for (auto u : userList) {
        if (u->fullname() == userFullName && u->fullname() != nullptr) {
            edit->setAlert(true);
            edit->showAlertMessage(tr("The full name already exists"), -1);
            return false;
        }
    }

    if (userFullName.size() > 100) {
        edit->setAlert(true);
        edit->showAlertMessage(tr("The full name is too long"), -1);
        return false;
    }
    return true;
}
