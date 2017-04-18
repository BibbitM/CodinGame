#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

struct position
{
	int x;
	int y;
};

template <typename T>
constexpr T sqr(T val)
{
	return val * val;
}

int main()
{
	vector<position> barrelPositions;

	// game loop
	while (1)
	{
		position myPosition;
		barrelPositions.clear();

		int myShipCount; // the number of remaining ships
		cin >> myShipCount; cin.ignore();
		int entityCount; // the number of entities (e.g. ships, mines or cannonballs)
		cin >> entityCount; cin.ignore();
		for (int i = 0; i < entityCount; ++i)
		{
			int entityId;
			string entityType;
			int x;
			int y;
			int arg1;
			int arg2;
			int arg3;
			int arg4;
			cin >> entityId >> entityType >> x >> y >> arg1 >> arg2 >> arg3 >> arg4; cin.ignore();
			if (entityType == "SHIP" && arg4 == 1)
			{
				myPosition.x = x;
				myPosition.y = y;
			}
			else if (entityType == "BARREL")
			{
				barrelPositions.push_back({ x, y });
			}
		}

		sort(barrelPositions.begin(), barrelPositions.end(), [&myPosition](const position& first, const position& second)
		{
			auto first_distance = sqr(first.x - myPosition.x) + sqr(first.y - myPosition.y);
			auto second_distance = sqr(second.x - myPosition.x) + sqr(second.y - myPosition.y);
			return first_distance < second_distance;
		});

		for (int i = 0; i < myShipCount; i++)
		{

			// Write an action using cout. DON'T FORGET THE "<< endl"
			// To debug: cerr << "Debug messages..." << endl;

			if (barrelPositions.empty())
				cout << "SLOWER" << endl;
			else
				cout << "MOVE " << barrelPositions[0].x << " " << barrelPositions[0].y << endl;
		}
	}
}
