#include <iostream>
#include <array>
#include <cassert>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

static const size_t ROTS = 4;
static const size_t MOVES = 8;

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

inline int GetColorBonus(int colors)
{
    switch (colors)
    {
    case 2:
        return 2;
    case 3:
        return 4;
    case 4:
        return 8;
    case 5:
        return 16;
    default:
        return 0;
    }
}


class Grid
{
public:
    static const size_t COLS = 6;
    static const size_t ROWS = 12;

    Grid()
    {
        //for (auto& row : m_Grid)
        //    row.fill(Block::Empty);
    }

    /// Allows direct read access to grid.
    //const array<Block, COLS>& operator[](size_t idx) const { assert(idx < ROWS); return m_Grid[idx]; }
    const Block* operator[](size_t idx) const { assert(idx < ROWS); return m_Grid[idx]; }

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

    size_t GetHeight(size_t row) const
    {
        if (row >= ROWS)
            return 0;

        size_t col = 0;
        while (col < COLS && m_Grid[row][col] != Block::Empty)
            ++col;

        return col;
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
    size_t AddBlock(size_t col, Block color)
    {
        if (col >= COLS)
            return (size_t)-1;

        for (size_t row = 0; row < ROWS; ++row)
        {
            if (m_Grid[row][col] == Block::Empty)
            {
                m_Grid[row][col] = color;
                return row;
            }
        }

        return (size_t)-1;
    }

    size_t CalculateBlocksNumber(Block block) const
    {
        size_t num = 0;
        for (size_t row = 0; row < ROWS; ++row)
        {
            for (size_t col = 0; col < COLS; ++col)
            {
                if (Get(row, col) == block)
                    ++num;
            }
        }

        return num;
    }

    /// Calculates current grid rate.
    /** This is custom method of rating grid. */
    int CalculateRate() const
    {
        static const int MUL = ROWS*ROWS;

        int rate = 0;

        size_t prev = 0;
        size_t curr = GetHeight((size_t)-1);
        size_t next = GetHeight(0);
        for (size_t row = 0; row < ROWS; ++row)
        {
            prev = curr;
            curr = next;
            next = GetHeight(row + 1);

            static const int HEIGHT_DIFF_BONUS = 32 * MUL;

            if (curr - prev >= 2 && curr - prev <= 3)
                rate += HEIGHT_DIFF_BONUS;
            if (prev - curr >= 2 && prev - curr <= 3)
                rate += HEIGHT_DIFF_BONUS;

            if (curr - next >= 2 && curr - next <= 3)
                rate += HEIGHT_DIFF_BONUS;
            if (next - curr >= 2 && next - curr <= 3)
                rate += HEIGHT_DIFF_BONUS;
        }
        //for_each_row_and_col([&](size_t row, size_t col)
        for (size_t row = 0; row < ROWS; ++row)
            for (size_t col = 0; col < COLS; ++col)
        {
            Block block = Get(row, col);

            //if (IsEmpty(block))
            //    rate += 64/*(ROWS - row)*/ * MUL;

            if (IsSkull(block))
            {
                //rate -= MUL;

                if (IsColor(Get(row + 1, col)))
                    rate += MUL;
                if (IsColor(Get(row - 1, col)))
                    rate += MUL;
                if (IsColor(Get(row, col + 1)))
                    rate += MUL;
                if (IsColor(Get(row, col - 1)))
                    rate += MUL;
            }

            if (IsColor(block))
            {
                // .
                // .
                // x  x..
                if (Get(row + 1, col + 0) == block &&
                    Get(row + 2, col + 0) == block)
                    rate += 32 * MUL;
                if (Get(row + 0, col + 1) == block &&
                    Get(row + 0, col + 2) == block)
                    rate += 32 * MUL;

                //  .  .    .  .
                //  .  .   .    .
                // x    x  x    x
                if (Get(row + 1, col + 1) == block &&
                    Get(row + 2, col + 1) == block)
                    rate += 64 * MUL;
                if (Get(row + 1, col - 1) == block &&
                    Get(row + 2, col - 1) == block)
                    rate += 64 * MUL;
                if (Get(row + 1, col + 0) == block &&
                    Get(row + 2, col + 1) == block)
                    rate += 64 * MUL;
                if (Get(row + 1, col + 0) == block &&
                    Get(row + 2, col - 1) == block)
                    rate += 64 * MUL;

                //  ..  ..     x  x
                // x      x  ..    ..
                if (Get(row + 1, col + 1) == block &&
                    Get(row + 1, col + 2) == block)
                    rate += 32 * MUL;
                if (Get(row + 1, col - 1) == block &&
                    Get(row + 1, col - 2) == block)
                    rate += 32 * MUL;
                if (Get(row - 1, col - 1) == block &&
                    Get(row - 1, col - 2) == block)
                    rate += 32 * MUL;
                if (Get(row - 1, col + 1) == block &&
                    Get(row - 1, col + 2) == block)
                    rate += 32 * MUL;

                // ..  ..   x  x
                // x    x  ..  ..
                if (Get(row + 1, col + 0) == block &&
                    Get(row + 1, col + 1) == block)
                    rate += 16 * MUL;
                if (Get(row + 1, col + 0) == block &&
                    Get(row + 1, col - 1) == block)
                    rate += 16 * MUL;
                if (Get(row - 1, col + 0) == block &&
                    Get(row - 1, col - 1) == block)
                    rate += 16 * MUL;
                if (Get(row - 1, col + 0) == block &&
                    Get(row - 1, col + 1) == block)
                    rate += 16 * MUL;

                //// .      .  . x  x .
                //// . x  x .  .      .
                //if (Get(row + 0, col - 2) == block &&
                //    Get(row + 1, col - 2) == block)
                //    rate += 2 * MUL;
                //if (Get(row + 0, col + 2) == block &&
                //    Get(row + 1, col + 2) == block)
                //    rate += 2 * MUL;
                //if (Get(row - 0, col - 2) == block &&
                //    Get(row - 1, col - 2) == block)
                //    rate += 2 * MUL;
                //if (Get(row - 0, col + 2) == block &&
                //    Get(row - 1, col + 2) == block)
                //    rate += 2 * MUL;

                // ..  ..   x  x
                //
                //  x  x   ..  ..
                if (Get(row + 2, col - 0) == block &&
                    Get(row + 2, col - 1) == block)
                    rate += 4 * MUL;
                if (Get(row + 2, col + 0) == block &&
                    Get(row + 2, col + 1) == block)
                    rate += 4 * MUL;
                if (Get(row - 2, col - 0) == block &&
                    Get(row - 2, col - 1) == block)
                    rate += 4 * MUL;
                if (Get(row - 2, col + 0) == block &&
                    Get(row - 2, col + 1) == block)
                    rate += 4 * MUL;

                //     .
                // x.  x
                if (Get(row, col + 1) == block)
                    rate += 1 * MUL;
                if (Get(row + 1, col) == block)
                    rate += 128 * MUL;

                // .
                // .
                //
                // x
                if (Get(row + 1, col) != block &&
                    Get(row + 2, col) == block &&
                    Get(row + 3, col) == block)
                    rate += 128 * MUL;

                // .
                // +
                // +
                // x
                if (Get(row + 1, col) != block &&
                    IsColor(Get(row + 1, col)) &&
                    Get(row + 2, col) == Get(row + 1, col) &&
                    Get(row + 3, col) == block)
                    rate += 128 * MUL;

                // .
                // s
                // x
                if (IsSkull(Get(row + 1, col)) &&
                    Get(row + 3, col) == block)
                    rate += 128 * MUL;
            }
        }//);

        return rate;
    }
    
    /// Simulates next move and calculates score.
    int Simulate(size_t row1, size_t col1, size_t row2, size_t col2)
    {
        int blocks_count = 0;
        int chain_power = -1;
        int group_bonus = 0;
        bool colors[(size_t)Block::Count] = { false };

        size_t min_row = (size_t)max<ptrdiff_t>(0, min<ptrdiff_t>(row1, row2) - 2);
        size_t max_row = min((size_t)ROWS, max(row1, row2) + 3);

        size_t min_col = (size_t)max<ptrdiff_t>(0, min<ptrdiff_t>(col1, col2) - 2);
        size_t max_col = min((size_t)COLS, max(col1, col1) + 3);

        do
        {
            const int prev_blocks_count = blocks_count;

            //for_each_row_and_col([&](size_t row, size_t col)
            for (size_t row = min_row; row < max_row; ++row)
                for (size_t col = min_col; col < max_col; ++col)
            {
                if (IsAtLeastFourColorBlocks(row, col))
                {
                    colors[(size_t)Get(row, col)] = true;

                    int blocks = ClearBlocks(row, col);

                    blocks_count += blocks;
                    group_bonus += GetGroupBonus(blocks);
                }
            }//);

            min_row = 0;
            max_row = ROWS;

            min_col = 0;
            max_col = COLS;

            if (blocks_count != prev_blocks_count)
            {
                Colapse();

                if (chain_power == -1)
                    chain_power = 0;
                else if (chain_power == 0)
                    chain_power = 8;
                else
                    chain_power *= 2;

                continue;
            }

            break;

        } while (true);

        if (blocks_count == 0)
            return 0;

        int colors_count = 0;
        for (size_t i = 0; i < countof(colors); ++i)
        {
            if (colors[i])
                ++colors_count;
        }
        int color_bonus = GetColorBonus(colors_count);

        return (10 * blocks_count) * max(chain_power + color_bonus + group_bonus, 1);
    }

    void Colapse()
    {
        for (size_t col = 0; col < COLS; ++col)
        {
            for (size_t row = 0, copy_row = 0; row < ROWS; ++row, ++copy_row)
            {
                while (copy_row < ROWS && Get(copy_row, col) == Block::Empty)
                    ++copy_row;

                if (row != copy_row)
                    Set(row, col, Get(copy_row, col));
            }
        }
    }

    bool IsAtLeastFourColorBlocks(size_t row, size_t col) const
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
        static string line;
        for (size_t i = 0; i < ROWS; ++i)
        {
            in >> line; in.ignore();

            ReadRow(ROWS - 1 - i, line);
        }
    }

