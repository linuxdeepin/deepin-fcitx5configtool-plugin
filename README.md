# deepin-fcitxconfigtool-plugin 

#### 介绍

deepin-fcitxconfigtool-plugin 

dde控制中心输入法管理插件，目前做了基于fcitx4输入法框架的适配。

#### 安装教程

```
sudo apt install deepin-fcitxconfigtool-plugin 
```

#### 编译依赖

```
sudo apt build-dep deepin-fcitxconfigtool-plugin 
```

```
sudo apt install  debhelper qt5-qmake libqt5widgets5 libqt5network5 libdtkwidget-dev libdtkgui-dev libdtkgui5 libdtkgui5-bin libdtkcore5-bin libdtkcore-dev qttools5-dev qttools5-dev-tools pkg-config cmake fcitx-bin fcitx-libs-dev libfcitx-qt5-1 libfcitx-qt5-data libfcitx-qt5-dev iso-codes libdbus-glib-1-dev libgtk-3-dev libgtk2.0-dev libunique-dev libdframeworkdbus-dev libgtest-dev libgmock-dev dde-control-center-dev extra-cmake-modules libkf5i18n-dev
```

#### 翻译

更新

```
tx pull -s
```

推送

```
tx push -s
```

翻译使用

(https://www.transifex.com/linuxdeepin/)[https://www.transifex.com/linuxdeepin/]

所以请大家不要在github提交翻译.

#### 使用说明

git clone https://github.com/linuxdeepin/deepin-fcitxconfigtool-plugin.git

sudo apt build-dep deepin-fcitxconfigtool-plugin 

cd ./deepin-fcitxconfigtool-plugin 

sudo dpkg-buildpackage -b

cd ..

sudo dpkg -i *.deb

#### 学习开发说明

欢迎进行pr，鼓励大家参与输入法管理器的产品设计与代码编写。

#### 参与贡献

1.  cssplay
2.  liuwenhao
3.  chenshijie
4.  zhaoyue


