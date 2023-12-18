#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#define BOARD_HEIGHT 8
#define BOARD_WIDTH 7
#define BOARD_SIZE (sizeof(int_fast8_t) * BOARD_HEIGHT * BOARD_WIDTH)

typedef struct shapes
{
    int height, width;
    struct shapes *next;
    int_fast8_t *data;
    uint64_t *mask;
} shapes;

typedef struct shapes_list
{
    struct shapes *shapes;
    struct shapes_list *next;
} shapes_list;

typedef struct solutions
{
    int_fast8_t *board;
    struct solutions *next;
} solutions;

int gcount = 0;

void create_mask(const shapes *shape, uint64_t *mask);
shapes *load_shape_from_file(const char *file_name);
void print_shape(const shapes *shape);
void print_all_shapes(shapes *shape);
void print_shape_list(shapes_list *list);
void free_shapes(shapes *shape);
void free_all_shapes(shapes *shape);
void free_list(shapes_list *list);
bool shapes_equal(const shapes *shape_1, const shapes *shape_2);
bool shape_equal_in_list(const shapes *shape, shapes *other_shapes);
bool rotate_last_90_degrees(shapes *shape);
bool mirror_shape(shapes *shape);
void add_shapes(shapes *new_shape, shapes_list *shapes_list);
bool can_place(const shapes *shape, const int x, const int y, const int_fast8_t *board);
void place(const shapes *shape, const int x, const int y, int_fast8_t *board);

void print_board(const int_fast8_t *board);
void mark_date(const int month, const int month_day, const int day, int_fast8_t *board);
void generate_board(const int month, const int month_day, const int day, int_fast8_t *board);
void save_board(solutions *solutions, const int_fast8_t *board);

void find_solutions(const shapes_list *shapes_list, const int_fast8_t *board, solutions *solutions);

int main(int argc, char *argv[])
{
    int month;
    int month_day;
    int week_day;
    if (argc > 3)
    {
        month = atoi(argv[1]);
        month_day = atoi(argv[2]);
        week_day = atoi(argv[3]);
    }
    else
    {
        perror("Usage: ./solver month day week_day");
        exit(1);
    }

    shapes_list *slist = (shapes_list *)malloc(sizeof(shapes_list));
    slist->next = NULL;
    slist->shapes = NULL;

    char filename[20];
    for (int i = 1; i <= 10; i++)
    {
        sprintf(filename, "shapes/%d.txt", i);
        shapes *shape = load_shape_from_file(filename);
        add_shapes(shape, slist);
        printf("Loaded shape %d\n\n", i);
        print_all_shapes(shape);
    }

    // TODO : check if number of slots - 3 are filled

    solutions *sols = (solutions *)malloc(sizeof(solutions));
    sols->next = NULL;
    sols->board = NULL;

    int_fast8_t board[BOARD_HEIGHT * BOARD_WIDTH] = {0};
    generate_board(month, month_day, week_day, board);
    puts("=== Initial Board ===");
    print_board(board);

    clock_t start = clock();
    find_solutions(slist, board, sols);
    clock_t end = clock();

    double cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Found %d solutions in %f seconds.\n", gcount, cpu_time_used);

    int counter_sols = 0;
    while (sols != NULL)
    {
        counter_sols++;
        printf("-----Solution %d-----\n", counter_sols);
        print_board(sols->board);
        printf("\n");
        sols = sols->next;
    }

    // Free list
    free_list(slist);
}

void create_mask(const shapes *shape, uint64_t *mask)
{
    for (int i = 0; i < shape->height; i++)
    {
        for (int j = 0; j < shape->width; j++)
        {
            if (shape->data[i * shape->width + j] != 0)
            {
                *mask |= (1ULL << (i * shape->width + j));
            }
        }
    }
}

shapes *load_shape_from_file(const char *file_name)
{
    shapes *shape = (shapes *)malloc(sizeof(shapes));

    FILE *fp;
    int i;

    fp = fopen(file_name, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Erreur : Impossible d'ouvrir le fichier %s\n", file_name);
        exit(1);
    }
    shape->next = NULL;
    shape->height = 0;
    shape->width = 0;
    fscanf(fp, "%d", &shape->height);
    fscanf(fp, "%d", &shape->width);

    shape->data = (int_fast8_t *)malloc(shape->height * shape->width * sizeof(int_fast8_t));
    if (shape->data == NULL)
    {
        fprintf(stderr, "Erreur : Impossible d'allouer de la mémoire\n");
        exit(1);
    }

    for (i = 0; i < shape->height * shape->width; i++)
    {
        fscanf(fp, "%d", &shape->data[i]);
    }

    fclose(fp);

    uint64_t *mask = (uint64_t *)malloc(sizeof(uint64_t));
    create_mask(shape, mask);
    shape->mask = mask;

    return shape;
}

void print_shape(const shapes *shape)
{
    for (int i = 0; i < shape->height; i++)
    {
        for (int j = 0; j < shape->width; j++)
        {
            printf("%d ", shape->data[i * shape->width + j]);
        }
        printf("\n");
    }
}

