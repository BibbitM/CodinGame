#include <iostream>
#include <array>
#include <cassert>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

template <class T, size_t ROW, size_t COL>
using matrix = array<array<T, COL>, ROW>;

enum class Block
{
    Empty = -1,
    Skull,
    Color1,
    Color2,
    Color3,
    Color4,
    Color5,
};

Block BlockFromChar(char ch)
{
    if (ch == '.')
        return Block::Empty;
    else
        return (Block)(ch - '0');
}

char CharFromBlock(Block block)
{
    if (block == Block::Empty)
        return '.';
    else
        return (char)((int)'0' + (int)block);
}


class Grid
{
public:
    static const size_t COLS = 6;
    static const size_t ROWS = 12;

    Grid()
    {
        for (auto& row : m_Grid)
            row.fill(Block::Empty);
    }

    const array<Block, COLS>& operator[](size_t idx) const { assert(idx < ROWS); return m_Grid[idx]; }

    size_t GetRows() const { return ROWS; }
    size_t GetCols() const { return COLS; }

    void Read(istream& in)
    {
        string line;
        for (size_t i = 0; i < ROWS; ++i)
        {
            in >> line; in.ignore();

            ReadRow(ROWS - 1 - i, line);
        }
    }

    void Print(ostream& out)
    {
        for (size_t i = 0; i < ROWS; ++i)
        {
            size_t row = ROWS - 1 - i;
            for (size_t col = 0; col < COLS; ++col)
                out << CharFromBlock(m_Grid[row][col]);

            out << endl;
        }
    }

private:
    void ReadRow(size_t row, const string& line)
    {
        assert(row < ROWS);

        for (size_t i = 0; i < COLS; ++i)
            m_Grid[row][i] = BlockFromChar(line[i]);
    }

    matrix<Block, ROWS, COLS> m_Grid;
};

int first_highest_column_with_color(const Grid& grid, Block color)
{
    int column = -1;
    int height = -1;

    for (int j = 0; j < Grid::COLS; ++j)
    {
        for (int i = Grid::ROWS - 1; i >= 0; --i)
        {
            if (grid[i][j] == color)
            {
                if (i > height)
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


int first_column_with_color(int(&myGrid)[12][6], int color)
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

int last_column_with_color(int(&myGrid)[12][6], int color)
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


int first_lowest_column(const Grid& grid)
{
    int column = 0;
    int height = 12;

    for (int j = 0; j < Grid::COLS; ++j)
    {
        for (int i = 0; i < Grid::ROWS; ++i)
        {
            if (grid[i][j] == Block::Empty)
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
    Grid MyGrid;
    Grid OtherGrid;

    // game loop
    while (1) {
        Block colorsA[8] = { Block::Skull };
        Block colorsB[8] = { Block::Skull };

        for (int i = 0; i < 8; i++)
        {
            char colA, colB;
            cin >> colA >> colB; cin.ignore();

            colorsA[i] = BlockFromChar(colA);
            colorsB[i] = BlockFromChar(colB);
        }

        MyGrid.Read(cin);
        MyGrid.Print(cerr);

        OtherGrid.Read(cin);
        //OtherGrid.Print(cerr);

        int col = first_highest_column_with_color(MyGrid, colorsA[0]);
        cerr << col << endl;
        if (col == -1)
            col = first_lowest_column(MyGrid);
        else if (col >= 0 && col < 5)
            col += 1;

        cout << char((int)'0' + col) << " 3" << endl; // "x": the column in which to drop your blocks
    }
}