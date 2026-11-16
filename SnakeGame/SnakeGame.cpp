#include <iostream>;
#include <fstream>;

using namespace std;

enum Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

enum CellContains {
    EMPTY,
    APPLE,
    SNAKE
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
    char name[32];
};

User current_player;

int main()
{
    current_player = load_player_data();
}

User load_player_data() {
    User player;
    ifstream in_stream;
    ofstream out_stream;
    in_stream.open("data.txt");
    out_stream.open("data.txt");
    in_stream >> player.name;
    in_stream >> player.high_score;

    if (!player.name || !player.high_score) {
        cout << "Enter Player Name: ";
        cin >> player.name;
        player.high_score = 0;
    }

    in_stream.close();
    out_stream << player.name;
    out_stream << player.high_score;
    return player;
}