void print_all_shapes(shapes *shape)
{
    shapes *shapes_temp = shape;
    while (shapes_temp != NULL)
    {
        print_shape(shapes_temp);
        puts(" ");
        shapes_temp = shapes_temp->next;
    }
}

void print_shape_list(shapes_list *list)
{
    shapes_list *shapes_list_temp = list;
    while (shapes_list_temp != NULL)
    {
        print_all_shapes(shapes_list_temp->shapes);
        shapes_list_temp = shapes_list_temp->next;
    }
}

void free_shapes(shapes *shape)
{
    free(shape->data);
    free(shape);
}

void free_all_shapes(shapes *shape)
{
    shapes *temp;
    while (shape != NULL)
    {
        temp = shape;
        shape = shape->next;
        free_shapes(temp);
    }
}

void free_list(shapes_list *list)
{
    shapes_list *temp;
    while (list != NULL)
    {
        temp = list;
        list = list->next;
        free_all_shapes(temp->shapes);
        free(temp);
    }
}

bool shapes_equal(const shapes *shape_1, const shapes *shape_2)
{
    // Start by comparing the sizes
    if ((shape_1->width != shape_2->width) || (shape_1->height != shape_2->height))
        return false;

    // Then compare the contents
    for (int y = 0; y < shape_1->height; y++)
    {
        int i = y * shape_1->width;
        for (int x = 0; x < shape_1->width; x++)
        {
            if (shape_1->data[i + x] != shape_2->data[i + x])
                return false;
        }
    }

    return true;
}

bool shape_equal_in_list(const shapes *shape, shapes *other_shapes)
{
    shapes *temp = other_shapes;
    while (temp != NULL)
    {
        if (shapes_equal(shape, temp))
        {
            return true;
        }

        temp = temp->next;
    }

    return false;
}

bool rotate_last_90_degrees(shapes *shape)
{
    shapes *current = shape;
    while (current->next != NULL)
    {
        current = current->next;
    }

    // Declare new shape
    shapes *new_shape = (shapes *)malloc(sizeof(shapes));

    new_shape->width = current->height;
    new_shape->height = current->width;

    // Create array for new data and manually allocate memory
    int_fast8_t *n_data = (int_fast8_t *)malloc(sizeof(int_fast8_t) * new_shape->width * new_shape->height);

    // Loop over shape and rotate it 90deg by swapping x and y when getting from original shape
    for (int y = 0; y < new_shape->height; y++)
    {
        int i = y * new_shape->width;
        for (int x = 0; x < new_shape->width; x++)
            n_data[i + x] = current->data[(current->height - x - 1) * current->width + y];
    }

    // Add array to shape
    new_shape->data = n_data;

    uint64_t *mask = (uint64_t *)malloc(sizeof(uint64_t));
    create_mask(new_shape, mask);
    new_shape->mask = mask;

    // Check if shape already in shapes array
    if (shape_equal_in_list(new_shape, shape))
    {
        free_shapes(new_shape);
        return true;
    }

    new_shape->next = NULL;

    shapes *temp = shape;

    while (temp->next != NULL)
    {
        temp = temp->next;
    }

    temp->next = new_shape;

    return false;
}

bool mirror_shape(shapes *shape)
{
    // Declare new shape
    shapes *new_shape = malloc(sizeof(shapes));

    new_shape->width = shape->width;
    new_shape->height = shape->height;

    // Create array for new data and manually allocate memory
    int_fast8_t *n_data = (int_fast8_t *)malloc(sizeof(int_fast8_t) * new_shape->width * new_shape->height);
    // Loop over shape and rotate it 90deg by swapping x and y when getting from original shape
    for (int y = 0; y < new_shape->height; y++)
    {
        int i = y * new_shape->width;
        for (int x = 0; x < new_shape->width; x++)
        {
            int j = i + x;
            n_data[j] = shape->data[shape->width - x - 1 + i];
        }
    }

    // Add array to shape
    new_shape->data = n_data;

    uint64_t *mask = (uint64_t *)malloc(sizeof(uint64_t));
    create_mask(new_shape, mask);
    new_shape->mask = mask;

    // Check if shape is not symetric to the shape created
    if (shape_equal_in_list(new_shape, shape))
    {
        free_shapes(new_shape);
        return true;
    }

    new_shape->next = NULL;

    shapes *temp = shape;

    while (temp->next != NULL)
    {
        temp = temp->next;
    }

    temp->next = new_shape;

    return false;
}

