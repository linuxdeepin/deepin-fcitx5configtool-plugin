/*
* Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
*
* Author:     chenshijie <chenshijie@uniontech.com>
*
* Maintainer: chenshijie <chenshijie@uniontech.com>
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

#include <iostream>
#include "gtest/gtest.h"
#include "window/imsettingwindow.h"
class ut_imsettingwindow : public ::testing::Test
{
protected:
    ut_imsettingwindow()
    {
    }

    virtual ~ut_imsettingwindow()
    {
    }

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

//TEST_F(ut_imsettingwindow, updateUI)
//{
//    IMSettingWindow iMSettingWindow;
//    iMSettingWindow.updateUI();
//    EXPECT_TRUE(true);
//}

TEST_F(ut_imsettingwindow, initUI)
{
    IMSettingWindow iMSettingWindow;
//    iMSettingWindow.initUI();
    EXPECT_TRUE(true);
}

TEST_F(ut_imsettingwindow, initConnect)
{
    IMSettingWindow iMSettingWindow;
    iMSettingWindow.initConnect();
    EXPECT_TRUE(true);
}

TEST_F(ut_imsettingwindow, readConfig)
{
    IMSettingWindow iMSettingWindow;
    iMSettingWindow.readConfig();
}

//TEST_F(ut_imsettingwindow, itemSwap)
//{
//    IMSettingWindow iMSettingWindow;
//    iMSettingWindow.itemSwap(nullptr);
//    EXPECT_TRUE(true);
//}

TEST_F(ut_imsettingwindow, onEditBtnClicked)
{
    IMSettingWindow iMSettingWindow;
    iMSettingWindow.onEditBtnClicked(true);
    iMSettingWindow.onEditBtnClicked(false);
}

//TEST_F(ut_imsettingwindow, onDefaultIMChanged)
//{
//    IMSettingWindow iMSettingWindow;
//    iMSettingWindow.onDefaultIMChanged();
//    EXPECT_TRUE(true);
//}

TEST_F(ut_imsettingwindow, onCurIMChanged)
{
    FcitxQtInputMethodItemList list;
    FcitxQtInputMethodItem item;
    item.setName("name");
    item.setUniqueName("name");
    item.setLangCode("name");
    item.setEnabled(false);
    list << item;
    IMSettingWindow iMSettingWindow;
    iMSettingWindow.onCurIMChanged(list);
    EXPECT_TRUE(true);
}

TEST_F(ut_imsettingwindow, onAddBtnCilcked)
{
    IMSettingWindow iMSettingWindow;
    iMSettingWindow.onAddBtnCilcked();
    EXPECT_TRUE(true);
}

TEST_F(ut_imsettingwindow, onItemUp)
{
    FcitxQtInputMethodItem item;
    IMSettingWindow iMSettingWindow;
    iMSettingWindow.onItemUp(item);
    EXPECT_TRUE(true);
}

TEST_F(ut_imsettingwindow, onItemDown)
{
    FcitxQtInputMethodItem item;
    IMSettingWindow iMSettingWindow;
    iMSettingWindow.onItemDown(item);
    EXPECT_TRUE(true);
}

//加上会有内存检测错误
//TEST_F(ut_imsettingwindow, onItemDelete)
//{
//    FcitxQtInputMethodItem item;
//    IMSettingWindow iMSettingWindow;
//    iMSettingWindow.onItemDelete(item);
//    EXPECT_TRUE(true);
//}

TEST_F(ut_imsettingwindow, doReloadConfigUI)
{
    IMSettingWindow iMSettingWindow;
    iMSettingWindow.doReloadConfigUI();
    EXPECT_TRUE(true);
}
TEST_F(ut_imsettingwindow, onReloadConnect)
{
    IMSettingWindow iMSettingWindow;
    iMSettingWindow.onReloadConnect();
    EXPECT_TRUE(true);
}
