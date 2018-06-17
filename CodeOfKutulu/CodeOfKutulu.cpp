#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cassert>

using namespace std;

class Point
{
public:
	Point() : m_x( 0 ), m_y( 0 ) {}
	Point( int x, int y ) : m_x( x ), m_y( y ) {}

	int GetX() const { return m_x; }
	int GetY() const { return m_y; }

	void Set( int x, int y ) { m_x = x; m_y = y; }

private:
	int m_x;
	int m_y;
};

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

	Cell GetCell( const Point& point ) const { return m_cells[ GetIndex( point ) ]; }
	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }

private:
	void LoadSize( istream& in );
	void LoadLine( int lineIdx, istream& in );

	int GetIndex( const Point& point ) const
	{
		assert( point.GetX() >= 0 && point.GetX() < m_width );
		assert( point.GetY() >= 0 && point.GetY() < m_height );
		return point.GetX() + point.GetY() * m_height;
	}
	void SetCell( const Point& point, Cell cell )
	{
		m_cells[ GetIndex( point ) ] = cell;
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
			SetCell( Point( i, lineIdx ), Cell::Wall );
			break;
		case 'w':
			SetCell( Point( i, lineIdx ), Cell::Spawn );
			break;
		case '.':
			SetCell( Point( i, lineIdx ), Cell::Empty );
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
			switch ( grid.GetCell( Point( x, y ) ) )
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
