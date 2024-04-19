#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <Windows.h>

#define KIT_IMPL
#include "kit.h"

#define app_name "Tents & Trees v1.0.0"

enum
{
    EMPTY,
    GRASS,
    TENT,
    TREE,
};

typedef struct
{
    int x, y;
    int rows, cols;
    int gap, cellsz;
    int *cells;    // This will be used for the player's grid.
    int *solution; // This will store the complete solution with trees and tents.
    int *rowTents; // Number of tents in each row (calculated from the solution).
    int *colTents; // Number of tents in each column (calculated from the solution).
} grid_t;

grid_t *grid_new(int rows, int cols, int cellsz, int x, int y)
{
    int *cells = calloc(rows * cols, sizeof(int));
    int *solution = calloc(rows * cols, sizeof(int)); // Initialize the solution grid
    int *rowTents = calloc(rows, sizeof(int));        // Array for row tent requirements
    int *colTents = calloc(cols, sizeof(int));        // Array for column tent requirements

    grid_t *grid = (grid_t *)malloc(sizeof(grid_t));
    if (!grid || !cells || !solution || !rowTents || !colTents)
    {
        // Handle allocation failures; clean up and return NULL if any allocation fails
        free(cells);
        free(solution);
        free(rowTents);
        free(colTents);
        free(grid);
        return NULL;
    }

    grid->gap = 2;
    grid->x = x;
    grid->y = y;
    grid->rows = rows;
    grid->cols = cols;
    grid->cellsz = cellsz;
    grid->cells = cells;
    grid->solution = solution;
    grid->rowTents = rowTents;
    grid->colTents = colTents;
    return grid;
}

void grid_free(grid_t *grid)
{
    if (grid != NULL)
    {
        free(grid->cells);
        free(grid->solution);
        free(grid->rowTents);
        free(grid->colTents);
        free(grid);
    }
}

void grid_update(kit_Context *ctx, grid_t *grid)
{
    int mousex = 0;
    int mousey = 0;
    kit_mouse_pos(ctx, &mousex, &mousey);

    for (int y = 0; y < grid->rows; y++)
    {
        for (int x = 0; x < grid->cols; x++)
        {
            int cell_start_x = x * (grid->cellsz + grid->gap) + grid->x;
            int cell_start_y = y * (grid->cellsz + grid->gap) + grid->y;
            int cell_end_x = cell_start_x + grid->cellsz;
            int cell_end_y = cell_start_y + grid->cellsz;

            if (mousex >= cell_start_x && mousex <= cell_end_x && mousey >= cell_start_y && mousey <= cell_end_y)
            {
                if (kit_mouse_down(ctx, VK_LBUTTON))
                {
                    int currentValue = grid->cells[y * grid->cols + x];
                    if (currentValue == TREE)
                        return;
                    grid->cells[y * grid->cols + x] = TENT;
                }
                else if (kit_mouse_down(ctx, VK_RBUTTON))
                {
                    int currentValue = grid->cells[y * grid->cols + x];
                    if (currentValue == TREE)
                        return;
                    grid->cells[y * grid->cols + x] = GRASS;
                }
                else if (kit_mouse_down(ctx, 0x03))
                {
                    int currentValue = grid->cells[y * grid->cols + x];
                    if (currentValue == TREE)
                        return;
                    grid->cells[y * grid->cols + x] = EMPTY;
                }
            }
        }
    }
}

