#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <limits>

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

    int prev_x = std::numeric_limits<int>::min();
    int prev_y = std::numeric_limits<int>::min();

    bool stop_accelerate = false;

    // game loop
    while (1)
    {
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

        int speed_x;
        int speed_y;

        int out_speed_x;
        int out_speed_y = 0;

        if (prev_x == std::numeric_limits<int>::min()/* || prev_y == std::numeric_limits<int>::min()*/)
        {
            speed_x = 0;
            speed_y = 0;
        }
        else
        {
            speed_x = x - prev_x;
            speed_y = y - prev_y;
        }

        prev_x = x;
        prev_y = y;

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;

        static const int MAX_SPEED = 100;
        int speed = MAX_SPEED;
        
        bool bust_now = false;

        if (!stop_accelerate && speed_x > 400)
            stop_accelerate = true;

        //if (stop_accelerate)
        //    speed = 0;

        out_speed_x = (int)(speed_x * 0.85f) + speed;

        /*
        speed = clamp(speed, 5, nextCheckpointDist / 3);
        
        if (nextCheckpointAngle > 90 || nextCheckpointAngle < -90)
            speed = 0;

        if (!busted && nextCheckpointAngle < 10 && nextCheckpointAngle > -10 && speed == MAX_SPEED && nextCheckpointDist > 200)
        {
            bust_now = true;
            busted = false;
        }
        */

        // You have to output the target position
        // followed by the power (0 <= thrust <= 100)
        // i.e.: "x y thrust"
        cout << x + 10000 << " " << y << " ";
        if (bust_now)
            cout << "BOOST";
        else
            cout << speed;

        cout << " Speed: " << speed_x << ", " << speed_y;
        cout << " Out Speed: " << out_speed_x << ", " << out_speed_y;

        cout << endl;
    }
}