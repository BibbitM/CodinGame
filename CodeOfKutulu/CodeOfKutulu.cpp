#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cassert>
#include <cmath>

using namespace std;

template< typename T, std::size_t N >
constexpr std::size_t countof( T const ( & )[ N ] ) noexcept
{
	return N;
}

class Point
{
public:
	Point() : m_x( 0 ), m_y( 0 ) {}
	Point( int x, int y ) : m_x( x ), m_y( y ) {}

	int GetX() const { return m_x; }
	int GetY() const { return m_y; }

	void Set( int x, int y ) { m_x = x; m_y = y; }

	Point& operator += ( const Point& other )
	{
		m_x += other.m_x;
		m_y += other.m_y;
	}
	Point& operator -= ( const Point& other )
	{
		m_x -= other.m_x;
		m_y -= other.m_y;
	}

private:
	int m_x;
	int m_y;
};

// Calculates Manhattan distances between two points.
inline int Distance( const Point& p1, const Point& p2 )
{
	return abs( p1.GetX() - p2.GetX() ) + abs( p1.GetY() - p2.GetY() );
}

inline Point operator + ( const Point& p1, const Point& p2 )
{
	return Point( p1.GetX() + p2.GetX(), p1.GetY() + p2.GetY() );
}

inline Point operator - ( const Point& p1, const Point& p2 )
{
	return Point( p1.GetX() - p2.GetX(), p1.GetY() - p2.GetY() );
}

istream& operator >> ( istream& in, Point& point );
ostream& operator << ( ostream& out, const Point& point ); 

class Entity
{
public:
	enum class Type
	{
		Explorer,
		Wanderer,
		EffectPlan,
		EffectLight,
		Slasher,
		EffectShelter,
		EffectYell,
	};

	Entity() : m_position(), m_id( -1 ), m_type( Type::Explorer ) {}
	Entity( Type type, int id, const Point& position ) : m_type( type ), m_position( position ), m_id( id ) {}

	void SetEntity( const Entity& entity ) { *this = entity; }
	void Load( istream& in );

	Type GetType() const { return m_type; }
	void SetType( Type type ) { m_type = type; }

	int GetId() const { return m_id; }
	void SetId( int id ) { m_id = id; }

	const Point& GetPosition() const { return m_position; }
	void SetPosition( const Point& position ) { m_position = position; }

private:
	Point m_position;
	int m_id;
	Type m_type;
};

const char* ToString( Entity::Type type );

ostream& operator << ( ostream& out, const Entity& entity );

class Explorer : public Entity
{
public:
	Explorer() : Entity(), m_sanity( 0 ) {}
	Explorer( const Entity& entity ) : Entity( entity ), m_sanity( 0 ) {}

	void LoadParams( istream& in );

	int GetSanity() const { return m_sanity; }
	void SetSanity( int sanity ) { m_sanity = sanity; }

private:
	int m_sanity;

};

ostream& operator << ( ostream& out, const Explorer& explorer );

class Wanderer : public Entity
{
public:
	enum class State
	{
		Spawning,
		Wandering,
		Stalking,
		Rushing,
		Stunned,
	};
	Wanderer() : Entity(), m_timeBefore( 0 ), m_state( State::Spawning ), m_target( -1 ) {}
	Wanderer( const Entity& entity ) : Entity( entity ), m_timeBefore( 0 ), m_state( State::Spawning ), m_target( -1 ) {}

	void LoadParams( istream& in );

	int GetTimeBefore() const { return m_timeBefore; }
	void SetTimeBefore( int timeBefore ) { m_timeBefore = timeBefore; }

	State GetState() const { return m_state; }
	void SetState( State state ) { m_state = state; }

	int GetTarget() const { return m_target; }
	void SetTarget( int target ) { m_target = target; }

private:
	int m_timeBefore;
	State m_state;
	int m_target;
};

const char* ToString( Wanderer::State state );

ostream& operator << ( ostream& out, const Wanderer& wanderer );

class Grid;

class Player : public Explorer
{
public:
	Player() : Explorer(), m_targetPosition( 0, 0 ) {}

	const Point& GetTargetPosition() const { return m_targetPosition; }
	void SetTargetPosition( const Point& targetPosition ) { m_targetPosition = targetPosition; }

	void Update( const Grid& grid, const vector< Explorer >& explorers, const vector< Wanderer >& wanderers );

private:
	Point m_targetPosition;
};

class Grid
{
public:
	enum class Cell
	{
		Wall,
		Spawn,
		Shelter,
		Empty,
	};

