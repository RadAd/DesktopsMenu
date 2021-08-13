<!-- ![Icon](res/main.ico) Desktops Menu -->
<img src="res/main.ico" width=32/> Desktops Menu
==========

Adds items to the windows system menu for virtual desktops

![Windows](https://img.shields.io/badge/platform-Windows-blue.svg)
[![Releases](https://img.shields.io/github/release/RadAd/DesktopsMenu.svg)](https://github.com/RadAd/DesktopsMenu/releases/latest)
[![Build](https://img.shields.io/appveyor/ci/RadAd/DesktopsMenu.svg)](https://ci.appveyor.com/project/RadAd/DesktopsMenu)

![Screenshot](docs/screenshot.png)

Build
=======
```bat
msbuild DesktopsMenu.vcxproj -p:Configuration=Release -p:Platform=x64
```
Run
=======
```bat
rundll32 DesktopsMenu.dll,DesktopsMenu
```
