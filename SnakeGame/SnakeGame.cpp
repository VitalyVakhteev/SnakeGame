// input/output, file input/output
#include <iostream>
#include <fstream>
// string methods
#include <string>
// linked list methods for snake
#include <deque>
// standard library of functions and time (for randomizing with timestamp)
#include <cstdlib>
#include <ctime>
#include <chrono>
// thread for sleeping an accurate amount of time
#include <thread>
#include <limits>
#include <cctype>
#include <cstdio>
// signal is used for properly handling interrupts/returns on unix terminals
#include <csignal>
// vector is used to store graphical string elements properly at runtime
// we cannot use a char[] array (or rather, it's bad to do so) because the user can resize the screen, which causes drift w.r.t offsets
// thus, we use a  container that can resize and adjust to reindexing of string characters at runtime, aka a vector
#include <vector>
#include <algorithm>

// i am selfish and want this game to work on both windows shells and my own linux laptop shell >:)
// so we define different imports using #ifdef to use the correct libraries for each platform
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#endif

using namespace std;

// game constants (e.g. grid size for the map boundary for the snake, spawn radius, speed)
const int GRID_SIZE = 20;
const int SPAWN_RADIUS = 6;
const int TICK_MS = 250;
const string DATA_FILE = "data.txt";
const size_t MAX_NAME_LEN = 32;

// main menu settings
const int MENU_TICK_MS = 200;
const int MENU_MAX_SNAKES = 3;
const int MENU_MAX_DELAY = 90;
const int END_SCREEN_DELAY_MS = 500;

// keys for the menu options
const char KEY_PLAY = 'p';
const char KEY_EDIT = 'e';
const char KEY_QUIT = 'q';

// characters we use to draw the game objects
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


struct MenuSnake {
    deque<Point> body;
    Direction dir;
    Point apple;
    bool has_apple;
    int delay;
};

void enable_raw_mode(); // we run this when the game starts to make inputs much more natural
void disable_raw_mode(); // we run this to disable the raw mode enabled above
struct RawModeGuard {
    RawModeGuard() {
        enable_raw_mode();
        cout << "\x1b[?25l\x1b[2J" << flush; // \x1b is an escape character we use to send commands; in this case, we hide cursor, clear screen
    }
    ~RawModeGuard() {
        cout << "\x1b[?25h\x1b[2J\x1b[H" << flush; // show cursor, clear screen, and move the cursor to the top-left of the terminal
        disable_raw_mode();
    }
};

// note: these functions work differently for windows and linux because their respective terminals work differently
// this is also the only part of the game that runs differently on each platform
void init_console();
int read_key();
void get_terminal_size(int& rows, int& cols);
void restore_terminal();

// we use the ampersand symbol, &, to pass by reference (as we don't want to duplicate the player/snake/etc.)
bool load_player_data(User& player);
void save_player_data(const User& player);
void create_or_edit_player(User& player);
string trim(const string& s);

MenuChoice run_main_menu(const User& player);
void display_main_menu(const User& player, const vector<MenuSnake>& snakes, int rows, int cols);
bool get_menu_choice(int key, MenuChoice& choice);
void menu_field_size(int rows, int cols, int& field_w, int& field_h);
bool in_field(const Point& p, int field_w, int field_h);

MenuSnake spawn_menu_snake(int field_w, int field_h);
void place_menu_apple(MenuSnake& s, int field_w, int field_h);
void step_menu_snake(MenuSnake& s, int field_w, int field_h);
bool menu_snake_finished(const MenuSnake& s, int field_w, int field_h);

// graphics and drawing functions
vector<string> make_canvas(int rows, int cols);
void put_text(vector<string>& canvas, int row, int col, const string& text);
string pad_right(const string& s, size_t width);
bool draw_box(vector<string>& canvas, const vector<string>& lines, vector<Point>* line_starts = nullptr);
void present(const vector<string>& canvas);
bool draw_box_screen(const vector<string>& lines, vector<Point>* line_starts = nullptr);

void init_grid(Cell grid[GRID_SIZE][GRID_SIZE]);
void init_snake(deque<Point>& snake, Cell grid[GRID_SIZE][GRID_SIZE]);
bool place_apple(Cell grid[GRID_SIZE][GRID_SIZE]);