	Grid();

	void Load( istream& in );

	Cell GetCell( const Point& point ) const { return m_cells[ GetIndex( point ) ]; }
	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }

	bool CanMove( const Point& point ) const
	{
		return IsValid( point ) && GetCell( point ) != Cell::Wall;
	}

private:
	void LoadSize( istream& in );
	void LoadLine( int lineIdx, istream& in );

	bool IsValid( const Point& point ) const
	{
		return point.GetX() >= 0 && point.GetX() < m_width
			&& point.GetY() >= 0 && point.GetY() < m_height;
	}
	int GetIndex( const Point& point ) const
	{
		assert( IsValid( point ) );
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

	Player player;

	vector< Explorer > explorers;
	vector< Wanderer > wanderers;

	// game loop
	while (1)
	{
		int entityCount; // the first given entity corresponds to your explorer
		cin >> entityCount; cin.ignore();

		assert( entityCount >= 1 );

		explorers.clear();
		explorers.reserve( entityCount - 1 );

		wanderers.clear();
		wanderers.reserve( entityCount - 1 );

		// load player
		player.Load( cin );
		player.LoadParams( cin );

		cerr << player << endl;

		for ( int i = 1; i < entityCount; ++i )
		{
			Entity entity;
			entity.Load( cin );

			if ( entity.GetType() == Entity::Type::Explorer )
			{
				explorers.push_back( entity );
				explorers.back().LoadParams( cin );

				cerr << explorers.back() << endl;
			}
			else if ( entity.GetType() == Entity::Type::Wanderer
				|| entity.GetType() == Entity::Type::Slasher )
			{
				wanderers.push_back( entity );
				wanderers.back().LoadParams( cin );

				cerr << wanderers.back() << endl;
			}
			else
			{
				int param0;
				int param1;
				int param2;
				cin >> param0 >> param1 >> param2; cin.ignore();
			}
		}

		player.Update( grid, explorers, wanderers );

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;

        cout << "MOVE " << player.GetTargetPosition() << endl; // MOVE <x> <y> | WAIT
    }
}


istream& operator >> ( istream& in, Point& point )
{
	int x, y;
	in >> x >> y;

	point.Set( x, y );

	return in;
}

ostream& operator << ( ostream& out, const Point& point )
{
	return out << point.GetX() << ' ' << point.GetY();
}


void Entity::Load( istream& in )
{
	string entityType;
	in >> entityType >> m_id >> m_position;

	if ( entityType == "WANDERER" )
		m_type = Type::Wanderer;
	else if ( entityType == "EXPLORER" )
		m_type = Type::Explorer;
	else if ( entityType == "EFFECT_PLAN" )
		m_type = Type::EffectPlan;
	else if ( entityType == "EFFECT_LIGHT" )
		m_type = Type::EffectLight;
	else if ( entityType == "SLASHER" )
		m_type = Type::Slasher;
	else if ( entityType == "EFFECT_SHELTER" )
		m_type = Type::EffectShelter;
	else if ( entityType == "EFFECT_YELL" )
		m_type = Type::EffectYell;
	else
	{
		cerr << "Unknown entity type: " << entityType << endl;
		assert( false && "Unknown entity type!" );
	}
}

const char* ToString( Entity::Type type )
{
	switch ( type )
	{
	case Entity::Type::Wanderer:
		return "WANDERER";
	case Entity::Type::Explorer:
		return "EXPLORER";
	case Entity::Type::EffectPlan:
		return "EFFECT_PLAN";
	case Entity::Type::EffectLight:
		return "EFFECT_LIGHT";
	case Entity::Type::Slasher:
		return "SLASHER";
	case Entity::Type::EffectShelter:
		return "EFFECT_SHELTER";
	case Entity::Type::EffectYell:
		return "EFFECT_YELL";
	default:
		assert( false && "Unknown entity type!" );
		return "UNKNOWN";
	}
}

ostream& operator << ( ostream& out, const Entity& entity )
{
	out << ToString( entity.GetType() )
		<< ' '
		<< entity.GetId()
		<< ' '
		<< entity.GetPosition();

	return out;
}

void Explorer::LoadParams( istream& in )
{
	int param0;
	int param1;
	int param2;
	in >> param0 >> param1 >> param2; in.ignore();

	SetSanity( param0 );
}

ostream& operator << ( ostream& out, const Explorer& explorer )
{
	out << ( Entity& ) explorer
		<< ' '
		<< explorer.GetSanity();

	return out << endl;
}

