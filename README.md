# MediaViewer v1.0.0

## Visionneuse d'Images & Lecteur Audio - Application C++ / Qt6

---

## Fonctionnalites

### Visionneuse d'Images
- **Formats supportes** : PNG, JPEG, BMP, GIF, TIFF, WebP, SVG, ICO
- **Zoom** : molette de souris (zoom vers le curseur), +/-
- **Panoramique** : cliquer-glisser pour deplacer l'image
- **Rotation** : gauche (-90 degres) / droite (+90 degres)
- **Miroir** : horizontal / vertical
- **Ajustement** : ajuster a la fenetre / taille reelle / double-clic pour basculer
- **Navigation** : precedent / suivant dans une serie d'images
- **Diaporama** : deroulement automatique (intervalle 3 secondes)
- **Ouvrir** : fichier unique ou dossier complet

### Lecteur Audio
- **Formats supportes** : MP3, WAV, FLAC, OGG, WMA, AAC, M4A, Opus, AIFF
- **Controles** : Lecture, Pause, Stop, Precedent, Suivant
- **Barre de progression** : navigation dans la piste
- **Volume** : curseur de volume + bouton mute
- **Playlist** : ajout de fichiers/dossiers, glisser-deposer, reordonnancement
- **Melange** : lecture aleatoire
- **Repetition** : aucune / tout / piste unique
- **Info piste** : titre, artiste (extrait du nom de fichier), format

---

## Prerequis (Windows)

### 1. Installer Qt 6

**Option A : Qt Online Installer (recommande)**
1. Telecharger depuis : https://www.qt.io/download-qt-installer
2. Lancer l'installateur
3. Selectionner les composants suivants :
   - **Qt 6.x** > **Desktop** > **MinGW 11.2+ (ou MSVC 2019/2022)**
   - **Qt Multimedia**
   - **Qt SVG**
   - **CMake**

**Option B : vcpkg**
```powershell
vcpkg install qt6-base qt6-multimedia qt6-svg qt6-declarative
```

### 2. Installer CMake

- Telecharger : https://cmake.org/download/
- Ou via l'installateur Qt (inclus)

### 3. Installer un compilateur

**MinGW (inclut avec Qt)** - Recommande :
- Installe automatiquement avec le composant Qt > MinGW

**MSVC (Visual Studio Build Tools)** :
- Telecharger : https://visualstudio.microsoft.com/downloads/
- Selectionner "Build Tools for Visual Studio"
- Cocher "Desktop development with C++"

---

## Compilation

### Avec CMake (ligne de commande)

Ouvrez un terminal (PowerShell ou cmd) et executez :

```powershell
# Se placer dans le repertoire du projet
cd MediaViewer

# Creer le repertoire de build
mkdir build
cd build

# Configurer avec CMake
# Pour MinGW :
cmake .. -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/mingw_64"

# Pour MSVC :
cmake .. -G "Visual Studio 17 2022" -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2022_64"

# Compiler
cmake --build . --config Release

# Executer
Release\MediaViewer.exe   # MSVC
MediaViewer.exe           # MinGW
```

### Avec Qt Creator (IDE)

1. Ouvrir Qt Creator
2. **File** > **Open File or Project**
3. Selectionner le fichier `CMakeLists.txt`
4. Configurer le Kit de compilation (Desktop Qt 6.x MinGW/MSVC)
5. Cliquer sur **Build** (Ctrl+B)
6. Cliquer sur **Run** (Ctrl+R)

---

## Structure du projet

```
MediaViewer/
  CMakeLists.txt          # Systeme de build CMake
  src/
    main.cpp              # Point d'entree
    MainWindow.h/cpp      # Fenetre principale avec onglets
    ImageViewer.h/cpp     # Visionneuse d'images (zoom, rotation, diaporama)
    AudioPlayer.h/cpp     # Lecteur audio (lecture, volume, playlist)
    PlaylistWidget.h/cpp  # Liste de lecture avec glisser-deposer
```

---

## Raccourcis clavier

| Raccourci | Action |
|-----------|--------|
| Ctrl+O | Ouvrir un fichier |
| Ctrl+Shift+O | Ouvrir un dossier |
| F11 | Plein ecran |
| Gauche/Droite | Image precedente/suivante |
| Ctrl++ | Zoom avant |
| Ctrl+- | Zoom arriere |
| Ctrl+F | Ajuster a la fenetre |
| Ctrl+1 | Taille reelle |
| Ctrl+L | Rotation gauche |
| Ctrl+R | Rotation droite |
| Espace | Lecture/Pause audio |
| S | Melange (shuffle) |
| R | Repeter |
| M | Mute/Unmute |
| Ctrl+Q | Quitter |

---

## Technologies

- **Langage** : C++17
- **Framework GUI** : Qt 6 (Widgets + Multimedia)
- **Build** : CMake 3.16+
- **Formats image** : Qt QImage + plugin SVG
- **Formats audio** : Qt QMediaPlayer + QAudioOutput

---

## Licence

Projet libre d'utilisation.