Result run_game(User& player, short int& score);
Direction read_input(Direction current);
bool is_opposite(Direction a, Direction b);
Point next_head_position(const Point& head, Direction dir);
bool is_out_of_bounds(const Point& p);
bool hits_self(const deque<Point>& snake, const Point& next);
bool grid_is_full(Cell grid[GRID_SIZE][GRID_SIZE]);
void render(Cell grid[GRID_SIZE][GRID_SIZE], const deque<Point>& snake);

void show_end_screen(Result result, short int score, const User& player, bool new_high_score);
void update_high_score(User& player, int score);

int main()
{
    // we use the current Unix timestamp and cast it to an unsigned integer to use it as a seed for rand
    srand(static_cast<unsigned>(time(nullptr)));
    init_console();

    User current_player;
    short int current_score = 0;

    bool success = load_player_data(current_player);

    if (!success) {
	// create junk data
        current_player.name = "";
        current_player.high_score = 0;
        create_or_edit_player(current_player);
    }

    bool running = true;
    while (running) {
        // if START, run_game and show_end screen, update_high_score
        // if EDIT_PLAYER, create_or_edit_player
        // if QUIT, running is set to false

        MenuChoice choice = run_main_menu(current_player);
        switch (choice) {
            case MenuChoice::START: {
                Result player_result = run_game(current_player, current_score);
                bool new_high_score = current_score > current_player.high_score;
                update_high_score(current_player, current_score);
                show_end_screen(player_result, current_score, current_player, new_high_score);
                break;
            }
            case MenuChoice::EDIT_PLAYER:
                create_or_edit_player(current_player);
                break;
            case MenuChoice::QUIT:
                running = false;
                break;
        }
    }

    return 0;
}

bool load_player_data(User& player) {
    ifstream in_stream;
    in_stream.open(DATA_FILE);
    if (!in_stream.is_open() || in_stream.fail()) {
        return false;
    }

    getline(in_stream, player.name);
    player.name = trim(player.name);
    in_stream >> player.high_score;

    if (in_stream.fail() || player.name.empty()) {
        return false;
    }

    return true;
}

void save_player_data(const User& player) {
    ofstream out_stream;
    out_stream.open(DATA_FILE);

    if (out_stream.is_open()) {
        out_stream << player.name << '\n';
        out_stream << player.high_score << '\n';
        out_stream.close();
    }
    else {
        cout << "ERROR: FAILED TO SAVE PLAYER";
    }
}

void create_or_edit_player(User& player) {
    string heading = player.name.empty() ? "No player found. Let's create one!" : "Edit player";
    string name;

    while (true) {
        vector<string> lines = { heading, "", "Enter player name:", string(MAX_NAME_LEN, '_') };
        vector<Point> starts;
        cout << "\x1b[2J" << flush;
        if (draw_box_screen(lines, &starts)) {
            cout << "\x1b[" << starts[3].y + 1 << ";" << starts[3].x + 1 << "H" << flush;
        }
        else {
            cout << "\x1b[2J\x1b[HEnter player name: " << flush;
        }

        if (!getline(cin, name)) {
            cin.clear();
            name = player.name.empty() ? "Player" : player.name;
            break;
        }

        name = trim(name);
        if (name.size() > MAX_NAME_LEN) {
            name.resize(MAX_NAME_LEN);
        }
        if (!name.empty()) {
            break;
        }
        heading = "Your name can't be empty";
    }

    player.name = name;
    save_player_data(player);
}

string trim(const string& s) {
    // regex; we use size_t, which is defined under cstring.h, to hold string lengths
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) {
        return "";
    }
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

