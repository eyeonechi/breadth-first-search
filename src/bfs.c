/***************************************************************************/

/* Program concept and description :                                       *
 * Breadth First Search implementation of maze traversing.                 *
 * Uses a queue (linked-list) with which each node stores a parent pointer *
 * to enable the queue to also perform as a cost-leveled tree, with maze   *
 * entrances as the roots. The exit with the lowest cost is determined     *
 * after all cells have been traversed and the shortest leftmost path is   *
 * constructed via following parent pointers back to the root.             *
 * Nodes are initialised with cost -1 as an indication of not visited      */

/***************************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Maximum maze size */
#define MAX_ROWS 100
#define MAX_COLS 100

/* Cell character types */
#define NEWLINE     '\n'
#define WALL        '#'
#define PATH        '.'
#define REACHABLE   '+'
#define UNREACHABLE '-'
#define NONSOLUTION ' '

/* Miscellaneous Constants */
#define NOTVISIT  - 1
#define TRUE        1
#define FALSE       0
#define NIL         0
#define LAST_ROW    maze->rows - 1
#define LAST_COL    maze->cols - 1

/* Stage numbers */
#define STAGE1 1
#define STAGE2 2
#define STAGE3 3
#define STAGE4 4

/* Printing related constants */
#define STAGENUM "Stage %d\n=======\n"
#define PRINT1   "maze has %d rows and %d columns\n"
#define PRINT2A  "maze has a solution\n"
#define PRINT2B  "maze has no solution\n"
#define PRINT3A  "maze has solution with cost %d\n"
#define PRINT3B  "maze has no solution\n"
#define PRINT4   "maze solution\n"
#define ONEDIGIT "0%1d"
#define TWODIGIT "%2d"
#define ONECHAR  "%c"
#define TWOCHAR  "%c%c"

/***************************************************************************/

/* Structure naming convention */
typedef struct maze_s maze_t;
typedef struct cell_s cell_t;
typedef struct list_s list_t;

/* Cell structure */
struct cell_s {
	int  x;         /* x-coordinate               */
	int  y;         /* y-coordinate               */
	int  cost;      /* Cost from nearest entrance */
	int  reach;     /* Reachability of cell       */
	int  soln;      /* Part of shortest path      */
	char type;      /* Cell visualisation         */
};

/* Maze structure */
struct maze_s {
	int      rows;  /* Number of rows             */
	int      cols;  /* Number of columns          */
	int      cost;  /* Lowest cost of solution    */
	int      soln;  /* Maze has a solution        */
	cell_t **cells; /* 2D Array of cells          */
};

/* List structure */
struct list_s {
	cell_t *cell;   /* Corresponding cell in maze */
	list_t *parent; /* Parent node                */
	list_t *next;   /* Next node in the list      */
};

/***************************************************************************/

/* Function prototypes */
maze_t *new_maze();
void    new_row(cell_t **cells, int lim);
list_t *new_list();
list_t *prepend(list_t *list, cell_t *cell);
void    append(list_t *list, list_t *parent, cell_t *cell);
maze_t *read_maze(maze_t *maze);
int     read_row(maze_t *maze, char c, int x, int lim);
int     read_cell(cell_t *cell, char c, int x, int y, int lim);
maze_t *print_maze(maze_t *maze);
void    print_stage_1(cell_t **cell, int x, int y, int xlim, int ylim);
void    print_stage_2(cell_t **cell, int x, int y, int xlim, int ylim);
void    print_stage_3(cell_t **cell, int x, int y, int xlim, int ylim);
void    print_stage_4(cell_t **cell, int x, int y, int xlim, int ylim);
maze_t *traverse_maze(maze_t *maze);
list_t *find_entries(cell_t *cell, list_t *queue, int lim);
cell_t *find_exit(cell_t *cell, cell_t *exit, int cost, int lim);
void    recursive_flood(maze_t *maze, list_t *queue);
void    flood_up(maze_t *maze, list_t *queue, int x, int y, int cost);
void    flood_down(maze_t *maze, list_t *queue, int x, int y, int cost);
void    flood_left(maze_t *maze, list_t *queue, int x, int y, int cost);
void    flood_right(maze_t *maze, list_t *queue, int x, int y, int cost);
void    visit_cell(maze_t *maze, list_t *queue, int x, int y, int cost);
int     shortest_path(maze_t *maze, list_t *queue, cell_t *exit);
void    free_list(list_t *list);
int     free_maze(maze_t *maze);
void    free_cells(cell_t **cells, int lim);

