#include <iostream>;
#include <fstream>;
#include <string>
#include <deque>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>

using namespace std;

const int GRID_SIZE = 20;
const int SPAWN_RADIUS = 6;
const int TICK_MS = 500;
const string DATA_FILE = "data.txt";

const char SNAKE_HEAD = '@';
const char SNAKE_BODY = '#';
const char APPLE_CHAR = '$';
const char EMPTY_CHAR = '.';

enum class Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

enum class CellContains {
    EMPTY,
    APPLE,
    SNAKE
};

enum class MenuChoice {
    START,
    EDIT_PLAYER,
    QUIT
};

enum class Result {
    GAME_OVER,
    WIN
};

struct Cell {
    CellContains object;
};

struct Point {
    int x; 
    int y;
};

struct User {
    short int high_score;
    string name;
};

// we use the ampersand symbol, &, to pass by reference (as we don't want to duplicate the player/snake/etc.)
bool load_player_data(User& player);
void save_player_data(const User& player);
void create_or_edit_player(User& player);

void display_main_menu(const User& player);
MenuChoice get_menu_choice();

void init_grid(Cell grid[GRID_SIZE][GRID_SIZE]);
void init_snake(deque<Point>& snake, Cell grid[GRID_SIZE][GRID_SIZE]);
bool place_apple(Cell grid[GRID_SIZE][GRID_SIZE]);

Result run_game(User& player);
Direction read_input(Direction current);
Point next_head_position(const Point& head, Direction dir);
bool is_out_of_bounds(const Point& p);
bool hits_self(const deque<Point>& snake, const Point& next);
bool grid_is_full(Cell grid[GRID_SIZE][GRID_SIZE]);
void render(Cell grid[GRID_SIZE][GRID_SIZE], const deque<Point>& snake);

void show_end_screen(Result result, short int score, const User& player);
void update_high_score(User& player, int score);

int main()
{
    // we use the current Unix timestamp and cast it to an unsigned integer to use it as a seed for rand 
    srand(static_cast<unsigned>(time(nullptr)));

    User current_player;

    // TODO: if load_player_data fails, call create_or_edit_player

    bool running = true;
    while (running) {
        // Todo: display_main_menu, switch case on get_menu_choice()
        // if START, run_game and show_end screen, update_high_score
        // if EDIT_PLAYER, create_or_edit_player
        // if QUIT, running is set to false
        running = false;
    }
    
    return 0;
}

bool load_player_data(User& player) {
    ifstream in_stream;
    in_stream.open(DATA_FILE);
    if (!in_stream.is_open() || in_stream.fail()) {
        cout << "ERROR: FAILED TO LOAD PLAYER";
        return false;
    }

    in_stream >> player.name;
    in_stream >> player.high_score;

    if (in_stream.fail()) {
        cout << "ERROR: FAILED TO READ DATA";
        return false;
    }

    return true;
}

void save_player_data(const User& player) {
    // Todo: use the OFSTREAM on DATA_FILE, write the player.name and player.high_score
}

void create_or_edit_player(User& player) {
    // Todo: ask for the name, save to player.name
    // Todo: save_player_data(player)
}

void display_main_menu(const User& player) {
    // Todo: print title, player name, high score, and menu options such that it fills the terminal window
}

MenuChoice get_menu_choice() {
    // Todo: read the user input, validate it, and map it to the MenuChoice enumerated class
    return MenuChoice::QUIT;
}

void init_grid(Cell grid[GRID_SIZE][GRID_SIZE]) {
    // Todo: Set every cell in grid to CellContains::EMPTY with a nested for loop
}

void init_snake(deque<Point>& snake, Cell grid[GRID_SIZE][GRID_SIZE]) {
    // Todo: pick a random cell within SPAWN_RADIUS of the center of the grid
    // Todo: push it to the snake queue as the head point, and mark the corresponding grid cell as CellContains::SNAKE
}

bool place_apple(Cell grid[GRID_SIZE][GRID_SIZE]) {
    // TODO: choose a random EMPTY cell in the grid and set it to APPLE with rand()
    return false;
}

Result run_game(User& player) {
    Cell grid[GRID_SIZE][GRID_SIZE];
    deque<Point> snake;
    // todo: decide if we want a random direction
    Direction dir = Direction::RIGHT;

    init_grid(grid);
    init_snake(snake, grid);
    place_apple(grid);

    bool playing = true;
    while (playing) {
        // todo: read input and set it as a direction
        // read the next position of the head of the snake
        // if it is out of bounds or has hit itself, set the result to GAME_OVER
        // otherwise, if an apple is eaten, push a point to the head of the snake, don't pop the tail, and place another apple
        // if the grid is full, return WIN
        // render the grid with render(grid, snake);

        this_thread::sleep_for(chrono::milliseconds(TICK_MS));
        playing = false;
    }

    return Result::GAME_OVER;
}

Direction read_input(Direction current) {
    // Todo: read the next input and assign a direction with it;
    return current;
}

Point next_head_position(const Point& head, Direction dir) {
    // Todo: offset the snake's head cell by one cell towards direction dir and return the head
    return head;
}

bool is_out_of_bounds(const Point& p) {
    // Check if p is outside of the bounds of the grid [0, GRID_SIZE-1], maybe with a for-loop
    // todo: determine if we should instead keep a list of empty cells for quicker checks
    return false;
}

bool hits_self(const deque<Point>& snake, const Point& next) {
    // Todo: Check if any of the cells in the queue have the same position as next, excluding the tail
    // we exclude the tail because "chasing the tail" is allowed, as the snake will move out of that position on the next frame
    return false;
}

bool grid_is_full(Cell grid[GRID_SIZE][GRID_SIZE]) {
    // Todo: return true if no grid cell is EMPTY
    return false;
}

void render(Cell grid[GRID_SIZE][GRID_SIZE], const deque<Point>& snake) {
    // Todo: clear the current screen and draw a new grid
    // the new grid will consist of the characters SNAKE_HEAD, SNAKE_BODY, APPLE_CHAR, and EMPTY_CHAR
    // show the current length with (snake.size())
}

void show_end_screen(Result result, short int score, const User& player) {
    // clear the screen and print "Game Over", "You Win", score, high_score
}

void update_high_score(User& player, int score) {
    // if the current score > player.high_score, update the player's high score and run save_player_data
}