MenuChoice run_main_menu(const User& player) {
    RawModeGuard raw;
    // we don't have a fixed size of snakes; we use a vector here instead of an array as a result
    vector<MenuSnake> snakes;
    int last_w = -1, last_h = -1;

    while (true) {
        int key;
        MenuChoice choice;
        while ((key = read_key()) != -1) {
            if (get_menu_choice(key, choice)) {
                return choice;
            }
        }

        int rows, cols;
        get_terminal_size(rows, cols);
        int field_w, field_h;
        menu_field_size(rows, cols, field_w, field_h);

        if (field_w != last_w || field_h != last_h) {
            snakes.clear();
            last_w = field_w;
            last_h = field_h;
            if (field_w > 0 && field_h > 0) {
                int count = min(max(field_w * field_h / 120, 2), MENU_MAX_SNAKES);
                for (int i = 0; i < count; i++) {
                    MenuSnake s = spawn_menu_snake(field_w, field_h);
                    s.delay = 0;
                    int warmup = rand() % max(field_w, field_h);
                    for (int k = 0; k < warmup && !menu_snake_finished(s, field_w, field_h); k++) {
                        step_menu_snake(s, field_w, field_h);
                    }
                    snakes.push_back(s);
                }
            }
        }

	// spawn snakes
        for (MenuSnake& s : snakes) {
            step_menu_snake(s, field_w, field_h);
            if (menu_snake_finished(s, field_w, field_h)) {
                s = spawn_menu_snake(field_w, field_h);
            }
        }

        display_main_menu(player, snakes, rows, cols);
        this_thread::sleep_for(chrono::milliseconds(MENU_TICK_MS));
    }
}

void display_main_menu(const User& player, const vector<MenuSnake>& snakes, int rows, int cols) {
    // we represent the canvas with a vector of strings
    vector<string> canvas = make_canvas(max(rows - 1, 1), max(cols, 1));
    int field_w, field_h;
    menu_field_size(rows, cols, field_w, field_h);

    // lambda function to ensure that the main menu elements we draw sits inside the cell properly
    auto plot = [&](const Point& p, char ch) {
        if (in_field(p, field_w, field_h)) {
            canvas[1 + p.y][1 + 2 * p.x] = ch;
        }
    };

    for (const MenuSnake& s : snakes) {
        if (s.has_apple) {
            plot(s.apple, APPLE_CHAR);
        }
    }
    for (const MenuSnake& s : snakes) {
        for (size_t i = 1; i < s.body.size(); i++) {
            plot(s.body[i], SNAKE_BODY);
        }
    }
    for (const MenuSnake& s : snakes) {
        plot(s.body.front(), SNAKE_HEAD);
    }

    // we use a vector to represent the main menu blocks to ensure we can space them properly
    vector<string> block = {
        "Player:     " + player.name,
        "High score: " + to_string(player.high_score),
        "",
        string("[") + (char)toupper(KEY_PLAY) + "] Play",
        string("[") + (char)toupper(KEY_EDIT) + "] Edit player",
        string("[") + (char)toupper(KEY_QUIT) + "] Quit",
    };
    size_t block_w = 0;
    for (const string& line : block) {
        block_w = max(block_w, line.size());
    }

    vector<string> lines = { "~    SNAKE    ~", "" };
    for (const string& line : block) {
        lines.push_back(pad_right(line, block_w));
    }

    if (!draw_box(canvas, lines)) {
        put_text(canvas, 1, 2, "Terminal window too small. Please enlarge it");
    }
    present(canvas);
}

bool get_menu_choice(int key, MenuChoice& choice) {
    switch (tolower(key)) {
        case KEY_PLAY: choice = MenuChoice::START;       return true;
        case KEY_EDIT: choice = MenuChoice::EDIT_PLAYER; return true;
        case KEY_QUIT: choice = MenuChoice::QUIT;        return true;
        default: return false;
    }
}

void menu_field_size(int rows, int cols, int& field_w, int& field_h) {
    field_w = max((cols - 2) / 2, 0);
    field_h = max(rows - 1 - 2, 0);
}

bool in_field(const Point& p, int field_w, int field_h) {
    return p.x >= 0 && p.x < field_w && p.y >= 0 && p.y < field_h;
}