/***************************************************************************/

/* Handles processing of the maze (Read from right to left) */
int main(int argc, char **argv) {
	return free_maze(print_maze(traverse_maze(read_maze(new_maze()))));
}

/***************************************************************************/

/* Allocates memory for a maze_t struct */
maze_t *new_maze() {
	maze_t *maze = (maze_t *)calloc(sizeof(*maze), sizeof(*maze));
	assert(maze);
	maze->cells = (cell_t **)malloc(MAX_ROWS * sizeof(*(maze->cells)));
	assert(maze->cells);
	new_row(maze->cells, MAX_ROWS);
	return maze;
}

/**^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^**/

/* Recursively allocates 2D array of cells */
void new_row(cell_t **cells, int lim) {
	if (lim && cells) {
		*cells = (cell_t *)calloc(sizeof(**cells), MAX_COLS * sizeof(**cells));
		assert(*cells);
		new_row(cells + 1, lim - 1);
	}
}

/***************************************************************************/

/* Creates a new list node instance */
list_t *new_list() {
	list_t *list = (list_t *)calloc(sizeof(*list), sizeof(*list));
	assert(list);
	return list;
}

/**^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^**/

/* Append a new node to the end of a list */
void append(list_t *list, list_t *parent, cell_t *cell) {
	if (list->next) { 
		append(list->next, parent, cell);
	} else {
		list->next = new_list();
		list->next->parent = parent;
		list->next->cell = cell;
	}
}

/**^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^**/

/* Prepends a new node as the head of the list */
list_t *prepend(list_t *list, cell_t *cell) {
	list_t *head = new_list();
	head->cell = cell;
	head->next = list;
	return head;
}

/***************************************************************************/

/* Reads in a maze from a text file */
maze_t *read_maze(maze_t *maze) {
	char c;
	if (scanf(ONECHAR, &c) != EOF && c != NEWLINE) {
		/* Number of rows read */
		maze->rows = read_row(maze, c, NIL, MAX_ROWS);
	}
	return maze;
}

/**^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^**/

/* Recursively reads cell rows */
int read_row(maze_t *maze, char c, int x, int lim) {
	if (lim) {
		/* Number of columns read */
		maze->cols = read_cell(maze->cells[x], c, x, NIL, MAX_COLS);
		if (scanf(ONECHAR, &c) != EOF) {
			return 1 + read_row(maze, c, x + 1, lim - 1);
		}
	}
	return 1;
}

/**^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^**/

/* Recursively reads individual cells as columns */
int read_cell(cell_t *cell, char c, int x, int y, int lim) {
	if (lim && c != NEWLINE) {
		cell[y].x = x;
		cell[y].y = y;
		cell[y].type = c;
		cell[y].cost = NOTVISIT;
		if (scanf(ONECHAR, &c) && c != NEWLINE) {
			return 1 + read_cell(cell, c, x, y + 1, lim - 1);
		}
	}
	return 1;
}

/***************************************************************************/

/* Traverses the maze using breadth first search */
maze_t *traverse_maze(maze_t *maze) {
	cell_t *ex;
	list_t *queue = find_entries(*(maze->cells), NULL, maze->cols);
	recursive_flood(maze, queue);
	if ((ex = find_exit(maze->cells[LAST_ROW], NULL, NOTVISIT, maze->cols))) {
		maze->cost = shortest_path(maze, queue, ex);
		maze->soln = TRUE;
	}
	free_list(queue);
	return maze;
}

/**^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^**/

/* Recursively enqueues maze entrances */
list_t *find_entries(cell_t *cell, list_t *queue, int lim) {
	if (lim) {
		if (cell->type == PATH) {
			cell->reach = TRUE;
			cell->cost = FALSE;
			if (!queue) {
				queue = prepend(queue, cell);
			} else {
				append(queue, NULL, cell);
			}
		}
		return find_entries(cell + 1, queue, lim - 1);
	}
	return queue;
}

/**^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^**/

