#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#define BOARD_HEIGHT 8
#define BOARD_WIDTH 7
#define BOARD_SIZE (sizeof(uint8_t) * BOARD_HEIGHT)

typedef struct shapes
{
    int height, width;
    struct shapes *next;
    uint8_t *mask;
} shapes;

typedef struct shapes_list
{
    struct shapes *shapes;
    struct shapes_list *next;
} shapes_list;

// Checking the board and shapes
int count_free_spots(const uint8_t *mask);
int count_full_spots(const shapes *shape);

// Debugging
void print_shape(const shapes *shape);
void print_all_shapes(shapes *shape);
void print_shape_list(shapes_list *list);

// Memory freeing
void free_shapes(shapes *shape);
void free_all_shapes(shapes *shape);
void free_list(shapes_list *list);

// Shape generation
shapes *load_shape_from_file(const char *file_name); //
bool shapes_equal(const shapes *shape_1, const shapes *shape_2);
bool shape_equal_in_list(const shapes *shape, shapes *other_shapes);
bool rotate_last_90_degrees(shapes *shape);
bool mirror_shape(shapes *shape);
void add_shapes(shapes *new_shape, shapes_list *shapes_list, const bool mirror);

// Board generation
void print_board(const uint8_t *board);
void mark_date(const int month, const int month_day, const int day, uint8_t *board);
void generate_board(const int month, const int month_day, const int day, uint8_t *board);

// Solutions generation
bool new_can_place(const shapes *shape, const int x, const int y, const uint8_t *board, uint8_t *new_board);

bool exist_solution(const shapes_list *shapes_list, const uint8_t *board);

int main(int argc)
{
    // Make shapes
    shapes_list *slist = (shapes_list *)malloc(sizeof(shapes_list));
    slist->next = NULL;
    slist->shapes = NULL;

    char filename[20];
    for (int i = 1; i <= 10; i++)
    {
        sprintf(filename, "shapes/%d.txt", i);
        shapes *shape = load_shape_from_file(filename);
        add_shapes(shape, slist, false);
    }

    uint8_t board_test[BOARD_HEIGHT] = {0};
    generate_board(0, 0, 0, board_test);

    int board_spaces = count_free_spots(board_test);
    int shape_blocks = 0;
    shapes_list *shape_list_hold = slist;
    while (shape_list_hold != NULL)
    {
        shape_blocks += count_full_spots(shape_list_hold->shapes);
        shape_list_hold = shape_list_hold->next;
    }

    if (shape_blocks != board_spaces)
    {
        printf("The space which needs to be occupied on the board has %d spaces, the shapes however can fill %d spaces.\n", board_spaces, shape_blocks);
        return 0;
    }

    clock_t start = clock();
    for (int i = 0; i <= 11; i++)
    {
        printf("Checking month %d\n", i);
        for (int j = 0; j <= 30; j++)
        {
            for (int k = 0; k <= 6; k++)
            {
                uint8_t board[BOARD_HEIGHT] = {0};
                generate_board(i, j, k, board);
                if (!exist_solution(slist, board))
                {
                    printf("Can't find any solution for day %d %d %d.\n", i, j, k);
                }
            }
        }
    }
    clock_t end = clock();

    double cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Terminated in %f seconds.\n", cpu_time_used);

    // Free list
    free_list(slist);
}

//--------------------Pre-checks--------------------

int count_free_spots(const uint8_t *board)
{
    int v = 0;
    for (int i = 0; i < BOARD_HEIGHT; i++)
    {
        uint8_t line = *(board + i);
        for (int j = 0; j < BOARD_WIDTH; j++)
            if (!(line & (1ULL << j)))
                v++;
    }

    return v;
}

int count_full_spots(const shapes *shape)
{
    int v = 0;

    uint8_t *mask = shape->mask;
    for (int i = 0; i < shape->height; i++)
        for (int j = 0; j < shape->width; j++)
            if (*(mask + i) & (1ULL << j))
                v++;

    return v;
}

//-------------------------Debugging-------------------------