MenuSnake spawn_menu_snake(int field_w, int field_h) {
    MenuSnake s;
    s.dir = static_cast<Direction>(rand() % 4);
    s.delay = rand() % MENU_MAX_DELAY;

    Point head;
    switch (s.dir) {
        case Direction::RIGHT: head = { -1, rand() % field_h };      break;
        case Direction::LEFT:  head = { field_w, rand() % field_h }; break;
        case Direction::DOWN:  head = { rand() % field_w, -1 };      break;
        case Direction::UP:    head = { rand() % field_w, field_h }; break;
    }

    Direction back = Direction::LEFT;
    switch (s.dir) {
        case Direction::RIGHT: back = Direction::LEFT;  break;
        case Direction::LEFT:  back = Direction::RIGHT; break;
        case Direction::DOWN:  back = Direction::UP;    break;
        case Direction::UP:    back = Direction::DOWN;  break;
    }

    int length = 3 + rand() % 4;
    Point p = head;
    for (int i = 0; i < length; i++) {
        s.body.push_back(p);
        p = next_head_position(p, back);
    }

    place_menu_apple(s, field_w, field_h);
    return s;
}

void place_menu_apple(MenuSnake& s, int field_w, int field_h) {
    Point a = s.body.front();
    int distance = 3 + rand() % 10;
    for (int i = 0; i < distance; i++) {
        a = next_head_position(a, s.dir);
    }
    s.apple = a;
    s.has_apple = in_field(a, field_w, field_h);
}

void step_menu_snake(MenuSnake& s, int field_w, int field_h) {
    if (s.delay > 0) {
        s.delay--;
        return;
    }
    Point next = next_head_position(s.body.front(), s.dir);
    s.body.push_front(next);
    if (s.has_apple && next.x == s.apple.x && next.y == s.apple.y) {
        place_menu_apple(s, field_w, field_h);
    }
    else {
        s.body.pop_back();
    }
}

bool menu_snake_finished(const MenuSnake& s, int field_w, int field_h) {
    const Point& tail = s.body.back();
    switch (s.dir) {
        case Direction::RIGHT: return tail.x >= field_w;
        case Direction::LEFT:  return tail.x < 0;
        case Direction::DOWN:  return tail.y >= field_h;
        case Direction::UP:    return tail.y < 0;
    }
    return true;
}

// set up the main canvas for the game, bordered by +-----....-----+
vector<string> make_canvas(int rows, int cols) {
    vector<string> canvas(rows, string(cols, ' '));
    if (rows < 2 || cols < 2) {
        return canvas;
    }
    string border = "+" + string(cols - 2, '-') + "+";
    canvas[0] = border;
    canvas[rows - 1] = border;
    for (int r = 1; r < rows - 1; r++) {
        canvas[r][0] = '|';
        canvas[r][cols - 1] = '|';
    }
    return canvas;
}

void put_text(vector<string>& canvas, int row, int col, const string& text) {
    if (row < 0 || row >= (int) canvas.size()) {
        return;
    }
    for (size_t i = 0; i < text.size(); i++) {
        int c = col + (int) i;
        if (c >= 0 && c < (int) canvas[row].size()) {
            canvas[row][c] = text[i];
        }
    }
}

string pad_right(const string& s, size_t width) {
    return s.size() >= width ? s : s + string(width - s.size(), ' ');
}

bool draw_box(vector<string>& canvas, const vector<string>& lines, vector<Point>* line_starts) {
    // draws a bordered box in the middle of the canvas
    int rows = (int) canvas.size();
    int cols = rows > 0 ? (int) canvas[0].size() : 0;

    size_t longest = 0;
    for (const string& line : lines) {
        longest = max(longest, line.size());
    }
    int inner_w = (int) longest + 4;
    int box_w = inner_w + 2;
    int box_h = (int) lines.size() + 4;
    if (box_w + 2 > cols || box_h + 2 > rows) {
        return false;
    }

    int top = (rows - box_h) / 2;
    int left = (cols - box_w) / 2;
    string border = "+" + string(inner_w, '-') + "+";
    string blank = "|" + string(inner_w, ' ') + "|";

    put_text(canvas, top, left, border);
    for (int r = 1; r < box_h - 1; r++) {
        put_text(canvas, top + r, left, blank);
    }
    put_text(canvas, top + box_h - 1, left, border);

    if (line_starts) {
	// dereference and clear
        line_starts->clear();
    }
    for (size_t i = 0; i < lines.size(); i++) {
        Point start = { left + 1 + (inner_w - (int) lines[i].size()) / 2, top + 2 + (int) i };
        put_text(canvas, start.y, start.x, lines[i]);
        if (line_starts) {
            line_starts->push_back(start);
        }
    }
    return true;
}