void grid_draw(kit_Context *ctx, grid_t *grid, bool showSolutionGrid)
{
    int *cells = showSolutionGrid ? grid->solution : grid->cells;

    for (int y = 0; y < grid->rows; y++)
    {
        for (int x = 0; x < grid->cols; x++)
        {
            kit_Color color;
            switch (cells[y * grid->cols + x])
            {
            case EMPTY:
                color = kit_rgb(255, 255, 255);
                break;
            case TENT:
                color = kit_rgb(100, 100, 255);
                break;
            case GRASS:
                color = kit_rgb(140, 255, 140);
                break;
            case TREE:
                color = kit_rgb(70, 180, 70);
                break;
            }
            kit_draw_rect(ctx, color, (kit_Rect){x * grid->cellsz + grid->gap * x + grid->x, y * grid->cellsz + grid->gap * y + grid->y, grid->cellsz, grid->cellsz});
        }
    }

    // Draw row and column tent requirements
    for (int y = 0; y < grid->rows; y++)
    {
        char rowReq[10];
        sprintf(rowReq, "%d", grid->rowTents[y]);
        kit_draw_text(ctx, KIT_WHITE, rowReq, grid->x + grid->cols * (grid->cellsz + grid->gap), y * (grid->cellsz + grid->gap) + grid->y);
    }
    for (int x = 0; x < grid->cols; x++)
    {
        char colReq[10];
        sprintf(colReq, "%d", grid->colTents[x]);
        kit_draw_text(ctx, KIT_WHITE, colReq, x * (grid->cellsz + grid->gap) + grid->x + 4, grid->y + grid->rows * (grid->cellsz + grid->gap));
    }
}

bool can_place_tent(const int *grid, int rows, int cols, int row, int col)
{
    // Check if the cell is within grid boundaries
    if (row < 0 || row >= rows || col < 0 || col >= cols)
    {
        return false;
    }

    // Check if the cell is empty and can host a tent
    if (grid[row * cols + col] != EMPTY)
    {
        return false;
    }

    // Check all adjacent cells (8 directions: N, NE, E, SE, S, SW, W, NW)
    for (int dr = -1; dr <= 1; dr++)
    {
        for (int dc = -1; dc <= 1; dc++)
        {
            int adjRow = row + dr;
            int adjCol = col + dc;
            // Ensure adjacent cell is within grid boundaries
            if (adjRow >= 0 && adjRow < rows && adjCol >= 0 && adjCol < cols)
            {
                // Check if there's a tent in any of the adjacent cells
                if (grid[adjRow * cols + adjCol] == TENT)
                {
                    return false;
                }
            }
        }
    }

    return true; // Valid position for a tent
}

void shuffle(int array[4][2], int n)
{
    for (int i = n - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);

        // Swap the current element with a randomly selected one.
        int tempRow = array[i][0];
        int tempCol = array[i][1];
        array[i][0] = array[j][0];
        array[i][1] = array[j][1];
        array[j][0] = tempRow;
        array[j][1] = tempCol;
    }
}

void init_game(grid_t *grid, int treeCount)
{
    int maxAttempts = grid->rows * grid->cols * 10; // Set a reasonable cap on attempts to prevent infinite loops
    int placedTrees = 0;
    bool fillMax = (treeCount == 0);

    if (fillMax)
        treeCount = grid->rows * grid->cols / 2; // Estimate the upper limit of trees that might fit

    int attempts = 0;
    while (placedTrees < treeCount && attempts < maxAttempts)
    {
        int r = rand() % grid->rows;
        int c = rand() % grid->cols;

        // Check if current spot is available for a tree
        if (grid->solution[r * grid->cols + c] == EMPTY)
        {
            // List potential tent positions
            int positions[4][2] = {{r - 1, c}, {r + 1, c}, {r, c - 1}, {r, c + 1}};
            shuffle(positions, 4); // Randomize tent positions

            bool tentPlaced = false;
            for (int j = 0; j < 4; j++)
            {
                int tr = positions[j][0], tc = positions[j][1];
                if (tr >= 0 && tr < grid->rows && tc >= 0 && tc < grid->cols && grid->solution[tr * grid->cols + tc] == EMPTY)
                {
                    if (can_place_tent(grid->solution, grid->rows, grid->cols, tr, tc))
                    {
                        // Place tree and tent
                        grid->solution[r * grid->cols + c] = TREE;
                        grid->solution[tr * grid->cols + tc] = TENT;
                        grid->rowTents[tr]++;
                        grid->colTents[tc]++;
                        tentPlaced = true;
                        placedTrees++;
                        break;
                    }
                }
            }
            if (!tentPlaced)
            {
                attempts++;
            }
        }
        else
        {
            attempts++;
        }
    }

    // Copy trees from solution to the player's grid
    for (int i = 0; i < grid->rows * grid->cols; i++)
    {
        grid->cells[i] = (grid->solution[i] == TREE) ? TREE : EMPTY;
        // grid->cells[i] = grid->solution[i];
    }
}

