#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <map>

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
static const int STUNT_RADIUS = 1760;
static const int STUNT_DELAY = 20;

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

class World
{
public:
    void SetPlayerTeamId(int teamId) { assert(teamId >= 0 && teamId < TEAMS_COUNT); m_PlayerTeamId = teamId; }
    int GetPlayerTeamId() const { return m_PlayerTeamId; }
    int GetEnemyTeamId() const { return (m_PlayerTeamId + 1) % TEAMS_COUNT; }

    const Base& GetBase(int teamId) const { return m_Bases[teamId]; }
    const Base& GetPlayerBase() const { return GetBase(GetPlayerTeamId()); }
    const Base& GetEnemyBase() const { return GetBase(GetEnemyTeamId()); }

    void NextRound() { ++m_RoundNum; }
    int GetRound() const { return m_RoundNum; }

private:
    static_assert(TEAMS_COUNT == 2, "World implementation assumes that there is only 2 worlds");
    Base m_Bases[TEAMS_COUNT] = { Base(0), Base(1) };
    int m_PlayerTeamId = 0;

    int m_RoundNum = 0;
};

class Path
{
public:
    Path() { }

    //template<typename... Args>
    //Path(Args&&... args)
    //    : m_Points(forward<Args>(args)...)
    //{  }

    Path(initializer_list<Point> list)
        : m_Points(list)
    { }
    
private:
    vector<Point> m_Points;
};

class Entity
{
public:
    Entity() : m_Position(numeric_limits<int>::min(), numeric_limits<int>::min()), m_Id(-1) { }

    void SetPosition(int x, int y) { m_Position.Set(x, y); }
    const Point& GetPosition() const { return m_Position; }

    void SetId(int id) { m_Id = id; }
    int GetId() const { return m_Id; }

private:
    Point m_Position;
    int m_Id;
};

class Ghost : public Entity
{
public:
    enum class EState
    {
        Undefined,
        UnknownPositon,
        KnownPosition,
        Busting,
        Busted,
    };
    Ghost() : Entity(), m_State(EState::Undefined), m_Stamina(0) { }

    void SetState(EState state) { m_State = state; }
    EState GetState() const { return m_State; }

    void SetStamina(int stamina) { m_Stamina = stamina; }
    int GetStamina() const { return m_Stamina; }

private:
    EState m_State;
    int m_Stamina;
};

class Buster : public Entity
{
public:
    enum class EState
    {
        Undefined,
        UnknownPosition,
        KnownPosition,
        Carring,
        Stunned,
    };
    Buster() : Entity(), m_State(EState::Undefined), m_CarriedId(-1) { }

    void SetState(EState state) { m_State = state; }
    EState GetState() const { return m_State; }

    void SetCarriedId(int id) { m_CarriedId = id; }
    int GetCarriedId() const { return m_CarriedId; }

private:
    EState m_State;
    int m_CarriedId;
};

class Player : public Buster
{
public:
    Player() : Buster(), m_DestPos(0, 0), m_StunningRound(numeric_limits<int>::min()) { }

    void SetDestPos(int x, int y) { m_DestPos.Set(x, y); }
    void SetDestPos(const Point& pos) { m_DestPos = pos; }
    Point GetDestPos() const { return m_DestPos; }

    void SetStunning(int round) { m_StunningRound = round; }
    bool CanStun(int round) const { return m_StunningRound + STUNT_DELAY < round; }

private:
    Point m_DestPos;
    int m_StunningRound;
};

