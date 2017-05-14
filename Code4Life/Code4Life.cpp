#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <cassert>

using namespace std;

static const string targetStartPos = "START_POS";
static const string targetSamples = "SAMPLES";
static const string targetDiagnosis = "DIAGNOSIS";
static const string targetMolecules = "MOLECULES";
static const string targetLaboratory = "LABORATORY";

enum class eMol
{
	A,
	B,
	C,
	D,
	E,
	count
};

enum class eArea
{
	start,
	samples,
	diagnosis,
	molecules,
	laboratory,
	count
};

string getAreaName(eArea area);
int getAreaMoveCost(eArea start, eArea end);

float rankHealthPoints[4] = { 0.0f, 2.125f, 18.62068966f, 40.f };
float rankHealthPointsMin[4] = { 0.0f,  1.f, 10.f, 30.f };
float rankHealthPointsMax[4] = { 0.0f, 10.f, 30.f, 50.f };
int rankMoleculeCosts[4] = { 0, 5, 8, 14 };

struct sPlayer
{
	string target;
	int eta;
	int score;
	int storage[(int)eMol::count];
	int expertise[(int)eMol::count];

	bool isInSamples() const { return target == targetSamples; }
	bool isInDiagnosis() const { return target == targetDiagnosis; }
	bool isInMolecules() const { return target == targetMolecules; }
	bool isInLaboratory() const { return target == targetLaboratory; }
};

struct sLocalPlayer : sPlayer
{
	int choosenSample;

	sLocalPlayer() : choosenSample(-1) { }
};
  
ostream& operator << (ostream& out, const sPlayer& player);
istream& operator >> (istream& input, sPlayer& player);

struct sSupplies
{
	int available[(int)eMol::count];

	bool isAvaiable(int molecule) const { return available[molecule] > 0; }
};

ostream& operator << (ostream& out, const sSupplies& supplies);
istream& operator >> (istream& input, sSupplies& supplies);

struct sSample
{
	int sampleId;
	int carriedBy;
	int rank;
	string expertiseGain;
	int health;
	int cost[(int)eMol::count];

	bool hasAllMolecules(const sPlayer& player) const
	{
		for (int i = 0; i < (int)eMol::count; ++i)
		{
			if (cost[i] > player.storage[i] + player.expertise[i])
				return false;
		}
		return true;
	}

	bool hasMolecules(const sPlayer& player, int molecule) const
	{
		if (cost[molecule] <= player.storage[molecule] + player.expertise[molecule])
			return true;
		return false;
	}

	int getMissingMoleculesCount(const sPlayer& player) const
	{
		int missing = 0;
		for (int i = 0; i < (int)eMol::count; ++i)
		{
			missing += max(cost[i] - (player.storage[i] + player.expertise[i]), 0);
		}
		return missing;
	}

	float getCost(const sPlayer& player) const
	{
		return (float)health / (float)(getMissingMoleculesCount(player) + 1);
	}

	bool isDiagnosed() const
	{
		return cost[0] >= 0;
	}
};

ostream& operator << (ostream& out, const sSample& sample);
istream& operator >> (istream& input, sSample& sample);

struct sSamplesCollection
{
	vector<sSample> samples;
};

ostream& operator << (ostream& out, const sSamplesCollection& collection);
istream& operator >> (istream& input, sSamplesCollection& collection);

namespace cmd
{
	void goTo(const string& target) { cout << "GOTO " << target << endl; }
	void goToDiagnosis() { goTo(targetDiagnosis); }
	void goToMolecules() { goTo(targetMolecules); }
	void goToLaboratory() { goTo(targetLaboratory); }
	void goToSamples() { goTo(targetSamples); }

	void wait() { cout << "WAIT" << endl; }

	void connectId(int id) { cout << "CONNECT " << id << endl; }
	void connectType(int type) { cout << "CONNECT " << (char)('A' + type) << endl; }
	void connectRank(int rank) { cout << "CONNECT " << rank << endl; }
}

/**
 * Bring data on patient samples from the diagnosis machine to the laboratory with enough molecules to produce medicine!
 **/