/* Breadth first search algorithm of 'flooding' the maze with water */
void recursive_flood(maze_t *maze, list_t *queue) {
	if (queue) {
		int x = queue->cell->x, y = queue->cell->y, cost = queue->cell->cost;
		flood_right(maze, queue, x, y + 1, cost + 1);
		flood_down(maze, queue, x + 1, y, cost + 1);
		flood_left(maze, queue, x, y - 1, cost + 1);
		flood_up(maze, queue, x - 1, y, cost + 1);
		recursive_flood(maze, queue->next);
	}
}

/**-----------------------------------------------------------------------**/

/* Case 1 : Water travels upwards */
void flood_up(maze_t *maze, list_t *queue, int x, int y, int cost) {
	if (x >= NIL && maze->cells[x][y].type == PATH) {
		visit_cell(maze, queue, x, y, cost);
	}
}

/**-----------------------------------------------------------------------**/

/* Case 2 : Water travels downwards */
void flood_down(maze_t *maze, list_t *queue, int x, int y, int cost) {
	if (x < maze->rows && maze->cells[x][y].type == PATH) {
		visit_cell(maze, queue, x, y, cost);
	}
}

/**-----------------------------------------------------------------------**/

/* Case 3 : Water travels to the left */
void flood_left(maze_t *maze, list_t *queue, int x, int y, int cost) {
	if (y >= NIL && maze->cells[x][y].type == PATH) {
		visit_cell(maze, queue, x, y, cost);
	}
}

/**-----------------------------------------------------------------------**/

/* Case 4 : Water travels to the right */
void flood_right(maze_t *maze, list_t *queue, int x, int y, int cost) {
	if (y < maze->cols && maze->cells[x][y].type == PATH) {
		visit_cell(maze, queue, x, y, cost);
	}
}

/**^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^**/

/* Assigns reachability and determines enqueuing of cell */
void visit_cell(maze_t *maze, list_t *queue, int x, int y, int cost) {
	maze->cells[x][y].reach = TRUE;
	if (maze->cells[x][y].cost < NIL || cost < maze->cells[x][y].cost) {
		maze->cells[x][y].cost = cost;
		append(queue, queue, &maze->cells[x][y]);
	}
}

/**^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^**/

/* Determines the lowest-costing exit of the maze, if any */
cell_t *find_exit(cell_t *cell, cell_t *exit, int cost, int lim) {
	if (lim) {
		if (cell->type == PATH && cell->reach) {
			if (cost < NIL || (cell->cost >= NIL && cell->cost < cost)) {
				return find_exit(cell + 1, cell, cell->cost, lim - 1);
			}
		}
		return find_exit(cell + 1, exit, cost, lim - 1);
	}
	return exit;
}

/**^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^**/

/* Recursively looks for the shortest exit in the queue and backtracks *
 * through the tree via node parents                                   */
int shortest_path(maze_t *maze, list_t *queue, cell_t *exit) {
	if (queue) {
		if (!exit || queue->cell == exit) {
			queue->cell->soln = TRUE;
			return 1 + shortest_path(maze, queue->parent, NULL);
		}
		return shortest_path(maze, queue->next, exit);
	}
	return - 1;
}

/***************************************************************************/

/* Handles maze output printing */
maze_t *print_maze(maze_t *maze) {
	printf(STAGENUM, STAGE1);
	printf(PRINT1, maze->rows, maze->cols);
	print_stage_1(maze->cells, NIL, NIL, maze->rows, maze->cols);
	printf(ONECHAR, NEWLINE);
	printf(STAGENUM, STAGE2);
	if (maze->soln) {
		printf(PRINT2A);
		print_stage_2(maze->cells, NIL, NIL, maze->rows, maze->cols);
		printf(ONECHAR, NEWLINE);
		printf(STAGENUM, STAGE3);
		printf(PRINT3A, maze->cost);
		print_stage_3(maze->cells, NIL, NIL, maze->rows, maze->cols);
		printf(ONECHAR, NEWLINE);
		printf(STAGENUM, STAGE4);
		printf(PRINT4);
		print_stage_4(maze->cells, NIL, NIL, maze->rows, maze->cols);
	} else {
		printf(PRINT2B);
		print_stage_2(maze->cells, NIL, NIL, maze->rows, maze->cols);
		printf(ONECHAR, NEWLINE);
		printf(STAGENUM, STAGE3);
		printf(PRINT3B);
		print_stage_3(maze->cells, NIL, NIL, maze->rows, maze->cols);
	}
	return maze;
}