void present(const vector<string>& canvas) {
    // render the screen
    static size_t last_rows = 0, last_cols = 0;
    size_t cols = canvas.empty() ? 0 : canvas[0].size();
    string frame;
    if (canvas.size() != last_rows || cols != last_cols) {
        frame += "\x1b[2J";
        last_rows = canvas.size();
        last_cols = cols;
    }
    for (size_t r = 0; r < canvas.size(); r++) {
        frame += "\x1b[" + to_string(r + 1) + ";1H" + canvas[r];
    }
    cout << frame << flush;
}

bool draw_box_screen(const vector<string>& lines, vector<Point>* line_starts) {
    int rows, cols;
    get_terminal_size(rows, cols);
    vector<string> canvas = make_canvas(max(rows - 1, 1), max(cols, 1));
    bool fits = draw_box(canvas, lines, line_starts);
    if (!fits) {
        put_text(canvas, 1, 2, "Terminal window too small. Please enlarge it");
    }
    present(canvas);
    return fits;
}

void init_grid(Cell grid[GRID_SIZE][GRID_SIZE]) {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            grid[i][j].object = CellContains::EMPTY;
        }
    }
}

void init_snake(deque<Point>& snake, Cell grid[GRID_SIZE][GRID_SIZE]) {
    // pick a random cell within SPAWN_RADIUS of the center of the grid
    // push it to the snake queue as the head point, and mark the corresponding grid cell as CellContains::SNAKE
    int center_of_grid = GRID_SIZE / 2;
    int randx = (rand() % (SPAWN_RADIUS + 1)) - (SPAWN_RADIUS / 2);
    int randy = (rand() % (SPAWN_RADIUS + 1)) - (SPAWN_RADIUS / 2);

    Point rand_point;
    rand_point.x = center_of_grid + randx;
    rand_point.y = center_of_grid + randy;

    snake.push_back(rand_point);
    grid[rand_point.y][rand_point.x].object = CellContains::SNAKE;
}

bool place_apple(Cell grid[GRID_SIZE][GRID_SIZE]) {
    if (grid_is_full(grid)) {
        return false;
    }

    int rand_row;
    int rand_col;
    do {
        rand_row = rand() % GRID_SIZE;
        rand_col = rand() % GRID_SIZE;
    } while (grid[rand_row][rand_col].object != CellContains::EMPTY);

    grid[rand_row][rand_col].object = CellContains::APPLE;
    return true;
}

Result run_game(User& player, short int& score) {
    RawModeGuard raw;

    Cell grid[GRID_SIZE][GRID_SIZE];
    deque<Point> snake;
    Direction dir = Direction::RIGHT;
    score = 0;

    init_grid(grid);
    init_snake(snake, grid);
    place_apple(grid);

    render(grid, snake);
    while (true) {
        this_thread::sleep_for(chrono::milliseconds(TICK_MS));

        dir = read_input(dir);
        Point next = next_head_position(snake.front(), dir);

        if (is_out_of_bounds(next) || hits_self(snake, next)) {
            return Result::GAME_OVER;
        }

        bool eating = grid[next.y][next.x].object == CellContains::APPLE;

        if (!eating) {
            Point tail = snake.back();
            snake.pop_back();
            grid[tail.y][tail.x].object = CellContains::EMPTY;
        }
        snake.push_front(next);
        grid[next.y][next.x].object = CellContains::SNAKE;

        if (eating) {
            score++;
            if (!place_apple(grid)) {
                return Result::WIN;
            }
        }

        render(grid, snake);
    }
}

Direction read_input(Direction current) {
    Direction next = current;
    int key;

    while ((key = read_key()) != -1) {
        Direction wanted = next;
        switch (tolower(key)) {
            case 'w': wanted = Direction::UP;    break;
            case 'a': wanted = Direction::LEFT;  break;
            case 's': wanted = Direction::DOWN;  break;
            case 'd': wanted = Direction::RIGHT; break;
            default: break;
        }

        if (!is_opposite(wanted, current)) {
            next = wanted;
        }
    }
    return next;
}

