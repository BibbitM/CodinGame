#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <map>
#include <set>

using namespace std;

static const int TEAMS_COUNT = 2;
static const int FOG_OF_WAR_RADIUS = 2200;
static const int MOVE_DISTANCE = 800;
static const int GHOST_RUN_AWAY_DISTANCE = 400;
static const int MIN_GHOST_BUST_RADIUS = 900;
static const int MAX_GHOST_BUST_RADIUS = 1760;
static const int RETURN_DISTANCE = 1600;
static const int RETURN_COORDS = 1130;
static const int MAP_TOP = 0;
static const int MAP_LEFT = 0;
static const int MAP_RIGHT = 16000;
static const int MAP_BOTTOM = 9000;
static const int MAP_GRID = 500;
static const int MAP_VISIBILITY_RADIUS = 1760 - MAP_GRID;
static const int STUN_RADIUS = 1760;
static const int STUN_DELAY = 20;
static const int STUN_TIME = 10;

template <typename T, std::size_t N>
constexpr std::size_t countof(T const (&)[N]) noexcept
{
    return N;
}

template <typename T>
inline T sqr(T val)
{
    return val * val;
}

struct Point
{
    Point() : m_X(0), m_Y(0) { }
    Point(int x, int y) : m_X(x), m_Y(y) { }

    void Set(int x, int y) { m_X = x; m_Y = y; }

    bool operator==(const Point& right) const { return m_X == right.m_X && m_Y == right.m_Y; }
    friend std::ostream& operator<< (std::ostream& stream, const Point& point) { return stream << point.m_X << " " << point.m_Y; }

    Point operator+(const Point& right) const { return Point(this->m_X + right.m_X, this->m_Y + right.m_Y); }
    Point operator-(const Point& right) const { return Point(this->m_X - right.m_X, this->m_Y - right.m_Y); }
    Point& operator+=(const Point& right) { m_X += right.m_X; m_Y += right.m_Y; return *this; }
    Point& operator-=(const Point& right) { m_X -= right.m_X; m_Y -= right.m_Y; return *this; }

    int m_X;
    int m_Y;
};

struct Vector
{
    Vector() : m_X(0.0f), m_Y(0.0f) { }
    Vector(float x, float y) : m_X(x), m_Y(y) { }
    Vector(const Point& point) : m_X(static_cast<float>(point.m_X)), m_Y(static_cast<float>(point.m_Y)) { }

    void Set(float x, float y) { m_X = x; m_Y = y; }

    operator Point() const { return Point(static_cast<int>(m_X + 0.5f), static_cast<int>(m_Y + 0.5f)); }

    bool operator==(const Vector& right) const { return m_X == right.m_X && m_Y == right.m_Y; }
    friend std::ostream& operator<< (std::ostream& stream, const Vector& point) { return stream << point.m_X << " " << point.m_Y; }

    Vector operator+(const Vector& right) const { return Vector(m_X + right.m_X, m_Y + right.m_Y); }
    Vector operator-(const Vector& right) const { return Vector(m_X - right.m_X, m_Y - right.m_Y); }
    Vector& operator+=(const Vector& right) { m_X += right.m_X; m_Y += right.m_Y; return *this; }
    Vector& operator-=(const Vector& right) { m_X -= right.m_X; m_Y -= right.m_Y; return *this; }

    friend Vector operator*(const Vector& left, float right) { return Vector(left.m_X * right, left.m_Y * right); }
    friend Vector operator*(float left, const Vector& right) { return Vector(left * right.m_X, left * right.m_Y); }
    friend Vector operator/(const Vector& left, float right) { return Vector(left.m_X / right, left.m_Y / right); }
    friend Vector operator/(float left, const Vector& right) { return Vector(left / right.m_X, left / right.m_Y); }
    Vector& operator*=(float right) { m_X *= right; m_Y *= right; return *this; }
    Vector& operator/=(float right) { m_X /= right; m_Y /= right; return *this; }

    float GetLength() const
    {
        return sqrt(sqr(m_X) - sqr(m_Y));
    }

    Vector GetNormalized() const
    {
        return *this / GetLength();

    }

    float m_X;
    float m_Y;
};

int Distance(const Point& first, const Point& second)
{
    int dist2 = sqr(first.m_X - second.m_X) + sqr(first.m_Y - second.m_Y);

    return (int)ceil(sqrt((float)dist2));
}

class Base
{
public:
    Base(int teamId) : m_TeamId(teamId) { }

    Point GetPosition() const
    {
        return m_TeamId == 0 ? Point(0, 0) : Point(MAP_RIGHT, MAP_BOTTOM);
    }

    bool IsPositionInside(const Point& position) const
    {
        return Distance(position, GetPosition()) < RETURN_DISTANCE;
    }

    Point GetReturnPosition() const
    {
        return m_TeamId == 0 ? Point(RETURN_COORDS, RETURN_COORDS) : Point(MAP_RIGHT - RETURN_COORDS, MAP_BOTTOM - RETURN_COORDS);
    }

private:
    int m_TeamId = 0;
};

