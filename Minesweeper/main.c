#include <SFML/Audio.h>
#include <SFML/Graphics.h>

#include "basics.h"
#include "Grid.h"

#define DEBUG_CLEAN false

int main(void)
{
    sfVideoMode mode = { WIDTH, HEIGHT, 32 };
    sfRenderWindow* window;
    sfEvent event; 

    window = sfRenderWindow_create(mode, "Minesweeper", sfClose, NULL);
    if (!window)
    {
        return NULL_WINDOW;
    }
    srand((unsigned int)time(NULL));

    Grid* grid = GridCreate();
    if (!grid) {
        sfRenderWindow_destroy(window);
        return EXIT_FAILURE;
    }

    printf("Debut du jeu ! \n");
    bool bFirstTouch = true;
    int gameState = 0;

    sfFont* testFont = sfFont_createFromFile("arial.ttf");
    if (!testFont) {
        printf("ERREUR: Impossible de charger la police arial.ttf\n");
    }
    else {
        printf("SUCCES: Police chargee\n");
        sfFont_destroy(testFont);
    }

    while (sfRenderWindow_isOpen(window))
    {
        while (sfRenderWindow_pollEvent(window, &event))
        {
            if (event.type == sfEvtClosed)
            {
                sfRenderWindow_close(window);
            }

            if (event.type == sfEvtMouseButtonPressed && gameState == 0)
            {
                if (event.mouseButton.button == sfMouseLeft)
                {
                    sfVector2i mousePos = { event.mouseButton.x, event.mouseButton.y };
                    sfVector2i cellPos = {
                        mousePos.x / 30,  
                        mousePos.y / 30
                    };
                    if (cellPos.x >= 0 && cellPos.x < GRID_SIZE &&
                        cellPos.y >= 0 && cellPos.y < GRID_SIZE)
                    {
                        if (bFirstTouch) {
                            GridPlantBomb(grid, BOMB_COUNT, cellPos);
                            bFirstTouch = false;
                        }

                        int result = CellReveal(grid, cellPos);
                        if (result == FAILURE) {
                            printf("Perdu ! Une bombe a exploser !\n");
                            gameState = FAILURE;
                        }
                        else if (result == SUCCESS) {
                            printf("Bravo, c'est une victoire !\n");
                            gameState = SUCCESS;
                        }
                    }
                }
                else if (event.mouseButton.button == sfMouseRight)
                {
                    sfVector2i mousePos = { event.mouseButton.x, event.mouseButton.y };
                    sfVector2i cellPos = {
                        mousePos.x / 30,
                        mousePos.y / 30
                    };

                    if (cellPos.x >= 0 && cellPos.x < GRID_SIZE &&
                        cellPos.y >= 0 && cellPos.y < GRID_SIZE)
                    {
                        CellFlag(grid, cellPos);
                    }
                }
            }
        }

        sfRenderWindow_clear(window, sfBlack);

        GridDraw(grid, window);

        sfRenderWindow_display(window);
    }

    GridDestroy(grid);
    sfRenderWindow_destroy(window);

    return SUCCESS;
}