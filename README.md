# Descente

**Descente** est un roguelike 2D : tu explores un donjon généré à chaque partie, tu combats au tour par tour, tu gères ton inventaire, et tu affrontes un boss final.

## À quoi ça sert ?

C'est un jeu. Tu incarnes un aventurier qui descend dans les profondeurs.

**Objectif :** atteindre l'étage 5, vaincre le **Gardien Abyssal**, puis ramasser l'**Éclat Abyssal**.

## Installation (recommandée)

1. Va sur la page [Releases](../../releases).
2. Télécharge l'archive pour ton système :
   - Windows → `descente-*-windows-x64.zip`
   - Linux → `descente-*-linux-x64.tar.gz`
   - macOS → `descente-*-macos-arm64.tar.gz`
3. Décompresse l'archive.
4. Lance l'exécutable `descente` (ou `descente.exe` sous Windows).

Sous Linux / macOS, si besoin :

```bash
chmod +x descente
./descente
```

## Compiler depuis les sources

Utile si tu préfères builder toi-même, ou s'il n'y a pas encore de release pour ta plateforme.

### Prérequis

- CMake ≥ 3.16
- Un compilateur C++20 (MSVC, Clang ou GCC)
- Ninja (recommandé)
- Une connexion internet au **premier** build (dépendances téléchargées automatiquement)

Sous Linux, installe aussi les paquets graphiques :

```bash
sudo apt install build-essential cmake ninja-build \
  libgl1-mesa-dev libx11-dev libxcursor-dev \
  libxrandr-dev libxi-dev libxinerama-dev
```

### Compilation

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Puis lance :

```bash
./build/descente          # Linux / macOS
.\build\descente.exe      # Windows
```

## Contrôles

| Touche | Action |
| --- | --- |
| WASD / flèches | Se déplacer (maintenir pour avancer) |
| G | Ramasser un objet |
| I | Ouvrir l'inventaire (cliquer un objet pour l'utiliser) |
| F | Descendre l'escalier |
| Espace / `.` | Attendre un tour |
| Shift+S | Sauvegarder |
| C | Charger une sauvegarde |
| Esc / Q | Quitter |
| N | Nouvelle partie (après une mort ou une victoire) |

## Licence

Ce projet est libre, sous licence [MIT](LICENSE).