class Map
{
public:
    struct Cell
    {
        Point m_Position;
        int m_LastVisitedRound = -1;
    };

    Map();

    void SetVisitedPlace(int round, const Point& position);

    Point GetRandomDestination(int round, const Point& prevDestination) const;

    bool IsPlaceVistedInRound(int round, const Point& position) const;

    static const int NUM_X = (MAP_RIGHT / MAP_GRID - 1);
    static const int NUM_Y = (MAP_BOTTOM / MAP_GRID - 1);

private:
    Cell& GetCell(int x, int y) { return m_Grid[x + y * NUM_X]; }
    const Cell& GetCell(int x, int y) const { return m_Grid[x + y * NUM_X]; }

    static int GetCellX(const Point& position) { return position.m_X / MAP_GRID - 1; }
    static int GetCellY(const Point& position) { return position.m_Y / MAP_GRID - 1; }

    Cell m_Grid[NUM_X * NUM_Y];

};

Map::Map()
{
    for (int x = 0; x < NUM_X; ++x)
    {
        for (int y = 0; y < NUM_Y; ++y)
        {
            GetCell(x, y).m_Position.Set(MAP_GRID * (x + 1), MAP_GRID * (y + 1));
        }
    }
}

void Map::SetVisitedPlace(int round, const Point& position)
{
    Point topLeft = position - Point(MAP_VISIBILITY_RADIUS, MAP_VISIBILITY_RADIUS);
    Point bottomRight = position + Point(MAP_VISIBILITY_RADIUS, MAP_VISIBILITY_RADIUS);

    const int minX = max(GetCellX(topLeft), 0);
    const int maxX = min(GetCellX(bottomRight) + 2, (int)NUM_X);

    const int minY = max(GetCellY(topLeft), 0);
    const int maxY = min(GetCellY(bottomRight) + 2, (int)NUM_Y);

    for (int x = minX; x < maxX; ++x)
    {
        for (int y = minY; y < maxY; ++y)
        {
            auto& cell = GetCell(x, y);

            if (Distance(cell.m_Position, position) <= MAP_VISIBILITY_RADIUS)
                cell.m_LastVisitedRound = round;
        }
    }
}

bool Map::IsPlaceVistedInRound(int round, const Point& position) const
{
    const int minX = max(GetCellX(position), 0);
    const int maxX = min(GetCellX(position) + 2, (int)NUM_X);

    const int minY = max(GetCellY(position), 0);
    const int maxY = min(GetCellY(position) + 2, (int)NUM_Y);

    for (int x = minX; x < maxX; ++x)
    {
        for (int y = minY; y < maxY; ++y)
        {
            const auto& cell = GetCell(x, y);
            cerr << cell.m_Position << " " << cell.m_LastVisitedRound << endl;

            if (cell.m_LastVisitedRound < round)
                return false;
        }
    }

    return true;
}


Point Map::GetRandomDestination(int round, const Point& prevDestination) const
{
    int prevX = GetCellX(prevDestination);
    int prevY = GetCellY(prevDestination);

    if (prevX >= 0 && prevX < NUM_X &&
        prevY >= 0 && prevY < NUM_Y)
    {
        const auto& cell = GetCell(prevX, prevY);
        if (cell.m_LastVisitedRound + 1 < round)
            return prevDestination;
    }

    Cell tempGrid[NUM_X * NUM_Y];
    copy(begin(m_Grid), end(m_Grid), tempGrid);

    sort(begin(tempGrid), end(tempGrid), [&](const Cell& left, const Cell& right)
    {
        if (left.m_LastVisitedRound == right.m_LastVisitedRound)
        {
            return Distance(prevDestination, left.m_Position) < Distance(prevDestination, right.m_Position);
        }
        return left.m_LastVisitedRound < right.m_LastVisitedRound;

    });

    return tempGrid[rand() % min(min((int)NUM_X, (int)NUM_Y), 10)].m_Position;
}

class Entity
{
public:
    void SetPosition(int x, int y) { m_Position.Set(x, y); }
    void SetPosition(const Point& pos) { m_Position = pos; }
    const Point& GetPosition() const { return m_Position; }

    void SetId(int id) { m_Id = id; }
    int GetId() const { return m_Id; }

    void SetLastSeenRound(int round) { m_LastSeenRound = round; }
    int GetLastSeenRound() const { return m_LastSeenRound; }

protected:
    Point m_Position;
    int m_Id = -1;
    int m_LastSeenRound = -1;
};

class Ghost : public Entity
{
public:
    enum class EState
    {
        Idle,
        Busting,
        Carried,
        Busted,
    };

