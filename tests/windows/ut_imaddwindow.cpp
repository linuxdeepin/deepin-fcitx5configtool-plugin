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
#include "window/imaddwindow.h"
class ut_imaddwindow : public ::testing::Test
{
protected:
    ut_imaddwindow()
    {
    }

    virtual ~ut_imaddwindow()
    {
    }

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(ut_imaddwindow, initUI)
{
    IMAddWindow iMAddWindow;
    iMAddWindow.initUI();
}

TEST_F(ut_imaddwindow, initConnect)
{
    IMAddWindow iMAddWindow;
    iMAddWindow.initConnect();
}

TEST_F(ut_imaddwindow, onAddIM)
{
    IMAddWindow iMAddWindow;
    iMAddWindow.initUI();
    iMAddWindow.onAddIM();
}

TEST_F(ut_imaddwindow, updateUI)
{
    IMAddWindow iMAddWindow;
    iMAddWindow.initUI();
    iMAddWindow.updateUI();
}
TEST_F(ut_imaddwindow, onOpenStore)
{
    IMAddWindow iMAddWindow;
    iMAddWindow.onOpenStore();

    sleep(4);
    FILE *fp;
    int pid = -1;
    char unused[70];
    char buf[150];
    char command[150];
    sprintf(command,
            "ps -ef | grep deepin-home-appstore-client | grep -v grep");
    if ((fp = popen(command, "r")) == NULL)
        return ;
    if ((fgets(buf, 150, fp)) != NULL) {
        sscanf(buf, "%69s\t%d\t%69s", unused, &pid, unused);
    }
    pclose(fp);
    if(pid == 32767)
        return ;
    sprintf(command, "kill -9 %d", pid);
    if ((fp = popen(command, "r")) == NULL)
        return ;
    pclose(fp);

    sprintf(command,
            "ps -ef | grep deepin-home-appstore-client | grep -v grep");
    if ((fp = popen(command, "r")) == NULL)
        return ;
    if ((fgets(buf, 150, fp)) != NULL) {
        sscanf(buf, "%69s\t%d\t%69s", unused, &pid, unused);
    }
    pclose(fp);
    if(pid == 32767)
        return ;
    sprintf(command, "kill -9 %d", pid);
    if ((fp = popen(command, "r")) == NULL)
        return ;
    pclose(fp);
    return ;
}
TEST_F(ut_imaddwindow, doRemoveSeleteIm)
{
    IMAddWindow iMAddWindow;
    FcitxQtInputMethodItem item;
    iMAddWindow.doRemoveSeleteIm(item);
}
