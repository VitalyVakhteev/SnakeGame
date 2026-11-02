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
    cout << "hi";
}