    void SetIdle(int round, const Point& position, int stamina)
    {
        m_LastSeenRound = round;
        m_State = EState::Idle;
        m_Position = position;
        m_Stamina = stamina;

        m_CarriedBy = -1;
        m_NumBusting = 0;
        m_BustingBy.clear();
    }
    void SetBusting(int round, const Point& position, int stamina, int numBusting)
    {
        m_LastSeenRound = round;
        m_State = EState::Idle;
        m_Position = position;
        m_Stamina = stamina;

        m_CarriedBy = -1;
        m_NumBusting = numBusting;
        m_BustingBy.clear();
    }
    void SetBusted()
    {
        m_State = EState::Busted;
    }
    void SetFromTwin(const Ghost& twin)
    {
        cerr << __FUNCTION__ << " assert(m_LastSeenRound < 0)" << endl;
        m_LastSeenByOthersRound = twin.m_LastSeenRound;

        m_Position.Set(MAP_RIGHT, MAP_BOTTOM);
        m_Position -= twin.m_Position;

        m_Stamina = twin.m_Stamina;
    }

    void SetCarriedBy(int round, int entityId, const Point& entityPos)
    {
        cerr << __FUNCTION__ << " assert(m_LastSeenRound != round)" << endl;

        m_LastSeenByOthersRound = round;
        m_LastSeenRound = round;

        m_State = EState::Carried;
        m_Position = entityPos;
        m_Stamina = 0;

        m_CarriedBy = entityId;
        m_NumBusting = 0;
        m_BustingBy.clear();
    }

    void SetBustingBy(int round, int entityId, const Point& entityPos)
    {
        if (m_LastSeenByOthersRound != round &&
            m_LastSeenRound != round)
        {
            m_CarriedBy = -1;
            m_NumBusting = 0;
            m_BustingBy.clear();
        }

        if (m_LastSeenRound != round)
        {
            ++m_NumBusting;
            --m_Stamina;
            if (m_Stamina < 0)
                m_Stamina = 0;
        }
        else
            cerr << __FUNCTION__ << " assert(m_NumBusting > 0)" << endl;

        m_BustingBy.insert(entityId);
        cerr << __FUNCTION__ << " assert((int)m_BustingBy.size() <= m_NumBusting)" << endl;

        if (m_LastSeenRound != round)
        {
            m_Position = (Vector(m_Position) * static_cast<float>(m_NumBusting - 1) + entityPos) / static_cast<float>(m_NumBusting);
        }

        m_State = EState::Busting;
        m_LastSeenByOthersRound = round;
    }

    void SetState(EState state) { m_State = state; }
    EState GetState() const { return m_State; }

    void SetStamina(int stamina) { m_Stamina = stamina; }
    int GetStamina() const { return m_Stamina; }

    int GetLastSeenByOtherRound() const { return m_LastSeenByOthersRound; }

private:
    EState m_State = EState::Idle;
    int m_Stamina = -1;
    int m_LastSeenByOthersRound = -1;
    int m_CarriedBy = -1;
    int m_NumBusting = 0;
    set<int> m_BustingBy;
};

class Buster : public Entity
{
public:
    enum class EState
    {
        Moving,
        Busting,
        Carring,
        Stunned,
    };

    void SetMoving(int round, const Point& position)
    {
        m_LastSeenRound = round;
        m_State = EState::Moving;
        m_Position = position;
        m_GhostId = -1;
    }
    void SetBusting(int round, const Point& position, int ghostId)
    {
        m_LastSeenRound = round;
        m_State = EState::Busting;
        m_Position = position;
        m_GhostId = ghostId;
    }
    void SetCarring(int round, const Point& position, int ghostId)
    {
        m_LastSeenRound = round;
        m_State = EState::Carring;
        m_Position = position;
        m_GhostId = ghostId;
    }
    void SetStunned(int round, const Point& position, int numRoundLeft)
    {
        m_LastSeenRound = round;
        m_State = EState::Stunned;
        m_Position = position;
        m_GhostId = -1;

        m_StunnedRound = round - STUN_TIME + numRoundLeft;
    }

    void SetStunningOther(int round)
    {
        m_StunningOtherRound = round;

        m_State = EState::Moving;
        m_GhostId = -1;
    }

    void SetState(EState state) { m_State = state; }
    EState GetState() const { return m_State; }

    void SetCarriedId(int id) { m_GhostId = id; }
    int GetCarriedId() const { return m_GhostId; }
    int GetBustingId() const { return m_GhostId; }

    int GetStunnedRound() const { return m_StunnedRound; }
    int GetStunningOtherRound() const { return m_StunningOtherRound; }

    bool IsStunned(int round) const
    {
        return m_StunnedRound >= 0 && m_StunnedRound + STUN_TIME >= round;
    }

    bool CanStunInRound(int round) const
    {
        if (IsStunned(round) && m_StunnedRound != round)
            return false;

        return m_StunningOtherRound < 0 || m_StunningOtherRound + STUN_DELAY < round;
    }

private:
    EState m_State = EState::Moving;
    int m_GhostId = -1;

