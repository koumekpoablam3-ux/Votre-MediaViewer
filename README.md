MediaViewer v1.1.0
Visionneuse d’Images, Lecteur Audio & Video - Application C++ / Qt6
Par RODRIGUE
Fonctionnalites
Visionneuse d’Images
Formats compatibles : PNG, JPEG, BMP, GIF, TIFF, WebP, SVG, ICO
Zoom : molette de souris (zoom vers le curseur), boutons +/-
Panoramique : cliquer-glisser pour déplacer l’image
Rotation : gauche (-90 degrés) / droite (+90 degrés)
Miroir : horizontal / vertical
Ajustement : ajuster à la fenètre / taille reelle / double-clic pour basculer
Navigation : précédent / suivant dans une série d’images
Diaporama : déroulement automatique (intervalle 3 secondes)
Ouvrir : fichier unique ou dossier complet
Lecteur Audio
Formats supportent : MP3, WAV, FLAC, OGG, WMA, AAC, M4A, Opus, AIFF
Contrôles : Lecture, Pause, Arrêt, Précédent, Suivant
Barre de progression : navigation dans la piste
Volume : curseur de volume + bouton mute
Playlist : ajout de fichiers/dossiers, glisser-deposer, reordonnancement
Melange : lecture aleatoire
Répétition : aucune / tout / piste unique
Info piste : titre, artiste (extrait du nom de fichier), format
Lecteur Video
Formats compatibles : MP4, AVI, MKV, WMV, MOV, FLV, WebM
Contrôles : Cours, Pause, Arrêt
Barre de progression : navigation dans la vidéo
Volume : curseur de volume + bouton mute
Plein ecran : mode plein ecran video (touche F ou bouton)
Affichage temps : format heures :minutes :secondes pour les vidéos longues
Statut : affichage du nom du fichier en cours de lecture
Prerequis (Windows)
1. Installateur Qt 6
Option A : Installateur Qt Online (recommandé)

Telecharger depuis : https://www.qt.io/download-qt-installer
Lancer l’installateur
Selectionner les composants suivants :
Qt 6.x > Desktop > MinGW 11.2+ (ou MSVC 2019/2022)
Qt Multimédia
Widgets multimédias Qt
Qt SVG
CMake
MinGW (sous-outils développeurs)
2. Installateur CMake
Téléchargeur : https://cmake.org/download/
Ou via l’installateur Qt (inclus)
3. Installer un compilateur
MinGW (inclut avec Qt) - Recommandation :

Installe automatiquement avec le composant Qt > MinGW
MSVC (Outils de compilation Visual Studio) :

Téléchargeur : https://visualstudio.microsoft.com/downloads/
Selectionner « Outils de construction pour Visual Studio »
Cocher « Développement de bureau avec C++ »
Compilation
Avec CMake (ligne de commande)
cd MediaViewer
mkdir build
cd build

# Pour MinGW :
cmake .. -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/mingw_64"

# Pour MSVC :
cmake .. -G "Visual Studio 17 2022" -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2022_64"

# Compiler
cmake --build . --config Release

# Executer
Release\MediaViewer.exe   # MSVC
MediaViewer.exe           # MinGW
Avec Qt Creator (recommande)
Ouvrir Qt Creator
Fichier > Fichier ou projet ouvert
Selectionner le fichier CMakeLists.txt
Configurer le Kit de compilation (Desktop Qt 6.x MinGW/MSVC)
Cliquer sur Build (Ctrl+B)
Cliquer sur Run (Ctrl+R)
Structure du projet
MediaViewer/
  CMakeLists.txt          # Systeme de build CMake
  main.cpp                # Point d'entree + theme global
  MainWindow.h/cpp        # Fenetre principale (onglets Image/Audio/Video)
  ImageViewer.h/cpp       # Visionneuse d'images (zoom, rotation, diaporama)
  AudioPlayer.h/cpp       # Lecteur audio (lecture, volume, playlist)
  VideoPlayer.h/cpp       # Lecteur video (lecture, volume, plein ecran)
  PlaylistWidget.h/cpp    # Liste de lecture avec glisser-deposer
  README.md               # Ce fichier
Clavier de Raccourcis
Généraux
Raccourci	Action
Ctrl+O	Ouvrir un fichier
Ctrl+Shift+O	Ouvrir un dossier
F11	Plein ecran
Ctrl+Q	Abandonneur
Visionneuse d’Images
Raccourci	Action
Gauche / Droite	Image précédente / suivante
Ctrl++	Zoom avant
Ctrl+-	Zoom arrière
Ctrl+F	Ajuster à la fenètre
Ctrl+1	Taille reelle
Ctrl+L	Gauche de rotation
Ctrl+R	Rotation droite
Double clic	Basculer ajuster/taille reelle
Molette souris	Zoom vers le curseur
Cliquer-glisser	Panoramique
Lecteur Audio
Raccourci	Action
Espace	Lecture / Pause
S	Activer/desactiver le melange
R	Changer le mode de repetition
M	Mute / Unmute
Lecteur Video
Raccourci	Action
Espace	Lecture / Pause
F	Plein ecran / Fenetre
Formats supportes
Type	Formats
Images	PNG, JPEG, BMP, GIF, TIFF, WebP, SVG, ICO
Audio	MP3, WAV, FLAC, OGG, WMA, AAC, M4A, Opus, AIFF
Vidéo	MP4, AVI, MKV, WMV, MOV, FLV, WebM
Note : Pour les formats vidéo AVI, MKV et WebM, il peut être nécessaire d’installer des codecs supplementaires sur Windows. Le format MP4 est le mieux supporté nativement.

Technologies
Langage : C++17
Interface graphique du cadre : Qt 6 (Widgets + Multimédia + MultimediaWidgets)
Build : CMake 3.16+
Format image : Qt QImage + plugin SVG
Formats audio/vidéo : Qt QMediaPlayer + QAudioOutput + QVideoWidget
Auteur
RODRIGUE - MediaViewer v1.1.0

Projet libre d’utilisation.
