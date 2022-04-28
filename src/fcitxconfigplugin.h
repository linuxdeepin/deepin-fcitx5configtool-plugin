#pragma once

#include "interface/moduleobject.h"
#include "interface/plugininterface.h"

DCC_USE_NAMESPACE

class FcitxConfigPlugin : public PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PluginInterface_iid FILE "plugin-fcitx.json")
    Q_INTERFACES(DCC_NAMESPACE::PluginInterface)
public:
    FcitxConfigPlugin(){}
    virtual QString name() const override;
    virtual ModuleObject *module() override;
    virtual QString follow() const override;
    virtual int location() const override;
};

class LabelModule : public ModuleObject
{
    Q_OBJECT
public:
    LabelModule(QObject *parent = nullptr) : ModuleObject(parent) {}
    LabelModule(const QString &name, const QString &displayName = {}, QObject *parent = nullptr) : ModuleObject(name, displayName, parent) {}
    virtual QWidget *page() override;

};

