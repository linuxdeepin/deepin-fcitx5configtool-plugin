#include "fcitx5configplugin.h"
#include "interface/moduleobject.h"

#include <QLabel>
#include <qboxlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwidget.h>

#include "window/imwindow.h"

QString Fcitx5ConfigPlugin::name() const
{
    return tr("Input Methods");
}

QString Fcitx5ConfigPlugin::follow() const
{
    return "keyboard";
}

int Fcitx5ConfigPlugin::location() const
{
    return 1;
}

ModuleObject* Fcitx5ConfigPlugin::module()
{
    ModuleObject *module111 = new ModuleObject(QString("menu1"), tr("输入法"), this);
    module111->setChildType(ModuleObject::Page);

        LabelModule *labelModule = new LabelModule();
        module111->appendChild(labelModule);

    return module111;
}



QWidget *LabelModule::page()
{
    IMWindow *m_imWindow = new IMWindow();
    return m_imWindow;
}