int main()
{
	/*
	assert(getAreaMoveCost(areas::start, areas::start) == 0);
	assert(getAreaMoveCost(areas::samples, areas::samples) == 0);
	assert(getAreaMoveCost(areas::start, areas::laboratory) == 2);
	assert(getAreaMoveCost(areas::molecules, areas::diagnosis) == 3);
	assert(getAreaMoveCost(areas::diagnosis, areas::laboratory) == 4);
	//**/


	int projectCount;
	cin >> projectCount; cin.ignore();
	for (int i = 0; i < projectCount; i++)
	{
		int a;
		int b;
		int c;
		int d;
		int e;
		cin >> a >> b >> c >> d >> e; cin.ignore();
	}

	sLocalPlayer player{};
	sPlayer enemy{};

	sSamplesCollection collection{};

	sSupplies supplies{};

	// game loop
	while (1)
	{
		cin >> player; cin.ignore();
		cin >> enemy; cin.ignore();

		cin >> supplies; cin.ignore();

		cin >> collection; cin.ignore();

		//*
		cerr << player << endl;
		cerr << enemy << endl;
		cerr << supplies << endl;
		cerr << collection << endl;
		//*/

		// Collect sample.
		vector<sSample> samplesOfPlayer;
		copy_if(collection.samples.begin(), collection.samples.end(), back_inserter(samplesOfPlayer), [](const sSample& sample) { return sample.carriedBy == 0; });

		if (samplesOfPlayer.empty())
		{
			collection.samples.erase(remove_if(collection.samples.begin(), collection.samples.end(), [](const sSample& sample) { return sample.carriedBy != -1; }), collection.samples.end());
			sort(collection.samples.begin(), collection.samples.end(), [&player](const sSample& first, const sSample& second)
			{
				return first.getCost(player) > second.getCost(player);
			});

			if (player.isInDiagnosis())
			{
				if (!collection.samples.empty())
					cmd::connectId(collection.samples[0].sampleId);
				else
					cmd::goToSamples();
			}
			else if (player.isInSamples())
			{
				if (collection.samples.size() >= 3)
					cmd::goToDiagnosis();
				else
				{
					cmd::connectRank(2);
				}
			}
			else
			{
				if (!collection.samples.empty())
					cmd::goToDiagnosis();
				else
					cmd::goToSamples();
			}
		}
		else
		{
			const auto& sampleToCollect = samplesOfPlayer[0];
			if (!sampleToCollect.isDiagnosed())
			{
				if (player.isInDiagnosis())
					cmd::connectId(sampleToCollect.sampleId);
				else
					cmd::goToDiagnosis();
			}
			else if (sampleToCollect.hasAllMolecules(player))
			{
				if (player.isInLaboratory())
				{
					cmd::connectId(sampleToCollect.sampleId);
				}
				else
				{
					cmd::goToLaboratory();
				}
			}
			else if (player.isInMolecules())
			{
				bool collectedMolecule = false;
				for (int i = 0; i < (int)eMol::count; ++i)
				{
					if (!sampleToCollect.hasMolecules(player, i) && supplies.isAvaiable(i))
					{
						cmd::connectType(i);
						collectedMolecule = true;
						break;
					}
				}

				if (!collectedMolecule)
					cmd::wait();
			}
			else
			{
				cmd::goToMolecules();
			}
		}
	}
}


ostream& operator << (ostream& out, const sPlayer& player)
{
	out << player.target << " " << player.eta << " " << player.score;
	for (int i = 0; i < (int)eMol::count; ++i)
		out << " " << player.storage[i];
	for (int i = 0; i < (int)eMol::count; ++i)
		out << " " << player.expertise[i];
	return out;
}
istream& operator >> (istream& input, sPlayer& player)
{
	input >> player.target >> player.eta >> player.score;
	for (int i = 0; i < (int)eMol::count; ++i)
		input >> player.storage[i];
	for (int i = 0; i < (int)eMol::count; ++i)
		input >> player.expertise[i];
	return input;
}


ostream& operator << (ostream& out, const sSupplies& supplies)
{
	for (int i = 0; i < (int)eMol::count; ++i)
	{
		if (i > 0)
			out << " ";
		out << supplies.available[i];
	}
	return out;
}
istream& operator >> (istream& input, sSupplies& supplies)
{
	for (int i = 0; i < (int)eMol::count; ++i)
		input >> supplies.available[i];
	return input;
}


ostream& operator << (ostream& out, const sSample& sample)
{
	out << sample.sampleId << " " << sample.carriedBy << " " << sample.rank << " " << sample.expertiseGain << " " << sample.health;
	for (int i = 0; i < (int)eMol::count; ++i)
		out << " " << sample.cost[i];
	return out;
}
istream& operator >> (istream& input, sSample& sample)
{
	input >> sample.sampleId >> sample.carriedBy >> sample.rank >> sample.expertiseGain >> sample.health;
	for (int i = 0; i < (int)eMol::count; ++i)
		input >> sample.cost[i];
	return input;
}


ostream& operator << (ostream& out, const sSamplesCollection& collection)
{
	out << collection.samples.size();

	for (int i = 0; i < (int)collection.samples.size(); ++i)
	{
		out << endl;
		out << collection.samples[i];
	}

	return out;
}
istream& operator >> (istream& input, sSamplesCollection& collection)
{
	int sampleCount;
	input >> sampleCount;

	collection.samples.resize(sampleCount);

	for (int i = 0; i < sampleCount; ++i)
	{
		input.ignore();
		input >> collection.samples[i];
	}

	return input;
}

string getAreaName(eArea area)
{
	switch (area)
	{
	case eArea::start:		return targetStartPos;
	case eArea::samples:	return targetSamples;
	case eArea::diagnosis:	return targetDiagnosis;
	case eArea::molecules:	return targetMolecules;
	case eArea::laboratory:	return targetLaboratory;
	default: return "";
	}
}

int areasMoveCost[(int)eArea::count][(int)eArea::count] =
{
	{ 0, 2, 2, 2, 2 },
	{ 2, 0, 3, 3, 3 },
	{ 2, 3, 0, 3, 4 },
	{ 2, 3, 3, 0, 3 },
	{ 2, 3, 4, 3, 0 },
};

int getAreaMoveCost(eArea start, eArea end)
{
	if (start != eArea::count && end != eArea::count)
		return areasMoveCost[(int)start][(int)end];
	return 0;
}
