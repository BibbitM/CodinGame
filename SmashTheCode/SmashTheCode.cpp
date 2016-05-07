#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

// aaa


int first_highest_column_with_color(int (&myGrid)[12][6], int color)
{
    int column = -1;
    int height = 13;
    
    for (int j = 0; j < 6; ++j)
    {
        for (int i = 0; i < 12; ++i)
        {
            if (myGrid[i][j] == color)
            {
                if (i < height)
                {
                    height = i;
                    column = j;
                }
                break;
            }
        }
    }
    return column;
}


int first_column_with_color(int (&myGrid)[12][6], int color)
{
    for (int j = 0; j < 6; ++j)
    {
        for (int i = 11; i >= 0; --i)
        {
            if (myGrid[i][j] == color)
                return j;
        }
    }
    return -1;
}

int last_column_with_color(int (&myGrid)[12][6], int color)
{
    for (int j = 5; j >= 0; --j)
    {
        for (int i = 11; i >= 0; --i)
        {
            if (myGrid[i][j] == color)
                return j;
        }
    }
    return -1;
}


int first_lowest_column(int (&myGrid)[12][6])
{
    int column = 0;
    int height = 12;
    
    for (int j = 0; j < 6; ++j)
    {
        for (int i = 11; i >= 0; --i)
        {
            if (myGrid[i][j] == -1)
            {
                if (i < height)
                {
                    height = i;
                    column = j;
                }
                break;
            }
        }
    }

    return column;
}


/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/
int main()
{

    // game loop
    while (1) {
        int colorsA[8] = { 0 };
        int colorsB[8] = { 0 };
        
        int myGrid[12][6] = { 0 };

        for (int i = 0; i < 8; i++)
        {
            cin >> colorsA[i] >> colorsB[i]; cin.ignore();
        }
        for (int i = 0; i < 12; i++)
        {
            string row;
            cin >> row; cin.ignore();
            
            for (size_t j = 0; j < 6; ++j)
            {
                if (row[j] == '.')
                    myGrid[i][j] = -1;
                else
                    myGrid[i][j] = (int)(row[j] - '0');
                    
                cerr << (char)((int)'0' + myGrid[i][j]);
            }
            cerr << endl;
        }
        
        for (int i = 0; i < 12; i++) {
            string row; // One line of the map ('.' = empty, '0' = skull block, '1' to '5' = colored block)
            cin >> row; cin.ignore();
        }
        
        int col = first_highest_column_with_color(myGrid, colorsA[0]);
        cerr << col << endl;
        if (col == -1)
            col = first_lowest_column(myGrid);
        else if (col >= 0 && col < 5)
            col += 1;

        cout << char((int)'0' + col) << " 3" << endl; // "x": the column in which to drop your blocks

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;
        /*
        else if      (colorsA[0] == 1)
            cout << "0 1" << endl; // "x": the column in which to drop your blocks
        else if (colorsA[0] == 2)
            cout << "1 1" << endl; // "x": the column in which to drop your blocks
        else if (colorsA[0] == 3)
            cout << "2 1" << endl; // "x": the column in which to drop your blocks
        else if (colorsA[0] == 4)
            cout << "3 1" << endl; // "x": the column in which to drop your blocks
        else if (colorsA[0] == 5)
            cout << "4 1" << endl; // "x": the column in which to drop your blocks
        else
            cout << "5 1" << endl; // "x": the column in which to drop your blocks
        */

    }
}