bool check_solution(const grid_t *grid)
{
    for (int i = 0; i < grid->rows * grid->cols; i++)
    {
        if ((grid->cells[i] == TENT && grid->solution[i] != TENT) ||
            (grid->cells[i] != TENT && grid->solution[i] == TENT))
        {
            return false; // Mismatch found
        }
    }
    return true; // All tents correctly placed
}

int main()
{
    srand((unsigned)time(NULL));

    int gridx = 30;
    int gridy = 35;
    int cellsz = 15;
    int rows = 8;
    int cols = 8;
    int screenw = 200;
    int screenh = 200;
    int appNameW = 0;
    double dt;

    kit_Context *ctx = kit_create(app_name, screenw, screenh, KIT_SCALE2X);

    appNameW = kit_text_width(ctx->font, app_name);

    grid_t *grid = grid_new(rows, cols, cellsz, gridx, gridy);

    // Timer variables
    clock_t startTime, endTime;
    double elapsed;

    bool firstStart = true;

new_game:

    startTime = clock();

    grid_free(grid);
    grid = grid_new(rows, cols, cellsz, gridx, gridy);

    init_game(grid, 0);

    bool solutionPeeked = false;
    bool showSolution = false;
    bool solved = false;

    while (kit_step(ctx, &dt))
    {
        kit_clear(ctx, kit_rgba(0, 0, 0, 5));

        kit_draw_text(ctx, KIT_WHITE, app_name, (screenw / 2) - (appNameW / 2), 10);

        if (kit_key_pressed(ctx, VK_ESCAPE))
            firstStart = true;

        if (kit_key_down(ctx, VK_SHIFT))
        {
            solutionPeeked = true;
            showSolution = true;
        }
        else
        {
            showSolution = false;
        }

        grid_update(ctx, grid);
        grid_draw(ctx, grid, showSolution);

        if (check_solution(grid) || firstStart)
        {

            if (firstStart)
            {
                kit_clear(ctx, KIT_BLACK);
                firstStart = false;
            }
            else
            {
                endTime = clock();
                elapsed = (double)(endTime - startTime) / CLOCKS_PER_SEC;
                solved = true;
            }
            break;
        }
    }

    // Display Solved/Start Screen
    while (kit_step(ctx, &dt))
    {
        kit_clear(ctx, kit_rgba(0, 0, 0, 5));
        int minutes = (int)elapsed / 60;                             // Get total minutes
        int seconds = (int)elapsed % 60;                             // Get remaining seconds
        int milliseconds = (int)((elapsed - ((int)elapsed)) * 1000); // Get milliseconds

        char solvedText[100];
        sprintf(solvedText, "Solved in %dm %ds %dms!", minutes, seconds, milliseconds);
        int solvedTextW = kit_text_width(ctx->font, solvedText);

        if (solved)
            kit_draw_text(ctx, KIT_WHITE, solvedText, (screenw / 2) - (solvedTextW / 2), 70);
        else
            kit_draw_text(ctx, KIT_WHITE, app_name, (screenw / 2) - (appNameW / 2), 10);

        int spaceTextW = kit_text_width(ctx->font, "HIT SPACE FOR NEW GAME");
        kit_draw_text(ctx, KIT_WHITE, "HIT SPACE FOR NEW GAME", (screenw / 2) - (spaceTextW / 2), 100);

        int peekedTextW = kit_text_width(ctx->font, "SOLUTION PEEKED!");
        int toPeekedTextW = kit_text_width(ctx->font, "SHIFT TO PEEK SOLUTION");
        if (solutionPeeked)
        {

            kit_draw_text(ctx, KIT_WHITE, "SOLUTION PEEKED!", (screenw / 2) - (peekedTextW / 2), 130);
        }
        else if (!solved)
        {
            kit_draw_text(ctx, KIT_WHITE, "SHIFT TO SHOW SOLUTION", (screenw / 2) - (toPeekedTextW / 2), 130);
        }

        if (kit_key_pressed(ctx, VK_ESCAPE))
            exit(0);

        if (kit_key_pressed(ctx, VK_SPACE))
            goto new_game;
    }

    grid_free(grid);
    kit_destroy(ctx);
    return 0;
}