void Wanderer::LoadParams( istream& in )
{
	int param0;
	int param1;
	int param2;
	in >> param0 >> param1 >> param2; in.ignore();

	SetTimeBefore( param0 );

	switch ( param1 )
	{
	case ( int )State::Spawning:
		SetState( State::Spawning );
		break;
	case (int)State::Wandering:
		SetState( State::Wandering );
		break;
	case ( int ) State::Stalking:
		SetState( State::Stalking );
		break;
	case ( int ) State::Rushing:
		SetState( State::Rushing );
		break;
	case ( int ) State::Stunned:
		SetState( State::Stunned );
		break;
	default:
		assert( false && "Unknown wanderer state!" );
		break;
	}

	SetTarget( param2 );
}

const char* ToString( Wanderer::State state )
{
	switch ( state )
	{
	case Wanderer::State::Spawning:
		return "SPAWNING";
	case Wanderer::State::Wandering:
		return "WANDERING";
	case Wanderer::State::Stalking:
		return "STALKING";
	case Wanderer::State::Rushing:
		return "RUSHING";
	case Wanderer::State::Stunned:
		return "STUNNED";
	default:
		assert( false && "Unknown wanderer state!" );
		return "UNKNOWN";
	}
}

ostream& operator << ( ostream& out, const Wanderer& wanderer )
{
	out << ( Entity& ) wanderer
		<< ' '
		<< wanderer.GetTimeBefore()
		<< ' '
		<< ToString( wanderer.GetState() ) 
		<< ' '
		<< wanderer.GetTarget();

	return out << endl;
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
		case 'U':
			SetCell( Point( i, lineIdx ), Cell::Shelter );
			break;
		case '.':
			SetCell( Point( i, lineIdx ), Cell::Empty );
			break;
		default:
			assert( false );
			break;
		}
	}
}

class Move
{
public:
	Move() : m_position(), m_cost( 0 ) {}
	Move( const Point& position ) : m_position( position ), m_cost( 0 ) {}

	const Point& GetPosition() const { return m_position; }

	int GetCost() const { return m_cost; }
	void AddCost( int cost ) { m_cost += cost; }

	bool operator < ( const Move& other ) { return m_cost < other.m_cost; }

private:
	Point m_position;
	int m_cost;
};

void Player::Update( const Grid& grid, const vector< Explorer >& explorers, const vector< Wanderer >& wanderers )
{
	static const Point directions[ 5 ] = { Point( 0, 0 ), Point(1, 0), Point(0, 1), Point(-1, 0), Point(0, -1) };

	vector< Move > moves;
	moves.reserve( countof( directions ) );

	// Gather posible moves.
	for ( const Point& d : directions )
	{
		if ( grid.CanMove( GetPosition() + d ) )
		{
			moves.push_back( Move( GetPosition() + d ) );
		}
	}
	assert( !moves.empty() );


	static const int WANDERER_DIST0_COST = 1000000;
	static const int WANDERER_DIST1_COST = WANDERER_DIST0_COST / 2;

	// Try to avoid wanderers.
	for ( const Wanderer& w : wanderers )
	{
		if ( w.GetState() == Wanderer::State::Spawning && w.GetTimeBefore() > 1 )
			continue;

		for ( Move& m : moves )
		{
			int dist = Distance( m.GetPosition(), w.GetPosition() );
			if ( dist == 0 )
				m.AddCost( WANDERER_DIST0_COST );
			else if ( dist == 1 )
				m.AddCost( WANDERER_DIST1_COST );
		}
	}


	static const int EXPLORER_DIST01_COST = 0;
	static const int EXPLORER_DIST2_COST = 100;
	static const int EXPLORER_NOTCOSE_ENOUGH = 1000;


	// Try to stay close other explorers.
	for ( const Explorer& e : explorers )
	{
		if ( e.GetSanity() <= 0 )
			continue;

		for ( Move& m : moves )
		{
			int dist = Distance( m.GetPosition(), e.GetPosition() );
			if ( dist <= 1 )
				m.AddCost( EXPLORER_DIST01_COST );
			else if ( dist == 2 )
				m.AddCost( EXPLORER_DIST2_COST );
			else
				m.AddCost( EXPLORER_NOTCOSE_ENOUGH );
		}
	}


	random_shuffle( moves.begin(), moves.end() );
	sort( moves.begin(), moves.end() );

	m_targetPosition = moves.front().GetPosition();
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
			case Grid::Cell::Shelter:
				out << 'U';
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