void add_shapes(shapes *new_shape, shapes_list *all_shapes)
{
    // For each rotation create a shape and make it from the previous shape,
    // then check it isn't equal to other shapes in list

    bool isdone = rotate_last_90_degrees(new_shape); // 90deg rotations
    if (!isdone)
        isdone = rotate_last_90_degrees(new_shape); // 180deg rotations
    if (!isdone)
        isdone = rotate_last_90_degrees(new_shape); // 270deg rotations

    // isdone = mirror_shape(new_shape); //Flip

    // if(!isdone)
    //     isdone = rotate_last_90_degrees(new_shape); //90deg rotations of flip
    // if (!isdone)
    //     isdone = rotate_last_90_degrees(new_shape); //180deg rotations of flip
    // if (!isdone)
    //     isdone = rotate_last_90_degrees(new_shape); //270deg rotations of flip

    if ((all_shapes)->shapes == NULL)
    {
        (all_shapes)->shapes = new_shape;
        all_shapes->next = NULL;
        return;
    }

    shapes_list *new_list = (shapes_list *)malloc(sizeof(shapes_list));

    // new_list->next = *all_shapes;
    // new_list->shapes = new_shape;

    // *all_shapes = new_list;

    new_list->next = NULL;
    new_list->shapes = new_shape;

    shapes_list *current = all_shapes;
    while (current->next != NULL)
    {
        current = current->next;
    }

    current->next = new_list;
}

bool can_place(const shapes *shape, const int x, const int y, const int_fast8_t *board)
{
    // Check if the shape is out of bounds
    if (y + shape->height > BOARD_HEIGHT || x + shape->width > BOARD_WIDTH)
        return false;

    // For each marker of the shape, we check if there is a collision
    int iw;
    for (int i = 0; i < shape->height; i++)
    {
        iw = i * shape->width;
        for (int j = 0; j < shape->width; j++)
        {
            if (shape->data[iw + j] == 0)
                continue;
            // There is a collision, we stop
            if (board[(i + y) * BOARD_WIDTH + j + x] != 0)
                return false;
        }
    }

    return true;
}

void place(const shapes *shape, const int x, const int y, int_fast8_t *board)
{
    int iw;
    for (int i = 0; i < shape->height; i++)
    {
        iw = i * shape->width;
        for (int j = 0; j < shape->width; j++)
        {
            if (shape->data[iw + j] == 0)
                continue;
            board[(i + y) * BOARD_WIDTH + j + x] = shape->data[iw + j];
        }
    }
}

void print_board(const int_fast8_t *board)
{
    for (int i = 0; i < BOARD_HEIGHT; i++)
    {
        for (int j = 0; j < BOARD_WIDTH; j++)
        {
            printf("%d\t", board[i * BOARD_WIDTH + j]);
        }
        printf("\n");
    }
}

void mark_date(const int month, const int month_day, const int day, int_fast8_t *board)
{
    // -1 on the board corresponds to a block (not placeable slot)
    // Defining the month position (0 1 2 3 4 5 - 6 7 8 9 10 11)
    if (month <= 5)
        board[month] = -1;
    else
        board[month + 1] = -1;

    // Defining the month-day position (months + 2)
    board[month_day + 14] = -1;

    // Defining the day (months + 2 + days + 0 1 2 3 + 4 + 4 5 6)
    if (day <= 3)
        board[day + 45] = -1;
    else
        board[day + 49] = -1;
}

void generate_board(const int month, const int month_day, const int day, int_fast8_t *board)
{
    // First blocks the date slots
    mark_date(month, month_day, day, board);

    // Blocks the border slots
    board[6] = -1;
    board[13] = -1;
    board[49] = -1;
    board[50] = -1;
    board[51] = -1;
    board[52] = -1;
}

void save_board(solutions *solutions, const int_fast8_t *board)
{
    int_fast8_t *new_board = (int_fast8_t *)malloc(BOARD_SIZE);
    memcpy(new_board, board, BOARD_SIZE);

    if (solutions->board == NULL)
    {
        solutions->board = new_board;
        return;
    }

    struct solutions *new_solution = (struct solutions *)malloc(sizeof(struct solutions));
    new_solution->board = new_board;
    new_solution->next = NULL;

    struct solutions *temp = solutions;

    while (temp->next != NULL)
    {
        temp = temp->next;
    }

    temp->next = new_solution;
}

void find_solutions(const shapes_list *shapes_list, const int_fast8_t *board, solutions *solutions)
{
    if (shapes_list == NULL)
    {
        save_board(solutions, board);
        gcount++;
        return;
    }

    shapes *current_shape = shapes_list->shapes;

    // For each shape variant
    while (current_shape != NULL)
    {
        // For each slots
        for (int y = 0; y < BOARD_HEIGHT; y++)
        {
            for (int x = 0; x < BOARD_WIDTH; x++)
            {
                if (can_place(current_shape, x, y, board))
                {
                    // Copy board
                    int_fast8_t *new_board = (int_fast8_t *)malloc(BOARD_SIZE);
                    memcpy(new_board, board, BOARD_SIZE);
                    // Place piece on board
                    place(current_shape, x, y, new_board);

                    find_solutions(shapes_list->next, new_board, solutions);
                    free(new_board);
                }
            }
        }

        current_shape = current_shape->next;
    }
}