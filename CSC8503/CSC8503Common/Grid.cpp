#include "Grid.h"

using namespace NCL;
using namespace CSC8503;

int Grid::index(int i, int j)
{
    if (i < 0 || j < 0 || i > cols - 1 || j > rows - 1) {
        return -1;
    }
    return i + j * cols;
}

void Grid::removeWalls(Cell a, Cell b)
{
    int i = a.c - b.d;
    int j = a.c - b.d;

    if (i == 1) {
        a.walls[3] = false;
        b.walls[1] = false;
    }
    else if (i == -1) {
        a.walls[1] = false;
        b.walls[3] = false;
    }

    if (j == 1) {
        a.walls[0] = false;
        b.walls[2] = false;
    }
    else if (j == -1) {
        a.walls[2] = false;
        b.walls[0] = false;
    }
}
