#include "fcitxconfigplugin.h"
#include "interface/moduleobject.h"

#include <QLabel>
#include <qboxlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwidget.h>

#include "window/imwindow.h"

QString FcitxConfigPlugin::name() const
{
    return tr("Input Methods");
}

QString FcitxConfigPlugin::follow() const
{
    return "keyboard";
}

int FcitxConfigPlugin::location() const
{
    return 1;
}

ModuleObject* FcitxConfigPlugin::module()
{
    ModuleObject *module111 = new ModuleObject(QString("menu1"), tr("输入法配置"), this);
    module111->setChildType(ModuleObject::ChildType::Page);

        LabelModule *labelModule = new LabelModule();
        module111->appendChild(labelModule);

    return module111;
}



QWidget *LabelModule::page()
{
    IMWindow *m_imWindow = new IMWindow();
    return m_imWindow;
}

