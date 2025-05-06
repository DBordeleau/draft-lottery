# 🏒 Fantasy Hockey Draft Lottery App

This is a Qt-based desktop application for running a draft lottery for your fantasy hockey league. I wrote it specifically for my fantasy hockey league, but the app dynamically generates the team inputs based on a spinner box and can be used for any league/fantasy sport.

![App Preview](https://www.dillonbordeleau.dev/DraftLotteryPreview.gif)

## ✨ Features

- **Dynamic Team Setup**  
  Add or remove teams using a spinner input — the app will generate input fields for team names and their weighted odds accordingly.

- **Animated Eliminations**  
  Watch teams get eliminated one by one with smooth, suspenseful animations built with Qt.

- **Winner Reveal with Confetti**  
  The final reveal is animated with a burst of confetti.

## 📁 Included in This Repository

- `mainwindow.cpp, draftlottery.pro` – Full C++ source code and the Qt project file can be found in the root directory.
- `build/release` – A precompiled Windows build of the app.
- `installerscript.iss` – An Inno Setup script used to generate a standalone Windows installer.

### Instructions

```bash
git clone https://github.com/your-username/fantasy-hockey-draft-lottery.git
cd fantasy-hockey-draft-lottery
# Open the .pro file in Qt Creator OR build via CMake if set up that way
```

## 📦 Windows Installer

A ready-to-run installer is available via Inno Setup. You can download it from [my fantasy league website](https://yofhl-db.vercel.app/lottery). There's also a ZIP option available at the same page.

To generate your own installer:

1. Download [Inno Setup](https://jrsoftware.org/isinfo.php).
2. Open the `installerscript.iss` in Inno Setup.
3. Compile to generate a `.exe` installer.