    int m_StunnedRound = -1;
    int m_StunningOtherRound = -1;

};

class Player : public Buster
{
public:
    enum class EDecision
    {
        Move,
        Bust,
        Release,
        Stun,
    };
    EDecision GetDecision() const { return m_Decision; }

    void SetStunEnemy(int enemyId) { m_Decision = EDecision::Stun; m_EnemyId = enemyId; }

    void SetBustGhost(int ghostId) { m_Decision = EDecision::Bust; m_GhostId = ghostId; }

    void SetReleaseGhost() { m_Decision = EDecision::Release; }

    void SetDestinationPosition(const Point& pos) { m_Decision = EDecision::Move;  m_DestinationPosition = pos; }
    Point GetDestinationPosition() const { return m_DestinationPosition; }

    void ExecuteDecision(int round, ostream& out)
    {
        switch (m_Decision)
        {
        case EDecision::Move:
            out << "MOVE " << m_DestinationPosition;
            break;
        case EDecision::Bust:
            out << "BUST " << m_GhostId;
            break;
        case EDecision::Release:
            out << "RELEASE ";
            break;
        case EDecision::Stun:
            SetStunningOther(round);
            out << "STUN " << m_EnemyId;
            break;
        }

        out << endl;
    }

private:
    EDecision m_Decision = EDecision::Move;
    Point m_DestinationPosition;
    int m_GhostId = -1;
    int m_EnemyId = -1;
};

Ghost* FindNearestGhost(const Point& position, map<int, Ghost>& ghosts);
Ghost* FindNearestGhostWithState(const Point& position, Ghost::EState state, map<int, Ghost>& ghosts);
Buster* FindNearestNotStunnedEnemy(const Point& position, vector<Buster>& enemies);

class World
{
public:
    void Initialize(istream& in)
    {
        ReadWorldParameters(in);

        CreateEntities();
    }

    void Simulate(istream& in, ostream& out, ostream& log)
    {
        UpdateWorldStateAfterPrevRound(in, log);

        NextRound();

        SimulateNextMove(out, log);
    }

    int GetPlayerTeamId() const { return m_PlayerTeamId; }
    int GetEnemyTeamId() const { return (m_PlayerTeamId + 1) % TEAMS_COUNT; }

    const Base& GetBase(int teamId) const { return m_Bases[teamId]; }
    const Base& GetPlayerBase() const { return GetBase(GetPlayerTeamId()); }
    const Base& GetEnemyBase() const { return GetBase(GetEnemyTeamId()); }

    void NextRound() { ++m_RoundNum; }
    int GetRound() const { return m_RoundNum; }

    const Map& GetMap() const { return m_Map; }

private:
    void ReadWorldParameters(istream& in)
    {
        in >> m_BustersPerPlayer; in.ignore();
        in >> m_GhostCount; in.ignore();
        in >> m_PlayerTeamId; in.ignore();

        cerr << __FUNCTION__ << " assert(m_PlayerTeamId >= 0 && m_PlayerTeamId < TEAMS_COUNT)" << endl;
    }

    void CreateEntities()
    {
        m_PlayersVector.resize(m_BustersPerPlayer);
        m_EnemiesVector.resize(m_BustersPerPlayer);
    }

    void UpdateWorldStateAfterPrevRound(istream& in, ostream& log);
    void SimulateNextMove(ostream& out, ostream& log);


    // The amount of busters you control.
    int m_BustersPerPlayer = 0;
    // The amount of ghosts on the map.
    int m_GhostCount = 0;
    // If this is 0, your base is on the top left of the map, if it is one, on the bottom right.
    int m_PlayerTeamId = 0;


    static_assert(TEAMS_COUNT == 2, "World implementation assumes that there is only 2 worlds");
    Base m_Bases[TEAMS_COUNT] = { Base(0), Base(1) };
    Map m_Map;

    map<int, Player> m_Players;
    map<int, Buster> m_Enemies;
    map<int, Ghost> m_Ghosts;

    vector<Player> m_PlayersVector;
    vector<Buster> m_EnemiesVector;

    int m_RoundNum = 0;
};