    void Read(string (&lines)[ROWS])
    {
        for (size_t i = 0; i < ROWS; ++i)
        {
            ReadRow(ROWS - 1 - i, lines[i]);
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

    //matrix<Block, ROWS, COLS> m_Grid;
    Block m_Grid[ROWS][COLS];
};

int CalculateMove(const Grid& grid, size_t col, size_t rot, Block(&colorsA)[MOVES], Block(&colorsB)[MOVES], int deep, int max_deep, int deep_bonus);

int CalculateBestMove(const Grid& grid, Block(&colorsA)[MOVES], Block(&colorsB)[MOVES], int deep, int max_deep, int deep_bonus)
{
    int values[Grid::COLS * ROTS] = { 0 };

    for (size_t col = 0; col < Grid::COLS; ++col)
    {
        for (size_t rot = 0; rot < ROTS; ++rot)
        {
            values[col * ROTS + rot] = CalculateMove(grid, col, rot, colorsA, colorsB, deep, max_deep, deep_bonus);
        }
    }

    return *max_element(begin(values), end(values));
}

inline bool AddBlocksToGrid(Grid& calc_grid, size_t col, size_t rot, Block(&colorsA)[MOVES], Block(&colorsB)[MOVES], int deep, size_t& row1, size_t& col1, size_t& row2, size_t& col2)
{
    if (rot == 0)
    {
        col1 = col;
        col2 = col + 1;
        row1 = calc_grid.AddBlock(col, colorsA[deep]);
        row2 = calc_grid.AddBlock(col + 1, colorsB[deep]);
    }
    else if (rot == 1)
    {
        col1 = col;
        col2 = col;
        row1 = calc_grid.AddBlock(col, colorsA[deep]);
        row2 = calc_grid.AddBlock(col, colorsB[deep]);
    }
    else if (rot == 2)
    {
        col1 = col - 1;
        col2 = col;
        row1 = calc_grid.AddBlock(col - 1, colorsB[deep]);
        row2 = calc_grid.AddBlock(col, colorsA[deep]);
    }
    else //if (rot == 3)
    {
        col1 = col;
        col2 = col;
        row1 = calc_grid.AddBlock(col, colorsB[deep]);
        row2 = calc_grid.AddBlock(col, colorsA[deep]);
    }

    return row1 != (size_t)-1 && row2 != (size_t)-1;
}

int CalculateMove(const Grid& grid, size_t col, size_t rot, Block(&colorsA)[MOVES], Block(&colorsB)[MOVES], int deep, int max_deep, int deep_bonus)
{
    Grid calc_grid = grid;

    int val = numeric_limits<int>::min();

    size_t row1, col1, row2, col2;
    bool added = AddBlocksToGrid(calc_grid, col, rot, colorsA, colorsB, deep, row1, col1, row2, col2);

    if (added)
    {
        int rate = calc_grid.CalculateRate();
        int score = calc_grid.Simulate(row1, col1, row2, col2);
        val = rate + (score + (score ? (/*MOVES - */deep) * deep_bonus : 0)) * numeric_limits<short>::max();

        if (deep < max_deep)//MOVES)
        {
            int best_val = CalculateBestMove(calc_grid, colorsA, colorsB, deep + 1, max_deep, deep_bonus);
            if (best_val > 0)
                val += best_val;
            //else
            //    val = best_val;
        }
    }

    return val;
}

size_t FindBestMove(const Grid& grid, Block (&colorsA)[MOVES], Block(&colorsB)[MOVES], int max_deep, int deep_bonus)
{
    int values[Grid::COLS * ROTS] = { 0 };

    for (size_t col = 0; col < Grid::COLS; ++col)
    {
        for (size_t rot = 0; rot < ROTS; ++rot)
        {
            values[col * ROTS + rot] = CalculateMove(grid, col, rot, colorsA, colorsB, 0, max_deep, deep_bonus);
        }
    }

    auto max_it = max_element(begin(values), end(values));
    size_t max_idx = distance(begin(values), max_it);

    return max_idx;
}

int CalculateScore(const Grid& grid, size_t col, size_t rot, Block(&colorsA)[MOVES], Block(&colorsB)[MOVES])
{
    Grid calc_grid = grid;

    int val = numeric_limits<int>::min();

    size_t row1, col1, row2, col2;
    bool added = AddBlocksToGrid(calc_grid, col, rot, colorsA, colorsB, 0, row1, col1, row2, col2);

    if (added)
    {
        val = calc_grid.Simulate(row1, col1, row2, col2);
    }

    return val;
}

int CalculateNextMaxScore(const Grid& grid, Block(&colorsA)[MOVES], Block(&colorsB)[MOVES])
{
    int values[Grid::COLS * ROTS] = { 0 };

    for (size_t col = 0; col < Grid::COLS; ++col)
    {
        for (size_t rot = 0; rot < ROTS; ++rot)
        {
            values[col * ROTS + rot] = CalculateScore(grid, col, rot, colorsA, colorsB);
        }
    }

    auto max_it = max_element(begin(values), end(values));

    return *max_it;
}

/**
* Auto-generated code below aims at helping you parse
* the standard input according to the problem statement.
**/
int main()
{
    if (0)
    {
        string lines[] = {
            "......",
            "......",
            "......",
            "......",
            "......",
            ".2....",
            ".0....",
            ".1....",
            "045...",
            "345...",
            "325...",
            "150000",
            //"1.....",
            //"2.....",
            //"1.....",
            //"1.....",
            //"1.....",
            //"2.....",
            //"22....",
            //"53....",
            //"55....",
            //"44....",
            //"315...",
            //"311..."
        };
        Grid MyGrid;

        Block colorsA[MOVES] = { Block::Color5, Block::Color2, Block::Color4, Block::Color1, Block::Color5, Block::Color2, Block::Color2, Block::Color1 };
        Block colorsB[MOVES] = { Block::Color3, Block::Color4, Block::Color4, Block::Color1, Block::Color3, Block::Color1, Block::Color3, Block::Color5 };

        MyGrid.Read(lines);
        MyGrid.Print(cerr);

        size_t best_idx = FindBestMove(MyGrid, colorsA, colorsB, 2, 1);

        int col = best_idx / ROTS;
        int rot = best_idx % ROTS;

        cout << col << ' ' << rot << endl; // "x": the column in which to drop your blocks

        return 0;
    }

    Grid MyGrid;
    Grid OtherGrid;

    // game loop
    while (1) {
        Block colorsA[MOVES] = { Block::Skull };
        Block colorsB[MOVES] = { Block::Skull };

        for (int i = 0; i < MOVES; i++)
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

        int max_deep = 2;
        int deep_bonus = 1;

        int other_best_score = CalculateNextMaxScore(OtherGrid, colorsA, colorsB);

        if (other_best_score >= 6 * 70 * 4)
            deep_bonus = 0;

        if (other_best_score >= 6 * 70 * 6)
            max_deep = 0;
        else if (other_best_score >= 6 * 70 * 3)
            max_deep = 1;

        size_t skulls_num = MyGrid.CalculateBlocksNumber(Block::Skull);
        if (skulls_num >= 6 * 2)
            deep_bonus = 0;
        size_t empty_num = MyGrid.CalculateBlocksNumber(Block::Empty);;
        if (empty_num <= 6 * 2)
            deep_bonus = 0;

        size_t best_idx = FindBestMove(MyGrid, colorsA, colorsB, max_deep, deep_bonus);

        int col = best_idx / ROTS;
        int rot = best_idx % ROTS;

        cout << col << ' ' << rot << endl; // "x": the column in which to drop your blocks
    }
}
