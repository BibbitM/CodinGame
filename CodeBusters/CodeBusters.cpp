#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>
#include <cmath>
#include <cstdlib>
#include <map>

using namespace std;

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

inline int sqr(int val)
{
    return val * val;
}

struct Point
{
    Point() : m_X(0), m_Y(0) { }
    Point(int x, int y) : m_X(x), m_Y(y) { }

    void Set(int x, int y) { m_X = x; m_Y = y; }

    int m_X;
    int m_Y;

    bool operator==(const Point& right)
    {
        return m_X == right.m_X && m_Y == right.m_Y;
    }

    friend std::ostream& operator<< (std::ostream& stream, const Point& point)
    {
        return stream << point.m_X << " " << point.m_Y;
    }
};

int Distance(const Point& first, const Point& second)
{
    int dist2 = sqr(first.m_X - second.m_X) + sqr(first.m_Y - second.m_Y);

    return (int)ceil(sqrt((float)dist2));
}

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
    Point GetDestPos() const { return m_DestPos; }

    void SetStunning(int round) { m_StunningRound = round; }
    bool CanStun(int round) const { return m_StunningRound + STUNT_DELAY < round; }

private:
    Point m_DestPos;
    int m_StunningRound;
};

Point GetBasePosition(int teamId)
{
    return teamId == 0 ? Point(0, 0) : Point(MAP_RIGHT, MAP_BOTTOM);
}

bool IsInBase(const Point& position, int teamId)
{
    return Distance(position, GetBasePosition(teamId)) < RETURN_DISTANCE;
}

Point GetReturnPosition(int teamId)
{
    return teamId == 0 ? Point(RETURN_COORDS, RETURN_COORDS) : Point(MAP_RIGHT - RETURN_COORDS, MAP_BOTTOM - RETURN_COORDS);
}

Ghost* FindNearestGhostWithState(const Point& position, Ghost::EState state, map<int, Ghost>& ghosts)
{
    int nearestDist = numeric_limits<int>::max();
    Ghost* nearestGhost = nullptr;
    for (auto& ghost: ghosts)
    {
        if (ghost.second.GetState() != state)
            continue;

        auto dist = Distance(position, ghost.second.GetPosition());
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
    int bustersPerPlayer; // the amount of busters you control
    cin >> bustersPerPlayer; cin.ignore();
    int ghostCount; // the amount of ghosts on the map
    cin >> ghostCount; cin.ignore();
    int myTeamId; // if this is 0, your base is on the top left of the map, if it is one, on the bottom right
    cin >> myTeamId; cin.ignore();

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

    int round = 0;
    
    // game loop
    while (1)
    {
        ++round;

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
                    if (found != ghosts.end())
                    {
                        auto& twinGhost = found->second;
                        if (twinGhost.GetState() == Ghost::EState::Undefined)
                        {
                            twinGhost.SetId(entityId);
                            twinGhost.SetPosition(MAP_RIGHT - x, MAP_BOTTOM - y);
                            twinGhost.SetState(Ghost::EState::UnknownPositon);
                            twinGhost.SetStamina(state);
                        }
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
                        cerr << '?';
                    }
                }
            }

            cerr << ' ';
        }
        cerr << endl;

        for (int i = 0; i < bustersPerPlayer; ++i)
        {
            auto& player = players[i];

            cerr << (player.CanStun(round) ? 'y' : 'n');
            if (player.CanStun(round))
            {
                auto enemy = FindNearestNotStunnedEnemy(player.GetPosition(), enemies);
                if (enemy)
                    cerr << Distance(player.GetPosition(), enemy->GetPosition()) << ' ';
                if (enemy && Distance(player.GetPosition(), enemy->GetPosition()) <= STUNT_RADIUS)
                {
                    auto dist = Distance(player.GetPosition(), enemy->GetPosition());
                    cerr << "S " << dist << endl;

                    cout << "STUN " << enemy->GetId() << endl;

                    player.SetStunning(round);
                    enemy->SetState(Buster::EState::Stunned);

                    continue;
                }
            }

            if (player.GetState() == Buster::EState::Carring)
            {
                auto dist = Distance(player.GetPosition(), GetReturnPosition(myTeamId));
                cerr << "R " << dist << endl;

                if (IsInBase(player.GetPosition(), myTeamId))
                {
                    ghosts[player.GetCarriedId()].SetState(Ghost::EState::Busted);
                    cout << "RELEASE" << endl;
                }
                else
                    cout << "MOVE " << GetReturnPosition(myTeamId) << endl;

                continue;
            }
            else
            {
                auto ghost = FindNearestGhostWithState(player.GetPosition(), Ghost::EState::Busting, ghosts);
                if (ghost)
                {
                    auto dist = Distance(player.GetPosition(), ghost->GetPosition());
                    cerr << "B " << dist << '(' << ghost->GetId() << ')' << endl;

                    ghost->SetState(Ghost::EState::Busting);

                    if (dist > MAX_GHOST_BUST_RADIUS)
                        cout << "MOVE " << ghost->GetPosition() << endl;
                    else if (dist < MIN_GHOST_BUST_RADIUS)
                        cout << "MOVE " << player.GetPosition() << endl;
                    else
                        cout << "BUST " << ghost->GetId() << endl;

                    continue;
                }

                ghost = FindNearestGhostWithState(player.GetPosition(), Ghost::EState::KnownPosition, ghosts);
                if (ghost)
                {
                    auto dist = Distance(player.GetPosition(), ghost->GetPosition());
                    cerr << "KP " << dist << '(' << ghost->GetId() << ')' << endl;

                    ghost->SetState(Ghost::EState::Busting);

                    if (dist > MAX_GHOST_BUST_RADIUS)
                        cout << "MOVE " << ghost->GetPosition() << endl;
                    else if (dist < MIN_GHOST_BUST_RADIUS)
                        cout << "MOVE " << player.GetPosition() << endl;
                    else
                        cout << "BUST " << ghost->GetId() << endl;

                    continue;
                }

                ghost = FindNearestGhostWithState(player.GetPosition(), Ghost::EState::UnknownPositon, ghosts);
                if (ghost)
                {
                    auto dist = Distance(player.GetPosition(), ghost->GetPosition());
                    cerr << "UP " << dist << '(' << ghost->GetId() << ')' << endl;

                    ghost->SetState(Ghost::EState::Busting);

                    cout << "MOVE " << ghost->GetPosition() << endl;
                                                                
                    continue;
                }
            }

            if (player.GetDestPos() == player.GetPosition())
                player.SetDestPos(rand() % (MAP_RIGHT + 1), rand() % (MAP_BOTTOM + 1));

            cerr << "N" << endl;
            cout << "MOVE " << player.GetDestPos() << endl; // MOVE x y | BUST id | RELEASE
        }
    }
}