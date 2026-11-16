#include <iostream>;
#include <fstream>;

using namespace std;

enum Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

struct Snake {
    short int length;
    Direction snake_direction;
};

struct User {
    short int high_score;
    char name[32];
};

int main()
{
    User username;
    User highscore;
    ifstream in_stream;
    ofstream out_stream;
    in_stream.open("infile.txt");
    out_stream.open("outfile.txt");
    cout << "Enter Player Name: ";
    cin >> username.name;
    highscore.high_score = 0;
    in_stream >> username.name;
    in_stream >> highscore.high_score;
    in_stream.close();
    out_stream.close();
    cout << "\nPlayer Information:\n";
    cout << "Name: " << username.name;
    cout << "High Score: " << username.high_score;
}