// check if the last two inputs directly oppose each other to avoid a 180-degree turn and instant game-over
bool is_opposite(Direction a, Direction b) {
    return (a == Direction::UP && b == Direction::DOWN) || (a == Direction::DOWN && b == Direction::UP) ||
           (a == Direction::LEFT && b == Direction::RIGHT) || (a == Direction::RIGHT && b == Direction::LEFT);
}

Point next_head_position(const Point& head, Direction dir) {
    switch (dir) {
        case Direction::UP:    return { head.x, head.y - 1 };
        case Direction::DOWN:  return { head.x, head.y + 1 };
        case Direction::LEFT:  return { head.x - 1, head.y };
        case Direction::RIGHT: return { head.x + 1, head.y };
    }
    return head;
}

bool is_out_of_bounds(const Point& p) {
    // Check if p is outside of the bounds of the grid [0, GRID_SIZE-1]
    return p.x < 0 || p.x > GRID_SIZE - 1 || p.y < 0 || p.y > GRID_SIZE - 1;
}

bool hits_self(const deque<Point>& snake, const Point& next) {
    // Check if any of the cells in the queue have the same position as next, excluding the tail
    // we exclude the tail because chasing your tail is allowed, as the snake will move out of that position on the next frame
    for (size_t i = 0; i + 1 < snake.size(); i++) {
        if (snake[i].x == next.x && snake[i].y == next.y) {
            return true;
        }
    }
    return false;
}

bool grid_is_full(Cell grid[GRID_SIZE][GRID_SIZE]) {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (grid[i][j].object == CellContains::EMPTY) {
                return false;
            }
        }
    }

    return true;
}

void render(Cell grid[GRID_SIZE][GRID_SIZE], const deque<Point>& snake) {
    int rows, cols;
    get_terminal_size(rows, cols);

    const int board_w = GRID_SIZE * 2 + 2;
    const int board_h = GRID_SIZE + 3;

    string frame;

    static int last_rows = -1, last_cols = -1;
    if (rows != last_rows || cols != last_cols) {
	// clear screen
        frame += "\x1b[2J";
        last_rows = rows;
        last_cols = cols;
    }

    if (cols < board_w || rows < board_h) {
	// move cursor to home position and write
        frame += "\x1b[HTerminal window too small. Please enlarge it";
        cout << frame << flush;
        return;
    }

    int top = (rows - board_h) / 2 + 1;
    int left = (cols - board_w) / 2 + 1;
    // lambda function to move the cursor to a given position and draw at the given r,c
    auto move_to = [&](int r, int c) {
        frame += "\x1b[" + to_string(r) + ";" + to_string(c) + "H";
    };

    // draw the border of the screen
    string border = "+" + string(GRID_SIZE * 2, '-') + "+";
    const Point& head = snake.front();

    move_to(top, left);
    frame += border;
    for (int y = 0; y < GRID_SIZE; y++) {
        move_to(top + 1 + y, left);
        frame += '|';
        for (int x = 0; x < GRID_SIZE; x++) {
            char ch = EMPTY_CHAR;
            if (x == head.x && y == head.y) {
                ch = SNAKE_HEAD;
            }
            else if (grid[y][x].object == CellContains::SNAKE) {
                ch = SNAKE_BODY;
            }
            else if (grid[y][x].object == CellContains::APPLE) {
                ch = APPLE_CHAR;
            }
            frame += ch;
            frame += ' ';
        }
        frame += '|';
    }
    move_to(top + 1 + GRID_SIZE, left);
    frame += border;
    move_to(top + 2 + GRID_SIZE, left);
    // the snake starts at length 1, so the score (apples eaten) is one less than the length
    frame += "Score: " + to_string(snake.size() - 1) + "    ";

    cout << frame << flush;
}

void show_end_screen(Result result, short int score, const User& player, bool new_high_score) {
    RawModeGuard raw;

    vector<string> lines;
    switch (result) {
        case Result::GAME_OVER: lines.push_back("GAME OVER."); break;
        case Result::WIN:       lines.push_back("YOU WIN!");   break;
    }
    lines.push_back("");
    lines.push_back(pad_right("Score:      " + to_string(score), 18));
    lines.push_back(pad_right("High score: " + to_string(player.high_score), 18));
    if (new_high_score) {
        lines.push_back("");
        lines.push_back("* New high score! *");
    }
    lines.push_back("");
    lines.push_back("Press any key to return to the menu");

    draw_box_screen(lines);
    this_thread::sleep_for(chrono::milliseconds(END_SCREEN_DELAY_MS));
    while (read_key() != -1) {}

    while (read_key() == -1) {
        draw_box_screen(lines);
        this_thread::sleep_for(chrono::milliseconds(MENU_TICK_MS));
    }
}

