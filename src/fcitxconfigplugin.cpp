#include "fcitxconfigplugin.h"
#include "interface/moduleobject.h"

#include <QLabel>
#include <qboxlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwidget.h>

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

    for (int j = 0; j < 4; j++) {
        LabelModule *labelModule = new LabelModule(QString("main%1menu%2").arg(1).arg(j), QString("具体页面%1的第%2个page的标题").arg(1).arg(j), module111);
        labelModule->setText(QString("我是具体页面%1的第%2个page").arg(1).arg(j));
        module111->appendChild(labelModule);
    }

    return module111;
}



QWidget *LabelModule::page()
{
    return new QLabel(text());
}

void LabelModule::setText(const QString &text)
{
    m_text = text;
}


