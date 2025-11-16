#include <SFML/Graphics.h>

#include "Grid.h"

static sfFont* globalFont = NULL;
Cell* CellCreate(sfVector2f size, sfVector2f pos, sfColor color)
{

	Cell* newCell = (Cell*)malloc(sizeof(Cell));
	if (newCell == NULL) {
		return NULL;
	}

	newCell->shape = sfRectangleShape_create();
	sfRectangleShape_setSize(newCell->shape, size);
	sfRectangleShape_setPosition(newCell->shape, pos);
	sfRectangleShape_setFillColor(newCell->shape, color);
	sfRectangleShape_setOutlineColor(newCell->shape, sfBlack);
	sfRectangleShape_setOutlineThickness(newCell->shape, 1.0f);

	newCell->text = sfText_create();

	sfText_setCharacterSize(newCell->text, 20);
	sfText_setFillColor(newCell->text, sfBlack);
	sfText_setString(newCell->text, ""); 

	sfVector2f textPos = {
		pos.x + (size.x - 10) / 2, 
		pos.y + (size.y - 20) / 2  
	};
	sfText_setPosition(newCell->text, textPos);


	newCell->bDiscovered = false;
	newCell->bFlagged = false;
	newCell->bPlanted = false;
	newCell->explosiveNeighbor = 0;

	return newCell;
}

void CellDraw(Cell* cell, sfRenderWindow* window)
{

	if (cell->bDiscovered) {
		if (cell->bPlanted) {
			// Bombe
			sfText_setString(cell->text, "X");
			sfText_setColor(cell->text, sfBlack);
			sfRenderWindow_drawText(window, cell->text, NULL);
		}
		else if (cell->explosiveNeighbor > 0) {
			// Numéro
			char numberStr[2];
			snprintf(numberStr, sizeof(numberStr), "%d", cell->explosiveNeighbor);
			sfText_setString(cell->text, numberStr);

		
			switch (cell->explosiveNeighbor) {
			case 1: sfText_setColor(cell->text, sfBlue); break;
			case 2: sfText_setColor(cell->text, sfGreen); break;
			case 3: sfText_setColor(cell->text, sfRed); break;
			case 4: sfText_setColor(cell->text, sfColor_fromRGB(0, 0, 128)); break;
			case 5: sfText_setColor(cell->text, sfColor_fromRGB(128, 0, 0)); break;
			case 6: sfText_setColor(cell->text, sfColor_fromRGB(0, 128, 128)); break;
			case 7: sfText_setColor(cell->text, sfBlack); break;
			case 8: sfText_setColor(cell->text, sfColor_fromRGB(64, 64, 64)); break;
			}
			sfRenderWindow_drawText(window, cell->text, NULL);
		}
	}
	else if (cell->bFlagged) {
		// Drapeau
		sfText_setString(cell->text, "F");
		sfText_setColor(cell->text, sfRed);
		sfRenderWindow_drawText(window, cell->text, NULL);
	}

	sfRenderWindow_drawRectangleShape(window, cell->shape, NULL);
}


int CellReveal(Grid* grid, sfVector2i cellGridPos)
{
	Cell* cell = grid->cells[cellGridPos.x][cellGridPos.y];
	if (cell->bDiscovered || cell->bFlagged) {
		return 0;
	}

	if (cell->bPlanted) {
		return FAILURE;
	}

	sfRectangleShape_setFillColor(cell->shape, sfColor_fromRGB(200, 200, 200));
	cell->bDiscovered = true;

	if (cell->explosiveNeighbor > 0) {
		char numberStr[2];
		snprintf(numberStr, sizeof(numberStr), "%d", cell->explosiveNeighbor);
		sfText_setString(cell->text, numberStr);

		switch (cell->explosiveNeighbor) {
		case 1: sfText_setColor(cell->text, sfBlue); break;
		case 2: sfText_setColor(cell->text, sfGreen); break;
		case 3: sfText_setColor(cell->text, sfRed); break;
		case 4: sfText_setColor(cell->text, sfColor_fromRGB(0, 0, 128)); break; // Dark blue
		case 5: sfText_setColor(cell->text, sfColor_fromRGB(128, 0, 0)); break; // Dark red
		case 6: sfText_setColor(cell->text, sfColor_fromRGB(0, 128, 128)); break; // Teal
		case 7: sfText_setColor(cell->text, sfBlack); break;
		case 8: sfText_setColor(cell->text, sfColor_fromRGB(64, 64, 64)); break; // Gray
		default: sfText_setColor(cell->text, sfBlack); break;
		}
	}
	else {
	
		sfText_setString(cell->text, "");
	}

	if (cell->explosiveNeighbor == 0) {
		for (int dx = -1; dx <= 1; dx++) {
			for (int dy = -1; dy <= 1; dy++) {
				int newX = cellGridPos.x + dx;
				int newY = cellGridPos.y + dy;

				if (newX >= 0 && newX < GRID_SIZE && newY >= 0 && newY < GRID_SIZE) {
					if (!(dx == 0 && dy == 0)) {
						Cell* neighbor = grid->cells[newX][newY];
						if (!neighbor->bDiscovered && !neighbor->bFlagged) {
							CellReveal(grid, (sfVector2i) { newX, newY });
						}
					}
				}
			}
		}
	}

	grid->discoveredCellCount++;

	int totalCells = GRID_SIZE * GRID_SIZE;
	int bombCount = 0;

	for (int i = 0; i < GRID_SIZE; i++) {
		for (int j = 0; j < GRID_SIZE; j++) {
			if (grid->cells[i][j]->bPlanted) {
				bombCount++;
			}
		}
	}

	if (grid->discoveredCellCount >= (totalCells - bombCount)) {
		return SUCCESS;
	}

	return 0;
}

