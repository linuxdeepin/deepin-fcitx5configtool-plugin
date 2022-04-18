/*
* Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
*
* Author:     liuwenhao <liuwenhao@uniontech.com>
*
* Maintainer: liuwenhao <liuwenhao@uniontech.com>
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

#include "namespace.h"

class QWidget;
class QString;

namespace DCC_NAMESPACE {
class ModuleInterface;
class FrameProxyInterface
{
public:
    enum PushType {
        Replace,
        CoverTop,
        Normal,
        DirectTop,
        Count
    };

public:
    // Module request to into next page
    virtual void pushWidget(ModuleInterface *const inter, QWidget *const w, PushType type = Normal) = 0;

    virtual void popWidget(ModuleInterface *const inter) = 0;

    virtual void setModuleVisible(ModuleInterface *const inter, const bool visible) = 0;

    virtual void showModulePage(const QString &module, const QString &page, bool animation) = 0;

    virtual void setModuleSubscriptVisible(const QString &module, bool bIsDisplay) = 0;

    virtual void setRemoveableDeviceStatus(QString type, bool state) = 0;
    virtual bool getRemoveableDeviceStatus(QString type) const = 0;
public:
    ModuleInterface *currModule() const { return m_currModule; }

protected:
    void setCurrModule(ModuleInterface *const m) { m_currModule = m; }

private:
    ModuleInterface *m_currModule{nullptr};
};
}
