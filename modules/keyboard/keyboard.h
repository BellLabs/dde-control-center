#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <QObject>
#include <QtPlugin>
#include <QRunnable>

#include <libdui/libdui_global.h>
#include <libdui/dbuttonlist.h>

#include "interfaces.h"

#include "dbus/dbuskeyboard.h"

DUI_USE_NAMESPACE

class QFrame;
class SearchItem;
class SearchList;
class AddRmDoneLine;
class FirstLetterClassify;
class QVBoxLayout;
class KeyboardLayoutDelegate;
class DbusLangSelector;
class Keyboard: public QObject, ModuleInterface, QRunnable
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.ControlCenter.ModuleInterface" FILE "keyboard.json")
    Q_INTERFACES(ModuleInterface)

public:
    Keyboard();
    ~Keyboard() Q_DECL_OVERRIDE;
    QFrame* getContent() Q_DECL_OVERRIDE;

private slots:
    void loadLetterClassify();
    void onAddLayoutItem(const QString &id, const QString &title, const QStringList &letterFirstList);

signals:
    void addLayoutItem(const QString &id, const QString &title, const QStringList &letterFirstList);

private:
    QFrame * m_frame;
    DBusKeyboard * m_dbusKeyboard;
    QMap<QString, QString> m_mapUserLayoutInfo;
    QMap<QString, int> m_mapUserLayoutIndex;
    QList<KeyboardLayoutDelegate*> m_selectLayoutList;
    FirstLetterClassify *m_letterClassifyList;
    QVBoxLayout *m_mainLayout;
    SearchList *m_languageSearchList;
    DbusLangSelector *m_dbusLangSelector;

    void updateKeyboardLayout(SearchList *button_list, AddRmDoneLine *line, bool showRemoveButton = false);
    void initBackend();
    void initUI();
    void run();
};

#endif //   KEYBOARD_H
