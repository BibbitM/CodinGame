#include <iostream>
#include <array>
#include <cassert>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

template <typename T, std::size_t N>
constexpr std::size_t countof(T const (&)[N]) noexcept
{
    return N;
}

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
    Count,
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
    return block != Block::Empty && block != Block::Skull && block != Block::Count;
}

inline int GetGroupBonus(int group)
{
    if (group <= 4)
        return 0;

    if (group <= 10)
        return group - 4;

    return 8;

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
    /// Sets value of specific grid cell.
    void Set(size_t row, size_t col, Block block)
    {
        if (row >= ROWS || col >= COLS)
            return;

        m_Grid[row][col] = block;
    }
    /// Clears vale of specific grid cell.
    void Clear(size_t row, size_t col)
    {
        if (row >= ROWS || col >= COLS)
            return;

        m_Grid[row][col] = Block::Empty;
    }

    template <typename FUNC>
    void for_each_row_and_col(FUNC func) const
    {
        for (size_t row = 0; row < ROWS; ++row)
            for (size_t col = 0; col < COLS; ++col)
                func(row, col);
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

            if (IsEmpty(block))
                rate += (ROWS - row);

            if (IsColor(block))
            {
                static const int MUL = ROWS*ROWS;

                if (Get(row + 1, col) == block)
                    rate += 1 * MUL;
                if (Get(row, col + 1) == block)
                    rate += 1 * MUL;
            }
        });

        return rate;
    }
    
    /// Simulates next move and calculates score.
    int Simulate()
    {
        int blocks_count = 0;
        int chain_power = 0;
        int group_bonus = 0;
        bool colors[(size_t)Block::Count] = { false };

        do
        {
            const int prev_blocks_count = blocks_count;

            for_each_row_and_col([&](size_t row, size_t col)
            {
                if (!IsAtLeastForColorBlocks(row, col))
                    return;

                colors[(size_t)Get(row, col)] = true;

                int blocks = ClearBlocks(row, col);

                blocks_count += blocks;
                group_bonus += GetGroupBonus(blocks);
            });

            if (blocks_count != prev_blocks_count)
            {
                chain_power = max(chain_power * 2, 8);
                continue;
            }

        } while (false);

        if (blocks_count)
            return 0;

        int color_bonus = 0;
        for (size_t i = 0; i < countof(colors); ++i)
        {
            if (colors[i])
                ++color_bonus;
        }

        return (10 * blocks_count) * max(chain_power + color_bonus + group_bonus, 0);
    }

    bool IsAtLeastForColorBlocks(size_t row, size_t col) const
    {
        Block block = Get(row, col);
        if (!IsColor(block))
            return false;

        int group = 1;

        int shiftA[4][2] = { { 1, 1 }, { 1, -1}, { -1, 1 }, { -1, -1} };
        int shiftB[4][2] = { { 1, 0 }, { -1, 0}, { 0, 1 }, { 0, -1} };

        for (size_t i = 0; i < 4; ++i)
        {
            if (Get(row + shiftA[i][0], col + shiftA[i][1]) == block &&
                (Get(row + shiftA[i][0], col) == block || Get(row, col + shiftA[i][1]) == block))
            {
                ++group;
            }

            if (Get(row + shiftB[i][0], col + shiftB[i][1]) == block)
            {
                ++group;
                if (Get(row + shiftB[i][0] * 2, col + shiftB[i][1] * 2) == block)
                    ++group;
            }
        }

        return group >= 4;
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
    int ClearBlocks(size_t row, size_t col)
    {
        int cleared_blocks = 1;

        Block block = Get(row, col);
        Clear(row, col);

        int shift[4][2] = { { 1, 0 },{ -1, 0 },{ 0, 1 },{ 0, -1 } };

        for (size_t i = 0; i < 4; ++i)
        {
            if (Get(row + shift[i][0], col + shift[i][1]) == block)
                cleared_blocks += ClearBlocks(row + shift[i][0], col + shift[i][1]);
            else if (Get(row + shift[i][0], col + shift[i][1]) == Block::Skull)
                Clear(row + shift[i][0], col + shift[i][1]);
        }

        return cleared_blocks;
    }

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

                bool added = false;
                if (rot == 0)
                {
                    added = CalcGrid.AddBlock(col, colorsA[0]) &&
                            CalcGrid.AddBlock(col + 1, colorsB[0]);
                }
                else if (rot == 1)
                {
                    added = CalcGrid.AddBlock(col, colorsA[0]) &&
                            CalcGrid.AddBlock(col, colorsB[0]);
                }
                else if (rot == 2)
                {
                    added = CalcGrid.AddBlock(col - 1, colorsB[0]) &&
                            CalcGrid.AddBlock(col, colorsA[0]);
                }
                else //if (rot == 3)
                {
                    added = CalcGrid.AddBlock(col, colorsB[0]) &&
                            CalcGrid.AddBlock(col, colorsA[0]);
                }

                if (added)
                    val = CalcGrid.CalculateRate() + CalcGrid.Simulate() * numeric_limits<short>::max();
            }
        }

        auto min_it = max_element(begin(values), end(values));
        size_t min_idx = distance(begin(values), min_it);

        int col = min_idx / ROTS;
        int rot = min_idx % ROTS;

        cout << col << ' ' << rot << endl; // "x": the column in which to drop your blocks
    }
}