void update_high_score(User& player, int score) {
    if (score > player.high_score) {
        player.high_score = score;
    }
    save_player_data(player);
}

void restore_terminal() {
    disable_raw_mode();
    const char seq[] = "\x1b[?25h\x1b[?1049l"; // show cursor, leave game screen
#ifdef _WIN32
    fwrite(seq, 1, sizeof(seq) - 1, stdout);
    fflush(stdout);
#else
    ssize_t ignored = write(STDOUT_FILENO, seq, sizeof(seq) - 1);
    (void)ignored;
#endif
}

// windows shells
#ifdef _WIN32

void init_console() {
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(out, &mode)) {
        SetConsoleMode(out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
    atexit(restore_terminal);
    cout << "\x1b[?1049h" << flush; // alt screen to avoid terminal history
}

void enable_raw_mode() {
    // nothing to do on windows
}

void disable_raw_mode() {
    // keys pressed during the game can suddenly get dumped from the buffer when we return to the main menu
    // because of this, we flush any inputs when exiting
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
}

int read_key() {
    if (!_kbhit()) {
        return -1;
    }
    int c = _getch();
    // map ascii to chars
    if (c == 0 || c == 224) {
        switch (_getch()) {
            case 72: return 'w'; // up
            case 80: return 's'; // down
            case 75: return 'a'; // left
            case 77: return 'd'; // right
            default: return -1;  // any special key we don't use
        }
    }
    return c;
}

void get_terminal_size(int& rows, int& cols) {
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info)) {
        cols = info.srWindow.Right - info.srWindow.Left + 1;
        rows = info.srWindow.Bottom - info.srWindow.Top + 1;
    }
    else {
        rows = 24;
        cols = 80;
    }
}

// linux or mac support (aka my laptop...)
#else

termios original_term;
bool raw_mode_on = false;

void on_signal(int sig) {
    restore_terminal();
    signal(sig, SIG_DFL);
    raise(sig);
}

void init_console() {
    // get terminal settings
    tcgetattr(STDIN_FILENO, &original_term);
    atexit(restore_terminal);
    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);
    cout << "\x1b[?1049h" << flush; // alt screen to avoid terminal history
}

void enable_raw_mode() {
    if (raw_mode_on) {
        return;
    }
    // terminal settings var
    termios raw = original_term;
    // don't echo characters when typed
    raw.c_lflag &= ~(ICANON | ECHO);
    // return characters instantly
    raw.c_cc[VMIN] = 0;
    // return characters when no key is waiting in the buffer
    raw.c_cc[VTIME] = 0;
    // write terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    raw_mode_on = true;
}

void disable_raw_mode() {
    if (!raw_mode_on) {
        return;
    }
    // TCSAFLUSH also throws away keys pressed during the game, similar to windows, so they don't get dumped into the main menu
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_term);
    raw_mode_on = false;
}

int read_key() {
    unsigned char c;
    // if we didn't read a character, return -1
    if (read(STDIN_FILENO, &c, 1) != 1) {
        return -1;
    }
    // on UNIX systems, keys arrive as three bytes, not ASCII integer codes. we have to read the array it returns
    if (c == 27) {
        unsigned char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) == 1 && seq[0] == '[' &&
            read(STDIN_FILENO, &seq[1], 1) == 1) {
            switch (seq[1]) {
                case 'A': return 'w'; // up
                case 'B': return 's'; // down
                case 'D': return 'a'; // left
                case 'C': return 'd'; // right
            }
        }
        return -1;
    }
    return c;
}

void get_terminal_size(int& rows, int& cols) {
    winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0) {
        rows = w.ws_row;
        cols = w.ws_col;
    }
    else {
        rows = 24;
        cols = 80;
    }
}

#endif
