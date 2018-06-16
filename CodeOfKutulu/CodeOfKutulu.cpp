#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cassert>

using namespace std;

class Grid
{
public:
	enum class Cell
	{
		Wall,
		Spawn,
		Empty,
	};

	Grid();

	void Load( istream& in );

	Cell GetCell( int x, int y ) const { return m_cells[ GetIndex( x, y ) ]; }
	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }

private:
	void LoadSize( istream& in );
	void LoadLine( int lineIdx, istream& in );

	int GetIndex( int x, int y ) const
	{
		assert( x >= 0 && x < m_width );
		assert( y >= 0 && y < m_height );
		return x + y * m_height;
	}
	void SetCell( int x, int y, Cell cell )
	{
		m_cells[ GetIndex( x, y ) ] = cell;
	}

	int m_width;
	int m_height;

	vector< Cell > m_cells;
};

ostream& operator << ( ostream& out, const Grid& grid );

/**
 * Survive the wrath of Kutulu
 * Coded fearlessly by JohnnyYuge & nmahoude (ok we might have been a bit scared by the old god...but don't say anything)
 **/
int main()
{
	Grid grid;
	grid.Load( cin );
	cerr << grid;

    int sanityLossLonely; // how much sanity you lose every turn when alone, always 3 until wood 1
    int sanityLossGroup; // how much sanity you lose every turn when near another player, always 1 until wood 1
    int wandererSpawnTime; // how many turns the wanderer take to spawn, always 3 until wood 1
    int wandererLifeTime; // how many turns the wanderer is on map after spawning, always 40 until wood 1
    cin >> sanityLossLonely >> sanityLossGroup >> wandererSpawnTime >> wandererLifeTime; cin.ignore();

    // game loop
    while (1) {
        int entityCount; // the first given entity corresponds to your explorer
        cin >> entityCount; cin.ignore();
        for (int i = 0; i < entityCount; i++) {
            string entityType;
            int id;
            int x;
            int y;
            int param0;
            int param1;
            int param2;
            cin >> entityType >> id >> x >> y >> param0 >> param1 >> param2; cin.ignore();
        }

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;

        cout << "MOVE 1 1" << endl; // MOVE <x> <y> | WAIT
    }
}

Grid::Grid()
	: m_width( 0 ), m_height( 0 )
{}

void Grid::Load( istream& in )
{
	LoadSize( in );

	for ( int i = 0; i < m_height; i++ )
	{
		LoadLine( i, in );
	}
}

void Grid::LoadSize( istream& in )
{
	in >> m_width; in.ignore();
	in >> m_height; in.ignore();

	m_cells.resize( m_width * m_height );
}

void Grid::LoadLine( int lineIdx, istream& in )
{
	string line;
	getline( in, line );

	for ( int i = 0; i < m_width; ++i )
	{
		switch ( line[ i ] )
		{
		case '#':
			SetCell( i, lineIdx, Cell::Wall );
			break;
		case 'w':
			SetCell( i, lineIdx, Cell::Spawn );
			break;
		case '.':
			SetCell( i, lineIdx, Cell::Empty );
			break;
		default:
			assert( false );
		}
	}
}

ostream& operator << ( ostream& out, const Grid& grid )
{
	for ( int x = 0; x < grid.GetWidth(); ++x )
	{
		for ( int y = 0; y < grid.GetHeight(); ++y )
		{
			switch ( grid.GetCell( x, y ) )
			{
			case Grid::Cell::Wall:
				out << '#';
				break;
			case Grid::Cell::Spawn:
				out << 'w';
				break;
			case Grid::Cell::Empty:
				out << '.';
				break;
			}
		}
		out << endl;
	}

	return out;
}