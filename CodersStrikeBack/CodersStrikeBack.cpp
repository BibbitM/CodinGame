#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

template <typename T>
T clamp(const T& n, const T& lower, const T& upper)
{
    return min(max(n, lower), upper);
}

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/
int main()
{
    bool busted = false;

    // game loop
    while (1) {
        int x;
        int y;
        int nextCheckpointX; // x position of the next check point
        int nextCheckpointY; // y position of the next check point
        int nextCheckpointDist; // distance to the next checkpoint
        int nextCheckpointAngle; // angle between your pod orientation and the direction of the next checkpoint
        cin >> x >> y >> nextCheckpointX >> nextCheckpointY >> nextCheckpointDist >> nextCheckpointAngle; cin.ignore();
        int opponentX;
        int opponentY;
        cin >> opponentX >> opponentY; cin.ignore();

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;

        static const int MAX_SPEED = 100;        
        int speed = MAX_SPEED;
        
        speed = clamp(speed, 5, nextCheckpointDist / 3);
        
        if (nextCheckpointAngle > 90 || nextCheckpointAngle < -90)
            speed = 0;

        bool bust_now = false;
        if (!busted && nextCheckpointAngle < 10 && nextCheckpointAngle > -10 && speed == MAX_SPEED && nextCheckpointDist > 200)
        {
            bust_now = true;
            busted = false;
        }

        // You have to output the target position
        // followed by the power (0 <= thrust <= 100)
        // i.e.: "x y thrust"
        cout << nextCheckpointX << " " << nextCheckpointY << " ";
        if (bust_now)
            cout << "BOOST" << endl;
        else
            cout << speed << endl;
    }
}