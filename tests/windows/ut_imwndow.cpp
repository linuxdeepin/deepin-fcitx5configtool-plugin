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
#include "window/imwindow.h"
//#include <QObject>
class ut_imwindow : public ::testing::Test
{
protected:
    ut_imwindow()
    {
    }

    virtual ~ut_imwindow()
    {
    }

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(ut_imwindow, initUI)
{
    IMWindow imwindow;
    imwindow.initUI();
}

TEST_F(ut_imwindow, initConnect)
{
    IMWindow imwindow;
    imwindow.initConnect();
}

//TEST_F(ut_imwindow, initFcitxInterface)
//{
//    IMWindow imwindow;
//    imwindow.initFcitxInterface();
//}