void World::UpdateWorldStateAfterPrevRound(istream& in, ostream& log)
{
    // Update state of known entities.
    int entities; // the number of busters and ghosts visible to you
    in >> entities; in.ignore();
    for (int i = 0; i < entities; ++i)
    {
        int entityId; // buster id or ghost id
        int x;
        int y; // position of this buster / ghost
        int entityType; // the team id if it is a buster, -1 if it is a ghost.
        int state; // For busters: 0=idle, 1=carrying a ghost.
        int value; // For busters: Ghost id being carried. For ghosts: number of busters attempting to trap this ghost.
        in >> entityId >> x >> y >> entityType >> state >> value; in.ignore();

        if (entityType != -1)
        {
            {
                int idx = entityId;
                if (idx >= m_BustersPerPlayer)
                    idx -= m_BustersPerPlayer;

                auto& entity = entityType == GetPlayerTeamId() ? m_PlayersVector[idx] : m_EnemiesVector[idx];

                entity.SetId(entityId);
                entity.SetPosition(x, y);
                switch (state)
                {
                default:
                    //case 0:
                    //case 3:
                    entity.SetState(Buster::EState::Moving);
                    break;
                case 1:
                    entity.SetState(Buster::EState::Carring);
                    break;
                case 2:
                    entity.SetState(Buster::EState::Stunned);
                    break;
                }
                entity.SetCarriedId(value);
            }

            {
                auto& entity = entityType == GetPlayerTeamId() ? m_Players[entityId] : m_Enemies[entityId];

                entity.SetId(entityId);

                switch (state)
                {
                case 0:
                    entity.SetMoving(GetRound(), Point(x, y));
                    break;
                case 1:
                    entity.SetCarring(GetRound(), Point(x, y), value);
                    break;
                case 2:
                    entity.SetStunned(GetRound(), Point(x, y), value);
                    break;
                case 3:
                    entity.SetBusting(GetRound(), Point(x, y), value);
                    break;

                default:
                    cerr << __FUNCTION__ << " assert(false)" << endl;
                }
            }
        }
        else
        {
            auto& ghost = m_Ghosts[entityId];
            ghost.SetId(entityId);

            if (value == 0)
                ghost.SetIdle(GetRound(), Point(x, y), state);
            else
                ghost.SetBusting(GetRound(), Point(x, y), state, value);

            if (entityId != 0)
            {
                int twinEntityId = entityId % 2 ? entityId + 1 : entityId - 1;

                auto found = m_Ghosts.find(twinEntityId);
                if (found == m_Ghosts.end())
                {
                    auto& twinGhost = m_Ghosts[twinEntityId];

                    twinGhost.SetId(twinEntityId);
                    twinGhost.SetFromTwin(ghost);
                }
            }
        }
    }

    // Update ghosts from busters.
    for (const auto& player : m_Players)
    {
        const auto& buster = player.second;

        if (buster.GetState() == Player::EState::Busting)
        {
            const int ghostId = buster.GetBustingId();

            auto& ghost = m_Ghosts[ghostId];
            ghost.SetId(ghostId);

            ghost.SetBustingBy(GetRound(), buster.GetId(), buster.GetPosition());
        }
        else if (buster.GetState() == Player::EState::Carring)
        {
            const int ghostId = buster.GetBustingId();

            auto& ghost = m_Ghosts[ghostId];
            ghost.SetId(ghostId);

            ghost.SetCarriedBy(GetRound(), buster.GetId(), buster.GetPosition());
        }

    }

    for (const auto& enemy : m_Enemies)
    {
        const auto& buster = enemy.second;

        if (buster.GetState() == Player::EState::Busting)
        {
            const int ghostId = buster.GetBustingId();

            auto& ghost = m_Ghosts[ghostId];
            ghost.SetId(ghostId);

            ghost.SetBustingBy(GetRound(), buster.GetId(), buster.GetPosition());
        }
        else if (buster.GetState() == Player::EState::Carring)
        {
            const int ghostId = buster.GetBustingId();

            auto& ghost = m_Ghosts[ghostId];
            ghost.SetId(ghostId);

            ghost.SetCarriedBy(GetRound(), buster.GetId(), buster.GetPosition());
        }
    }

    // Find who stunned me.
    for (auto& player : m_Players)
    {
        if (player.second.GetState() != Player::EState::Stunned)
            continue;
        if (player.second.GetStunnedRound() != GetRound())
            continue;

        log << "Player " << player.second.GetId() << " just stunned";

        //log << endl << "Round: " << GetRound() << endl;

        vector<Buster*> enemies;
        for (auto& enemy : m_Enemies)
        {
            //log << "ID:" << enemy.second.GetId() << " St:" << ('0' + (char)enemy.second.GetState()) << " ST:" << enemy.second.GetStunnedRound() << " SO:" << enemy.second.GetStunningOtherRound() << endl;

            if (enemy.second.GetState() == Buster::EState::Busting ||
                enemy.second.GetState() == Buster::EState::Carring)
                continue;

            if (enemy.second.GetState() == Buster::EState::Stunned &&
                enemy.second.GetStunnedRound() != GetRound())
                continue;

            if (!enemy.second.CanStunInRound(GetRound()))
                continue;

            if (Distance(player.second.GetPosition(), enemy.second.GetPosition()) > STUN_RADIUS)
                continue;

            enemies.push_back(&enemy.second);
        }

        cerr << __FUNCTION__ << " assert(!enemies.empty())" << endl;

        if (enemies.size() == 1)
        {
            enemies[0]->SetStunningOther(GetRound());
            log << " by " << enemies[0]->GetId();
        }
        else
            log << " by multiple enemies " << enemies.size();


        log << endl;
    }

    // Check visited places.
    for (const auto& player : m_Players)
    {
        const auto& buster = player.second;
        m_Map.SetVisitedPlace(GetRound(), buster.GetPosition());
    }

    for (auto& enemy : m_Enemies)
    {
        auto& buster = enemy.second;

        if (buster.GetLastSeenRound() != GetRound() &&
            buster.GetState() == Buster::EState::Carring)
        {
            if (GetEnemyBase().IsPositionInside(buster.GetPosition()))
                buster.SetState(Buster::EState::Moving);
            else
            {
                Vector toBase = GetEnemyBase().GetPosition() - buster.GetPosition();
                toBase = toBase.GetNormalized() * static_cast<float>(MOVE_DISTANCE);

                buster.SetPosition(buster.GetPosition() + Point(toBase));
            }
        }

        if (buster.GetLastSeenRound() == GetRound() ||
            buster.GetState() == Buster::EState::Carring)
        {
            m_Map.SetVisitedPlace(GetRound(), buster.GetPosition());
        }
    }



    // Check if Unknown position ghosts are still valid.
    for (auto& ghost : m_Ghosts)
    {
        log << ghost.second.GetId() << ':';
        switch (ghost.second.GetState())
        {
        case Ghost::EState::Idle:
            log << 'I'; break;
        case Ghost::EState::Busting:
            log << 'B'; break;
        case Ghost::EState::Carried:
            log << 'C'; break;
        case Ghost::EState::Busted:
            log << 'X'; break;
        }

        // TODO: Fix it. We don't know how to check if ghost is still in given position.
        /*
        if (ghost.second.GetState() == Ghost::EState::UnknownPositon)
        {
            for (auto& player : m_Players)
            {
                if (Distance(player.GetPosition(), ghost.second.GetPosition()) < FOG_OF_WAR_RADIUS)
                {
                    ghost.second.SetState(Ghost::EState::Undefined);
                    log << '-';
                }
            }
        }
        */

        log << ' ';
    }
    log << endl;
}

