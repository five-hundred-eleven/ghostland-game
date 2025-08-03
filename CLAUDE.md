# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

- **Build the game**: `make`
- **Clean build artifacts**: `make clean`
- **Run the game**: `./ghostland`

## Architecture Overview

This is a C++ OpenGL 3D game called "ghostland-game" - a continuation of a previous Go-based project. The player navigates a maze-like environment while being pursued by ghosts.

### Core Components

- **ghostland.cpp**: Main game loop, OpenGL setup, window management, and input handling
- **Player**: First-person camera system with mouse look, movement physics (including jumping), and flashlight mechanics
- **Ghost**: Enemy entities that move autonomously within defined boundaries and face the player
- **Shader system**: OpenGL shader loading and uniform management for different rendering passes
- **Text rendering**: FreeType-based text rendering system for UI elements like FPS display
- **Collision detection**: Ray-triangle intersection testing for wall collisions

### Key Systems

- **Maze loading**: The `maze.txt` file contains vertex data for walls parsed at runtime
- **Lighting**: Dynamic lighting system with player-controlled flashlight that responds to mouse movement
- **Rendering pipeline**: Separate shaders for different object types (walls, ghosts, text, trails)
- **Physics**: Simple gravity-based jumping with collision detection against maze geometry

### Dependencies

- OpenGL 3.3+ with GLAD loader
- GLFW for window management and input
- GLM for math operations
- FreeType for text rendering
- stb_image for texture loading

### File Structure

- Header files (.h) define class interfaces
- Implementation files (.cpp) contain the logic
- Shader files (.glsl) define rendering programs
- Object files (.o) are generated during build
- `maze.txt` contains level geometry data
- `fonts/` contains TrueType fonts for text rendering