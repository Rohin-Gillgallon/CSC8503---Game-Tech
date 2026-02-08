# CSC8503 - Advanced Game Tech

This repository contains the final coursework submission for the CSC8503 Advanced Game Tech module. It features a custom-built physics and networking engine designed to support a complex 3D obstacle course and maze environment.

The main project source and solution files are located in the CSC8503 directory.

## Project Overview
This project demonstrates advanced game engine programming, focusing on custom physics integration, collision detection, and interactive game mechanics. The application consists of two main levels: an obstacle course and a competitive maze environment.



### Technical Features
- Physics Engine: Custom implementation of gravity, forces, and impulses. Includes a penalty method for springboards and varied friction/restitution for different surfaces like ice.
- Collision Detection: Optimized collision handling using a Quadtree spatial partitioning system.
- Mechanics and Constraints: Features complex interactive elements such as magnet-driven lifts, wobble mazes with rotational constraints, and gravity wells.
- Gameplay Systems: Integrated scoring system, collectible items (coins), and a death plane system with respawn logic.
- UI and Debugging: Includes a main menu system, scoreboards, and a dedicated debug mode for real-time physics information.

## Project Structure
- CSC8503/
  - GameLevel.cpp / .h: Logic for level loading and objective management.
  - PhysicsSystem.cpp / .h: Core engine logic for integration and collision resolution.
  - Quadtree.h: Spatial partitioning implementation for performance optimization.
  - PlayerObject.cpp / .h: Specialized game object for player movement and interaction.
- Additional Document: Detailed breakdown of obstacles including the Wobble Maze, Constraint Net, and Teleports.

## Controls
- Mouse: Turn Camera
- W, A, S, D: Move Camera
- Q: Toggle Camera Mode
- Right Click: Apply force to ball (Main gameplay mechanic)
- B: Activate Quadtree Debugging
- G: Toggle Gravity
- M: Toggle Magnet
- R: Respawn
- T: Use Teleport (when active)
- Arrow Keys: Control specific platforms (Lifts and Wobble Maze)

## Levels
1. Level 1 - Obstacle Course: A series of challenges testing physics interactions, including rising platforms, magnet lifts, and a constraint-based net catch.
2. Level 2 - Maze: A competitive environment focusing on navigation, collection, and efficient movement using the force-application mechanic.

## Setup and Installation

### 1. Prerequisites
- Visual Studio 2019 or 2022 with C++ Desktop Development workload.
- Windows 10/11 environment.

### 2. Execution Instructions
1. Clone the repository:
   git clone https://github.com/Rohin-Gillgallon/CSC8503---Game-Tech
2. Open the .sln solution file in Visual Studio.
3. Set the startup project to CSC8503.
4. Build the solution and run in Release mode for optimal physics performance.

---
Author: Rohin Gillgallon****