void World::SimulateNextMove(ostream& out, ostream& log)
{
    for (auto& player : m_Players)
    {
        player.second.SetDestinationPosition(m_Map.GetRandomDestination(GetRound(), player.second.GetDestinationPosition()));
    }

    for (const auto& enemy : m_Enemies)
    {
        if (enemy.second.GetState() == Buster::EState::Stunned)
            continue;
        if (enemy.second.IsStunned(GetRound()))
            continue;
        if (enemy.second.GetLastSeenRound() != GetRound() - 1)
            continue;

        Player* nearestPlayer = nullptr;
        int nearestPlayerDistance = numeric_limits<int>::max();

        for (auto& player : m_Players)
        {
            if (!player.second.CanStunInRound(GetRound()))
                continue;
            if (player.second.GetDecision() != Player::EDecision::Move)
                continue;

            int distance = Distance(player.second.GetPosition(), enemy.second.GetPosition());
            if (distance <= STUN_RADIUS)
            {
                if (nearestPlayer && nearestPlayer->GetState() == Buster::EState::Carring && player.second.GetState() != Buster::EState::Carring)
                {
                    nearestPlayer = &player.second;
                    nearestPlayerDistance = distance;
                }
                else if (nearestPlayer && nearestPlayer->GetState() != Buster::EState::Carring && player.second.GetState() == Buster::EState::Carring)
                {
                     // Nearest is carring. Do not bust.
                }
                else if (distance < nearestPlayerDistance)
                {
                    nearestPlayer = &player.second;
                    nearestPlayerDistance = distance;
                }
            }
        }

        if (nearestPlayer)
        {
            nearestPlayer->SetStunEnemy(enemy.second.GetId());
        }
    }

    for (auto& player : m_Players)
    {
        if (player.second.IsStunned(GetRound()))
            continue;
        if (player.second.GetDecision() != Player::EDecision::Move)
            continue;
        if (player.second.GetState() != Buster::EState::Carring)
            continue;

        //auto dist = Distance(player.second.GetPosition(), GetPlayerBase().GetReturnPosition());
        //log << "R " << dist << endl;

        if (GetPlayerBase().IsPositionInside(player.second.GetPosition()))
        {
            player.second.SetReleaseGhost();
            m_Ghosts[player.second.GetCarriedId()].SetBusted();
        }
        else
            player.second.SetDestinationPosition(GetPlayerBase().GetReturnPosition());

        continue;
    }

    Ghost* nearestGhost = nullptr;
    int nearestGhostDistance = numeric_limits<int>::max();
    for (auto& ghost : m_Ghosts)
    {
        if (ghost.second.GetState() == Ghost::EState::Busted ||
            ghost.second.GetState() == Ghost::EState::Carried)
            continue;

        if (ghost.second.GetLastSeenRound() != GetRound() - 1)
        {
            int lastSeenRound = max(ghost.second.GetLastSeenRound(), ghost.second.GetLastSeenByOtherRound());

            log << "Id: " << ghost.second.GetId() << " LastSeenR: " << lastSeenRound << "(" << ghost.second.GetLastSeenRound() << "," << ghost.second.GetLastSeenByOtherRound() << ")" << " Pos:" << ghost.second.GetPosition() << endl;

            if (m_Map.IsPlaceVistedInRound(lastSeenRound, ghost.second.GetPosition()))
                continue;
        }

        int nearestPlayerDistance = numeric_limits<int>::max();
        for (auto& player : m_Players)
        {
            if (player.second.IsStunned(GetRound()))
                continue;
            if (player.second.GetDecision() != Player::EDecision::Move)
                continue;
            if (player.second.GetState() == Buster::EState::Carring)
                continue;

            auto dist = Distance(player.second.GetPosition(), ghost.second.GetPosition());
            if (dist < nearestPlayerDistance)
                nearestPlayerDistance = dist;
        }

        int dist = nearestPlayerDistance;
        if (ghost.second.GetStamina() > 0)
            dist += MOVE_DISTANCE * ghost.second.GetStamina() / 10;
        else
            dist += MOVE_DISTANCE * 40 / 10;

        if (dist < nearestGhostDistance)
        {
            log << "GhostId: " << ghost.second.GetId() << endl;
            nearestGhost = &ghost.second;
            nearestGhostDistance = dist;
        }
    }

    if (nearestGhost)
    {
        for (auto& player : m_Players)
        {
            if (player.second.IsStunned(GetRound()))
                continue;
            if (player.second.GetDecision() != Player::EDecision::Move)
                continue;
            if (player.second.GetState() == Buster::EState::Carring)
                continue;

            auto dist = Distance(player.second.GetPosition(), nearestGhost->GetPosition());
            //log << "B " << dist << '(' << ghost->GetId() << ')' << endl;

            if (nearestGhost->GetState() == Ghost::EState::Carried)
            {
                player.second.SetDestinationPosition(nearestGhost->GetPosition());
            }
            else
            {
                if (dist > MAX_GHOST_BUST_RADIUS || nearestGhost->GetLastSeenRound() != GetRound() - 1)
                    player.second.SetDestinationPosition(nearestGhost->GetPosition());
                else if (dist < MIN_GHOST_BUST_RADIUS)
                {
                    Vector dir;

                    if (dist == 0)
                        dir = Vector(GetPlayerBase().GetPosition()) - Vector(nearestGhost->GetPosition());
                    else
                        dir = Vector(player.second.GetPosition()) - Vector(nearestGhost->GetPosition());
                    dir = dir.GetNormalized();

                    Point dest_pos = nearestGhost->GetPosition() + dir * (static_cast<float>(MIN_GHOST_BUST_RADIUS));

                    player.second.SetDestinationPosition(dest_pos);
                }
                else
                    player.second.SetBustGhost(nearestGhost->GetId());
            }
        }
    }


    for (auto& player : m_Players)
    {
        player.second.ExecuteDecision(GetRound(), out);
    }

    //for (int i = 0; i < (int)m_PlayersVector.size(); ++i)
    //{
    //    auto& player = m_PlayersVector[i];

    //    log << (player.CanStun(GetRound()) ? 'y' : 'n');
    //    if (player.CanStun(GetRound()))
    //    {
    //        auto enemy = FindNearestNotStunnedEnemy(player.GetPosition(), m_EnemiesVector);
    //        if (enemy)
    //            log << Distance(player.GetPosition(), enemy->GetPosition()) << ' ';
    //        if (enemy && Distance(player.GetPosition(), enemy->GetPosition()) <= STUN_RADIUS)
    //        {
    //            auto dist = Distance(player.GetPosition(), enemy->GetPosition());
    //            log << "S " << dist << endl;

    //            out << "STUN " << enemy->GetId() << endl;

    //            player.SetStunning(GetRound());
    //            enemy->SetState(Buster::EState::Stunned);

    //            continue;
    //        }
    //    }

    //    if (player.GetState() == Buster::EState::Carring)
    //    {
    //        auto dist = Distance(player.GetPosition(), GetPlayerBase().GetReturnPosition());
    //        log << "R " << dist << endl;

    //        if (GetPlayerBase().IsPositionInside(player.GetPosition()))
    //        {
    //            m_Ghosts[player.GetCarriedId()].SetState(Ghost::EState::Busted);
    //            out << "RELEASE" << endl;
    //        }
    //        else
    //            out << "MOVE " << GetPlayerBase().GetReturnPosition() << endl;

    //        continue;
    //    }
    //    else
    //    {
    //        //auto ghost = FindNearestGhostWithState(player.GetPosition(), Ghost::EState::Busting, ghosts);
    //        //if (!ghost)
    //        //    ghost = FindNearestGhostWithState(player.GetPosition(), Ghost::EState::KnownPosition, ghosts);
    //        auto ghost = FindNearestGhost(player.GetPosition(), m_Ghosts);

    //        if (ghost)
    //        {
    //            auto dist = Distance(player.GetPosition(), ghost->GetPosition());
    //            log << "B " << dist << '(' << ghost->GetId() << ')' << endl;

    //            ghost->SetState(Ghost::EState::Busting);

    //            if (dist > MAX_GHOST_BUST_RADIUS)
    //                out << "MOVE " << ghost->GetPosition() << endl;
    //            else if (dist < MIN_GHOST_BUST_RADIUS)
    //            {
    //                Vector dir;

    //                if (dist == 0)
    //                    dir = Vector(GetPlayerBase().GetPosition()) - Vector(ghost->GetPosition());
    //                else
    //                    dir = Vector(player.GetPosition()) - Vector(ghost->GetPosition());
    //                dir = dir.GetNormalized();

    //                Point dest_pos = ghost->GetPosition() + dir * (static_cast<float>(MIN_GHOST_BUST_RADIUS));

    //                out << "MOVE " << dest_pos << endl;
    //            }
    //            else
    //                out << "BUST " << ghost->GetId() << endl;

    //            //if (ghost->GetState() == Ghost::EState::UnknownPositon)
    //            //    player.SetDestPos(ghost->GetPosition());

    //            continue;
    //        }

    //        /*
    //        ghost = FindNearestGhostWithState(player.GetPosition(), Ghost::EState::UnknownPositon, ghosts);
    //        if (ghost)
    //        {
    //        auto dist = Distance(player.GetPosition(), ghost->GetPosition());
    //        log << "UP " << dist << '(' << ghost->GetId() << ')' << endl;

    //        ghost->SetState(Ghost::EState::Busting);

    //        out << "MOVE " << ghost->GetPosition() << endl;

    //        player.SetDestPos(ghost->GetPosition());

    //        continue;
    //        }
    //        */
    //    }

    //    if (player.GetDestPos() == player.GetPosition())
    //    {
    //        int rand_x = rand() % (MAP_RIGHT + 1);
    //        rand_x = rand() % (rand_x + 1);
    //        int rand_y = rand() % (MAP_BOTTOM + 1);
    //        rand_y = rand() % (rand_y + 1);

    //        if (rand() % 2)
    //            rand_x = MAP_RIGHT - rand_x;
    //        else
    //            rand_y = MAP_BOTTOM - rand_y;

    //        player.SetDestPos(rand_x, rand_y);
    //    }

    //    log << "N" << endl;
    //    out << "MOVE " << player.GetDestPos() << endl; // MOVE x y | BUST id | RELEASE
    //}
}

