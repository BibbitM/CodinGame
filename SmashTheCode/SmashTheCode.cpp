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

inline Block BlockFromChar(char ch)
{
    if (ch == '.')
        return Block::Empty;
    else
        return (Block)(ch - '0');
}

inline char CharFromBlock(Block block)
{
    if (block == Block::Empty)
        return '.';
    else
        return (char)((int)'0' + (int)block);
}

inline bool IsEmpty(Block block)
{
    return block == Block::Empty;
}

inline bool IsSkull(Block block)
{
    return block == Block::Skull;
}

inline bool IsColor(Block block)
{
    return block != Block::Empty && block != Block::Skull;
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

    /// Allows direct read access to grid.
    const array<Block, COLS>& operator[](size_t idx) const { assert(idx < ROWS); return m_Grid[idx]; }

    /// Gets value of specified grid cell.
    /** If row or col is outside bounds then returns Block::Empty. */
    Block Get(size_t row, size_t col) const
    {
        if (row >= ROWS || col >= COLS)
            return Block::Empty;

        return m_Grid[row][col];
    }

    template <typename FUNC>
    void for_each_row_and_col(FUNC fun) const
    {
        for (size_t row = 0; row < ROWS; ++row)
            for (size_t col = 0; col < COLS; ++col)
                fun(row, col);
    }

    size_t GetNumRows() const { return ROWS; }
    size_t GetNumCols() const { return COLS; }

    /// Adds new block to grid.
    /** @return false if failed column is full. */
    bool AddBlock(size_t col, Block color)
    {
        if (col >= COLS)
            return false;

        for (size_t row = 0; row < ROWS; ++row)
        {
            if (m_Grid[row][col] == Block::Empty)
            {
                m_Grid[row][col] = color;
                return true;
            }
        }

        return false;
    }

    /// Calculates current grid rate.
    /** This is custom method of rating grid. */
    int CalculateRate() const
    {
        int rate = 0;
        for_each_row_and_col([&](size_t row, size_t col)
        {
            Block block = Get(row, col);
            if (!IsColor(block))
                return;

            if (Get(row + 1, col) == block)
                ++rate;
            if (Get(row, col + 1) == block)
                ++rate;
        });

        return rate;
    }

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
        //MyGrid.Print(cerr);

        OtherGrid.Read(cin);
        //OtherGrid.Print(cerr);

        static const size_t ROTS = 4;
        int values[Grid::COLS * ROTS] = { 0 };

        for (size_t col = 0; col < Grid::COLS; ++col)
        {
            for (size_t rot = 0; rot < ROTS; ++rot)
            {
                Grid CalcGrid = MyGrid;

                int& val = values[col * ROTS + rot];
                val = numeric_limits<int>::min();

                if (rot == 0)
                {
                    if (!CalcGrid.AddBlock(col, colorsA[0]))
                        break;
                    if (!CalcGrid.AddBlock(col + 1, colorsB[0]))
                        break;
                }
                else if (rot == 1)
                {
                    if (!CalcGrid.AddBlock(col, colorsA[0]))
                        break;
                    if (!CalcGrid.AddBlock(col, colorsB[0]))
                        break;
                }
                else if (rot == 2)
                {
                    if (!CalcGrid.AddBlock(col - 1, colorsB[0]))
                        break;
                    if (!CalcGrid.AddBlock(col, colorsA[0]))
                        break;
                }
                else //if (rot == 3)
                {
                    if (!CalcGrid.AddBlock(col, colorsB[0]))
                        break;
                    if (!CalcGrid.AddBlock(col, colorsA[0]))
                        break;
                }

                val = CalcGrid.CalculateRate();
            }
        }

        auto min_it = max_element(begin(values), end(values));
        size_t min_idx = distance(begin(values), min_it);

        int col = min_idx / ROTS;
        int rot = min_idx % ROTS;

        cout << col << ' ' << rot << endl; // "x": the column in which to drop your blocks
    }
}