void CellFlag(Grid* grid, sfVector2i cellGridPos)
{
	Cell* cell = grid->cells[cellGridPos.x][cellGridPos.y];

	if (cell->bDiscovered) return;

	cell->bFlagged = !cell->bFlagged;

	if (cell->bFlagged) {
		sfRectangleShape_setFillColor(cell->shape, sfColor_fromRGB(255, 100, 100));
	}
	else {
		sfRectangleShape_setFillColor(cell->shape, sfColor_fromRGB(120, 120, 120));
	}
}


void CellDestroy(Cell* cell)
{
	if (cell->shape != NULL) {
		sfRectangleShape_destroy(cell->shape);
		cell->shape = NULL;
	}

	if (cell->text != NULL) {
		sfText_destroy(cell->text);
		cell->text = NULL;
	}

	free(cell);
}


Grid* GridCreate()
{
	Grid* newGrid = (Grid*)malloc(sizeof(Grid));
	if (!newGrid) return NULL;

	newGrid->discoveredCellCount = 0;

	for (int x = 0; x < GRID_SIZE; x++) {
		for (int y = 0; y < GRID_SIZE; y++) {
			sfVector2f cellSize = { 30.0f, 30.0f };
			sfVector2f cellPos = { x * 30.0f, y * 30.0f };
			sfColor cellColor = sfColor_fromRGB(120, 120, 120); 

			newGrid->cells[x][y] = CellCreate(cellSize, cellPos, cellColor);

			if (globalFont != NULL) {
				sfText_setFont(newGrid->cells[x][y]->text, globalFont);
			}
		}
	}

	return newGrid;
}

void GridPlantBomb(Grid* grid, int bombCount, sfVector2i cellToAvoid)
{
	int bombsPlanted = 0;
	int attempts = 0;
	int maxAttempts = GRID_SIZE * GRID_SIZE * 2; 

	while (bombsPlanted < bombCount && attempts < maxAttempts) {
		int randomX = rand() % GRID_SIZE;
		int randomY = rand() % GRID_SIZE;

		if ((randomX != cellToAvoid.x || randomY != cellToAvoid.y) &&
			!grid->cells[randomX][randomY]->bPlanted) {

			grid->cells[randomX][randomY]->bPlanted = true;
			bombsPlanted++;

			for (int dx = -1; dx <= 1; dx++) {
				for (int dy = -1; dy <= 1; dy++) {
					int neighborX = randomX + dx;
					int neighborY = randomY + dy;

					if (neighborX >= 0 && neighborX < GRID_SIZE &&
						neighborY >= 0 && neighborY < GRID_SIZE) {
						grid->cells[neighborX][neighborY]->explosiveNeighbor++;
					}
				}
			}
		}
		attempts++;
	}

	if (bombsPlanted < bombCount) {
		printf("Warning: Only planted %d out of %d bombs\n", bombsPlanted, bombCount);
	}
}


sfVector2i GridUpdateLoop(Grid* grid, sfRenderWindow* window)
{
	sfVector2i mousePos = sfMouse_getPositionRenderWindow(window);

	sfVector2i cellCoord = { -1, -1 };

	return cellCoord;
}

void GridDraw(Grid* grid, sfRenderWindow* window)
{
	{
		for (int x = 0; x < GRID_SIZE; x++) {
			for (int y = 0; y < GRID_SIZE; y++) {
				CellDraw(grid->cells[x][y], window);
			}
		}
	}
}

void GridDestroy(Grid* grid)
{
	for (int x = 0; x < GRID_SIZE; x++) {
		for (int y = 0; y < GRID_SIZE; y++) {
			if (grid->cells[x][y] != NULL) {
				CellDestroy(grid->cells[x][y]);
				grid->cells[x][y] = NULL;
			}
		}
	}
	free(grid);

	if (globalFont != NULL) {
		sfFont_destroy(globalFont);
		globalFont = NULL;
	}
}