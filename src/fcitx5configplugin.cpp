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
    ModuleObject *module = new ModuleObject(QString("Manage Input Methods"), tr("Input Method"), this);
    module->setChildType(ModuleObject::Page);

        LabelModule *labelModule = new LabelModule();
        module->appendChild(labelModule);

    return module;
}



QWidget *LabelModule::page()
{
    IMWindow *m_imWindow = new IMWindow();
    return m_imWindow;
}