/**^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^**/

/* Prints output of Stage 1 */
void print_stage_1(cell_t **cell, int x, int y, int xlim, int ylim) {
	if (xlim) {
		if (ylim) {
			printf(TWOCHAR, (*cell)[y].type, (*cell)[y].type);
			print_stage_1(cell, x, y + 1, xlim, ylim - 1);
		} else {
			printf(ONECHAR, NEWLINE);
			print_stage_1(cell + 1, x + 1, NIL, xlim - 1, y);
		}
	}
}

/**^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^**/

/* Prints output of Stage 2 */
void print_stage_2(cell_t **cell, int x, int y, int xlim, int ylim) {
	if (xlim) {
		if (ylim) {
			if ((*cell)[y].type == PATH) {
				if ((*cell)[y].reach) {
					printf(TWOCHAR, REACHABLE, REACHABLE);
				} else {
					printf(TWOCHAR, UNREACHABLE, UNREACHABLE);
				}
			} else {
				printf(TWOCHAR, (*cell)[y].type, (*cell)[y].type);
			}
			print_stage_2(cell, x, y + 1, xlim, ylim - 1);
		} else {
			printf(ONECHAR, NEWLINE);
			print_stage_2(cell + 1, x + 1, NIL, xlim - 1, y);
		}
	}
}

/**^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^**/

/* Prints output of Stage 3 */
void print_stage_3(cell_t **cell, int x, int y, int xlim, int ylim) {
	if (xlim) {
		if (ylim) {
			if ((*cell)[y].type == PATH) {
				if ((*cell)[y].reach) {
					if (!((*cell)[y].cost % 2)) {
						if (((*cell)[y].cost % 100) > 9) {
							printf(TWODIGIT, (*cell)[y].cost % 100);
						} else {
							printf(ONEDIGIT, (*cell)[y].cost % 100);
						}
					} else {
						printf(TWOCHAR, REACHABLE, REACHABLE);
					}
				} else {
					printf(TWOCHAR, UNREACHABLE, UNREACHABLE);
				}
			} else {
				printf(TWOCHAR, (*cell)[y].type, (*cell)[y].type);
			}
			print_stage_3(cell, x, y + 1, xlim, ylim - 1);
		} else {
			printf(ONECHAR, NEWLINE);
			print_stage_3(cell + 1, x + 1, NIL, xlim - 1, y);
		}
	}
}

/**^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^**/

/* Prints output of Stage 4 */
void print_stage_4(cell_t **cell, int x, int y, int xlim, int ylim) {
	if (xlim) {
		if (ylim) {
			if ((*cell)[y].type == PATH) {
				if ((*cell)[y].reach) {
					if ((*cell)[y].soln) {	
						if (!((*cell)[y].cost % 2)) {
							if (((*cell)[y].cost % 100) > 9) {
								printf(TWODIGIT, (*cell)[y].cost % 100);
							} else {
								printf(ONEDIGIT, (*cell)[y].cost % 100);
							}
						} else {
							printf(TWOCHAR, PATH, PATH);
						}
					} else {
						printf(TWOCHAR, NONSOLUTION, NONSOLUTION);
					}
				} else {
					printf(TWOCHAR, UNREACHABLE, UNREACHABLE);
				}
			} else {
				printf(TWOCHAR, (*cell)[y].type, (*cell)[y].type);
			}
			print_stage_4(cell, x, y + 1, xlim, ylim - 1);
		} else {
			printf(ONECHAR, NEWLINE);
			print_stage_4(cell + 1, x + 1, NIL, xlim - 1, y);
		}
	}
}

/***************************************************************************/

/* Frees memory allocated to a list */
void free_list(list_t *list) {
	if (list) {
		free_list(list->next);
		free(list);
	}
}

/**^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^**/

/* Frees memory allocated to a maze */
int free_maze(maze_t *maze) {
	free_cells(maze->cells, MAX_ROWS);
	free(maze->cells);
	free(maze);
	return EXIT_SUCCESS;
}

/**^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^**/

/* Frees memory allocated to a 2D array of cells */
void free_cells(cell_t **cells, int lim) {
	if (lim && cells) {
		free_cells(cells + 1, lim - 1);
		free(*cells);
	}
}

/***************************************************************************/