Ghost* FindNearestGhost(const Point& position, map<int, Ghost>& ghosts)
{
    int nearestDist = numeric_limits<int>::max();
    Ghost* nearestGhost = nullptr;
    for (auto& ghost : ghosts)
    {
        if (ghost.second.GetState() == Ghost::EState::Undefined || ghost.second.GetState() == Ghost::EState::Busted)
            continue;

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
    for (auto& ghost: ghosts)
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
        if (enemy.GetState() == Buster::EState::Stunned || enemy.GetState() == Buster::EState::UnknownPosition || enemy.GetId() < 0)
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

    int bustersPerPlayer; // the amount of busters you control
    cin >> bustersPerPlayer; cin.ignore();
    int ghostCount; // the amount of ghosts on the map
    cin >> ghostCount; cin.ignore();
    int myTeamId; // if this is 0, your base is on the top left of the map, if it is one, on the bottom right
    cin >> myTeamId; cin.ignore();

    world.SetPlayerTeamId(myTeamId);

    vector<Player> players(bustersPerPlayer);
    vector<Buster> enemies(bustersPerPlayer);
    map<int, Ghost> ghosts;

    Path paths[] = {
        Path({ Point(1500, 1500), Point(14500, 1500) }),
        Path({ Point(1500, 1500), Point(4500, 4500), Point(14500, 4500) }),
        Path({ Point(1500, 1500), Point(1500, 7500), Point(14500, 7500) }),
        Path({ Point(1500, 1500), Point(4500, 7500), Point(7500, 1500), Point(10500, 7500), Point(13500, 1500), Point(15000, 4500) })
    };

    static_assert(countof(paths) == 4, "Wrong size");

    // Set initial destination positions.
    for (int i = 0; i < bustersPerPlayer; ++i)
    {
        auto& player = players[i];

        player.SetDestPos(MAP_RIGHT / (bustersPerPlayer + 1) * (i + 1), MAP_BOTTOM / 2);
    }

    // game loop
    while (1)
    {
        world.NextRound();

        // Clear entities' states.
        for (auto& player : players)
            player.SetState(Buster::EState::UnknownPosition);

        for (auto& enemy : enemies)
        {
            if (enemy.GetState() != Buster::EState::Undefined)
                enemy.SetState(Buster::EState::UnknownPosition);
        }

        for (auto& ghost : ghosts)
        {
            if (ghost.second.GetState() != Ghost::EState::Busted &&
                ghost.second.GetState() != Ghost::EState::Undefined)
            {
                ghost.second.SetState(Ghost::EState::UnknownPositon);
            }
        }

        // Update state of known entities.
        int entities; // the number of busters and ghosts visible to you
        cin >> entities; cin.ignore();
        for (int i = 0; i < entities; ++i)
        {
            int entityId; // buster id or ghost id
            int x;
            int y; // position of this buster / ghost
            int entityType; // the team id if it is a buster, -1 if it is a ghost.
            int state; // For busters: 0=idle, 1=carrying a ghost.
            int value; // For busters: Ghost id being carried. For ghosts: number of busters attempting to trap this ghost.
            cin >> entityId >> x >> y >> entityType >> state >> value; cin.ignore();

            if (entityType != -1)
            {
                int idx = entityId;
                if (idx >= bustersPerPlayer)
                    idx -= bustersPerPlayer;

                auto& entity = entityType == myTeamId ? players[idx] : enemies[idx];

                entity.SetId(entityId);
                entity.SetPosition(x, y);
                switch (state)
                {
                default:
                //case 0:
                //case 3:
                    entity.SetState(Buster::EState::KnownPosition);
                    break;
                case 1:
                    entity.SetState(Buster::EState::Carring);
                    {
                        auto& ghost = ghosts[value];

                        ghost.SetId(value);
                        ghost.SetState(Ghost::EState::Busted);
                        ghost.SetStamina(0);
                    }
                    break;
                case 2:
                    entity.SetState(Buster::EState::Stunned);
                    break;
                }
                entity.SetCarriedId(value);
            }
            else
            {
                auto& ghost = ghosts[entityId];

                ghost.SetId(entityId);
                ghost.SetPosition(x, y);
                ghost.SetState(value == 0 ? Ghost::EState::KnownPosition : Ghost::EState::Busting);
                ghost.SetStamina(state);

                if (entityId != 0)
                {
                    int twinEntityId = entityId % 2 ? entityId + 1 : entityId - 1;

                    auto found = ghosts.find(twinEntityId);
                    if (found == ghosts.end())
                    {
                        auto& twinGhost = ghosts[twinEntityId];

                        twinGhost.SetId(twinEntityId);
                        twinGhost.SetPosition(MAP_RIGHT - x, MAP_BOTTOM - y);
                        twinGhost.SetState(Ghost::EState::UnknownPositon);
                        twinGhost.SetStamina(state);
                    }
                }
            }
        }

        // Check if Unknown position ghosts are still valid.
        for (auto& ghost : ghosts)
        {
            cerr << ghost.second.GetId() << ':';
            switch (ghost.second.GetState())
            {
            case Ghost::EState::Undefined:
                cerr << '?'; break;
            case Ghost::EState::UnknownPositon:
                cerr << 'U'; break;
            case Ghost::EState::KnownPosition:
                cerr << 'K'; break;
            case Ghost::EState::Busting:
                cerr << 'B'; break;
            case Ghost::EState::Busted:
                cerr << 'X'; break;
            }

            if (ghost.second.GetState() == Ghost::EState::UnknownPositon)
            {
                for (auto& player : players)
                {
                    if (Distance(player.GetPosition(), ghost.second.GetPosition()) < FOG_OF_WAR_RADIUS)
                    {
                        ghost.second.SetState(Ghost::EState::Undefined);
                        cerr << '-';
                    }
                }
            }

            cerr << ' ';
        }
        cerr << endl;

        for (int i = 0; i < bustersPerPlayer; ++i)
        {
            auto& player = players[i];

            cerr << (player.CanStun(world.GetRound()) ? 'y' : 'n');
            if (player.CanStun(world.GetRound()))
            {
                auto enemy = FindNearestNotStunnedEnemy(player.GetPosition(), enemies);
                if (enemy)
                    cerr << Distance(player.GetPosition(), enemy->GetPosition()) << ' ';
                if (enemy && Distance(player.GetPosition(), enemy->GetPosition()) <= STUNT_RADIUS)
                {
                    auto dist = Distance(player.GetPosition(), enemy->GetPosition());
                    cerr << "S " << dist << endl;

                    cout << "STUN " << enemy->GetId() << endl;

                    player.SetStunning(world.GetRound());
                    enemy->SetState(Buster::EState::Stunned);

                    continue;
                }
            }

            if (player.GetState() == Buster::EState::Carring)
            {
                auto dist = Distance(player.GetPosition(), world.GetPlayerBase().GetReturnPosition());
                cerr << "R " << dist << endl;

                if (world.GetPlayerBase().IsPositionInside(player.GetPosition()))
                {
                    ghosts[player.GetCarriedId()].SetState(Ghost::EState::Busted);
                    cout << "RELEASE" << endl;
                }
                else
                    cout << "MOVE " << world.GetPlayerBase().GetReturnPosition() << endl;

                continue;
            }
            else
            {
                //auto ghost = FindNearestGhostWithState(player.GetPosition(), Ghost::EState::Busting, ghosts);
                //if (!ghost)
                //    ghost = FindNearestGhostWithState(player.GetPosition(), Ghost::EState::KnownPosition, ghosts);
                auto ghost = FindNearestGhost(player.GetPosition(), ghosts);

                if (ghost)
                {
                    auto dist = Distance(player.GetPosition(), ghost->GetPosition());
                    cerr << "B " << dist << '(' << ghost->GetId() << ')' << endl;

                    ghost->SetState(Ghost::EState::Busting);

                    if (dist > MAX_GHOST_BUST_RADIUS)
                        cout << "MOVE " << ghost->GetPosition() << endl;
                    else if (dist < MIN_GHOST_BUST_RADIUS)
                    {
                        Vector dir;

                        if (dist == 0)
                            dir = Vector(world.GetPlayerBase().GetPosition()) - Vector(ghost->GetPosition());
                        else
                            dir = Vector(player.GetPosition()) - Vector(ghost->GetPosition());
                        dir = dir.GetNormalized();

                        Point dest_pos = ghost->GetPosition() + dir * (static_cast<float>(MIN_GHOST_BUST_RADIUS));

                        cout << "MOVE " << dest_pos << endl;
                    }
                    else
                        cout << "BUST " << ghost->GetId() << endl;

                    if (ghost->GetState() == Ghost::EState::UnknownPositon)
                        player.SetDestPos(ghost->GetPosition());

                    continue;
                }

                /*
                ghost = FindNearestGhostWithState(player.GetPosition(), Ghost::EState::UnknownPositon, ghosts);
                if (ghost)
                {
                    auto dist = Distance(player.GetPosition(), ghost->GetPosition());
                    cerr << "UP " << dist << '(' << ghost->GetId() << ')' << endl;

                    ghost->SetState(Ghost::EState::Busting);

                    cout << "MOVE " << ghost->GetPosition() << endl;

                    player.SetDestPos(ghost->GetPosition());

                    continue;
                }
                */
            }

            if (player.GetDestPos() == player.GetPosition())
            {
                int rand_x = rand() % (MAP_RIGHT + 1);
                rand_x = rand() % (rand_x + 1);
                int rand_y = rand() % (MAP_BOTTOM + 1);
                rand_y = rand() % (rand_y + 1);

                if (rand() % 2)
                    rand_x = MAP_RIGHT - rand_x;
                else
                    rand_y = MAP_BOTTOM - rand_y;

                player.SetDestPos(rand_x, rand_y);
            }

            cerr << "N" << endl;
            cout << "MOVE " << player.GetDestPos() << endl; // MOVE x y | BUST id | RELEASE
        }
    }
}