void print_shape(const shapes *shape)
{
    uint8_t *mask = shape->mask;

    for (int i = 0; i < shape->height; i++)
    {
        for (int j = 0; j < shape->width; j++)
        {
            if (*(mask + i) & (1ULL << j))
                printf("1 ");
            else
                printf(". ");
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

//-------------------------Memory freeing-------------------------

void free_shapes(shapes *shape)
{
    free(shape->mask);
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

//-------------------------Shape Generation-------------------------

shapes *load_shape_from_file(const char *file_name)
{
    shapes *shape = (shapes *)malloc(sizeof(shapes));

    FILE *fp;

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

    uint8_t *mask = (uint8_t *)malloc(sizeof(uint8_t) * shape->height);
    if (mask == NULL)
    {
        fprintf(stderr, "Erreur : Impossible d'allouer de la mémoire\n");
        exit(1);
    }

    int data;
    uint8_t *current_line;
    for (int i = 0; i < shape->height; i++)
    {
        current_line = mask + i;
        *current_line = 0;
        for (int j = 0; j < shape->width; j++)
        {
            fscanf(fp, "%d", &data);
            if (data != 0)
                *(mask + i) |= 1ULL << j;
        }
    }

    shape->mask = mask;

    fclose(fp);

    return shape;
}

bool shapes_equal(const shapes *shape_1, const shapes *shape_2)
{
    // Start by comparing the sizes
    if ((shape_1->width != shape_2->width) || (shape_1->height != shape_2->height))
        return false;

    uint8_t *mask_1 = shape_1->mask;
    uint8_t *mask_2 = shape_2->mask;

    // Then compare the contents
    for (int i = 0; i < shape_1->height; i++)
    {
        if (*(mask_1 + i) != *(mask_2 + i))
            return false;
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

    new_shape->next = NULL;

    uint8_t *mask = (uint8_t *)malloc(sizeof(uint8_t) * new_shape->height);

    uint8_t *current_line;

    // Loop over shape and rotate it 90deg by swapping x and y when getting from original shape
    for (int i = 0; i < new_shape->height; i++)
    {
        current_line = mask + i;
        *current_line = 0;
        for (int j = 0; j < new_shape->width; j++)
        {
            if (*(current->mask + current->height - 1 - j) & (1ULL << i))
            {
                *(mask + i) |= (1ULL << j);
            }
        }
    }

    // Add mask to shape
    new_shape->mask = mask;

    // Check if shape already in shapes array
    if (shape_equal_in_list(new_shape, shape))
    {
        free_shapes(new_shape);
        return true;
    }

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
    shapes *current = shape;

    // Declare new shape
    shapes *new_shape = (shapes *)malloc(sizeof(shapes));

    new_shape->width = current->width;
    new_shape->height = current->height;
    new_shape->next = NULL;

    uint8_t *mask = (uint8_t *)malloc(sizeof(uint8_t) * new_shape->height);

    uint8_t *current_line;

    // Loop over shape and mirror it by swapping x and y when getting from original shape
    for (int i = 0; i < new_shape->height; i++)
    {
        current_line = mask + i;
        *current_line = 0;
        for (int j = 0; j < new_shape->width; j++)
        {
            if (*(current->mask + i) & (1ULL << (new_shape->width - 1 - j)))
            {
                *(mask + i) |= (1ULL << j);
            }
        }
    }

    // Add mask to shape
    new_shape->mask = mask;

    // Check if shape already in shapes array
    if (shape_equal_in_list(new_shape, shape))
    {
        free_shapes(new_shape);
        return true;
    }

    shapes *temp = shape;

    while (temp->next != NULL)
    {
        temp = temp->next;
    }

    temp->next = new_shape;

    return false;
}

void add_shapes(shapes *new_shape, shapes_list *all_shapes, const bool mirror)
{
    // For each rotation create a shape and make it from the previous shape,
    // then check it isn't equal to other shapes in list

    bool isdone = rotate_last_90_degrees(new_shape); // 90deg rotations
    if (!isdone)
        isdone = rotate_last_90_degrees(new_shape); // 180deg rotations
    if (!isdone)
        isdone = rotate_last_90_degrees(new_shape); // 270deg rotations

    if (mirror)
    {
        isdone = mirror_shape(new_shape); // Flip

        if (!isdone)
            isdone = rotate_last_90_degrees(new_shape); // 90deg rotations of flip
        if (!isdone)
            isdone = rotate_last_90_degrees(new_shape); // 180deg rotations of flip
        if (!isdone)
            isdone = rotate_last_90_degrees(new_shape); // 270deg rotations of flip
    }

    if ((all_shapes)->shapes == NULL)
    {
        (all_shapes)->shapes = new_shape;
        all_shapes->next = NULL;
        return;
    }

    shapes_list *new_list = (shapes_list *)malloc(sizeof(shapes_list));

    new_list->next = NULL;
    new_list->shapes = new_shape;

    shapes_list *current = all_shapes;
    while (current->next != NULL)
    {
        current = current->next;
    }

    current->next = new_list;
}

//-------------------------Board Generation-------------------------

void print_board(const uint8_t *board)
{
    for (int i = 0; i < BOARD_HEIGHT; i++)
    {
        uint8_t line = *(board + i);
        for (int j = 0; j < BOARD_WIDTH; j++)
        {
            if (line & (1ULL << j))
                printf("1 ");
            else
                printf(". ");
        }
        printf("\n");
    }
}

void mark_date(const int month, const int month_day, const int day, uint8_t *board)
{
    // 1 on the board corresponds to a block (not placeable slot)
    if (month <= 5)
        board[0] |= 1UL << month;
    else
        board[1] |= 1UL << month - 6;

    // Defining the month-day position
    board[(month_day / 7) + 2] |= 1UL << (month_day % 7);

    // Defining the day
    if (day <= 3)
        board[6] |= 1UL << (day + 3);
    else
        board[7] |= 1UL << day;
}

void generate_board(const int month, const int month_day, const int day, uint8_t *board)
{
    // First blocks the date slots
    mark_date(month, month_day, day, board);

    // Blocks the border slots
    board[0] |= 1UL << 6;
    board[1] |= 1UL << 6;
    board[7] |= 0x0F; // 0 0 0 0 1 1 1 1
}

//-----------------------------------FIND SOLUTIONS-----------------------------------
bool can_place(const shapes *shape, const int x, const int y, const uint8_t *board)
{
    uint8_t *mask = shape->mask;

    for (int i = 0; i < shape->height; i++)
    {
        if (*(board + i + y) & (*(mask + i) << x))
            return false;
    }

    return true;
}

void place(const shapes *shape, const int x, const int y, uint8_t *board)
{
    uint8_t *mask = shape->mask;

    int i = shape->height;

    while (i)
    {
        i--;
        *(board + i + y) |= (*(mask + i) << x);
    }
}

bool new_can_place(const shapes *shape, const int x, const int y, const uint8_t *board, uint8_t *new_board)
{
    uint8_t *mask = shape->mask;
    uint8_t *new_board_pointer = new_board + y;

    int i = shape->height;

    while (i)
    {
        i--;

        uint8_t board_value = *(board + i + y);
        uint8_t mask_value = *(mask + i) << x;

        if (board_value & mask_value)
            return false;

        *(new_board_pointer + i) = board_value | mask_value;
    }

    return true;
}

bool exist_solution(const shapes_list *shapes_list, const uint8_t *board)
{
    if (shapes_list == NULL)
        return true;

    shapes *current_shape = shapes_list->shapes;

    // For each shape variant
    while (current_shape != NULL)
    {
        int h = BOARD_HEIGHT - current_shape->height + 1;
        int w = BOARD_WIDTH - current_shape->width + 1;

        // For each slots
        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                uint8_t new_board[BOARD_HEIGHT];
                memcpy(new_board, board, BOARD_SIZE);

                if (new_can_place(current_shape, x, y, board, new_board))
                {
                    if (exist_solution(shapes_list->next, new_board))
                        return true;
                }
            }
        }

        current_shape = current_shape->next;
    }

    return false;
}