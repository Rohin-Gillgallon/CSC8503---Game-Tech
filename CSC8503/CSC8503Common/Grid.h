#pragma once

#include "../CSC8503Common/PhysicsSystem.h"


namespace NCL {
	class Grid{
	public:
        struct Cell {
            Cell(int i, int j) {
                c = i, d = j;
            }
            bool walls[4] = { true, true, true, true };
            
            int c;
            int d;
        };
		int index(int i, int j);
		void removeWalls(Cell a, Cell b);
        Cell checkNeighbours(Cell current) {
            int i = current.c;
            int j = current.d;
            vector<Cell> neighbours;

            Cell top = (j == 0) ? grid[index(i, j + 1)] : grid[index(i, j - 1)];
            bool topV = (j == 0) ? visited[index(i, j + 1)] : visited[index(i, j - 1)];
            Cell right = (i == 29) ? grid[index(i - 1, j)] : grid[index(i + 1, j)];
            bool rightV = (i == 29) ? visited[index(i - 1, j)] : visited[index(i + 1, j)];
            Cell bottom = (j == 29) ? grid[index(i, j - 1)] : grid[index(i, j + 1)];
            bool bottomV = (j == 29) ? visited[index(i, j - 1)] : visited[index(i, j + 1)];
            Cell left = (i == 0) ? grid[index(i + 1, j)] : grid[index(i - 1, j)];
            bool leftV = (i == 0) ? visited[index(i + 1, j)] : visited[index(i - 1, j)];

            if (!topV) {
                neighbours.push_back(top);
            }
            if (!rightV) {
                neighbours.push_back(right);
            }
            if (!bottomV) {
                neighbours.push_back(bottom);
            }
            if (!leftV) {
                neighbours.push_back(left);
            }

            if (neighbours.size() > 0) {
                int r = floor(rand() % neighbours.size());
                return neighbours[r];
            }
            else {
                return current;
            }
        }
		int cols = 30;
		int rows = 30;
        vector<Cell> grid;
        void Build() {
            for (int j = 0; j < rows; j++) {
                for (int i = 0; i < cols; i++) {
                    grid.push_back(Cell(i, j));
                }
            }
        }
                
            
        
        bool visited[300]{};
		int current;
		vector<Cell> stack;
	};
}