Ghost* FindNearestGhost(const Point& position, map<int, Ghost>& ghosts)
{
    int nearestDist = numeric_limits<int>::max();
    Ghost* nearestGhost = nullptr;
    for (auto& ghost : ghosts)
    {
        if (ghost.second.GetState() == Ghost::EState::Busted ||
            ghost.second.GetState() == Ghost::EState::Carried)
        {
            continue;
        }

        auto dist = Distance(position, ghost.second.GetPosition()) + ghost.second.GetStamina() * MOVE_DISTANCE / (ghost.second.GetState() != Ghost::EState::Busting ? 1 : 2);
        if (dist < nearestDist)
        {
            nearestDist = dist;
            nearestGhost = &ghost.second;
        }
    }

    return nearestGhost;
}

Ghost* FindNearestGhostWithState(const Point& position, Ghost::EState state, map<int, Ghost>& ghosts)
{
    int nearestDist = numeric_limits<int>::max();
    Ghost* nearestGhost = nullptr;
    for (auto& ghost : ghosts)
    {
        if (ghost.second.GetState() != state)
            continue;

        auto dist = Distance(position, ghost.second.GetPosition()) + ghost.second.GetStamina() * MOVE_DISTANCE;
        if (dist < nearestDist)
        {
            nearestDist = dist;
            nearestGhost = &ghost.second;
        }
    }

    return nearestGhost;
}

Buster* FindNearestNotStunnedEnemy(const Point& position, vector<Buster>& enemies)
{
    int nearestDist = numeric_limits<int>::max();
    int nearestIdx = -1;
    for (int i = 0; i < (int)enemies.size(); ++i)
    {
        const auto& enemy = enemies[i];
        if (enemy.GetState() == Buster::EState::Stunned || enemy.GetState() == Buster::EState::Moving || enemy.GetId() < 0)
            continue;

        auto dist = Distance(position, enemy.GetPosition());
        if (dist < nearestDist)
        {
            nearestDist = dist;
            nearestIdx = i;
        }
    }

    return nearestIdx >= 0 ? &enemies[nearestIdx] : nullptr;
}

/**
* Send your busters out into the fog to trap ghosts and bring them home!
**/
int main()
{
    World world;

    world.Initialize(cin);

    // game loop
    while (1)
    {
        world.Simulate(cin, cout, cerr);
    }
}
