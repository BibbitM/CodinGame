#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>

using namespace std;

static const string targetDiagnosis = "DIAGNOSIS";
static const string targetMolecules = "MOLECULES";
static const string targetLaboratory = "LABORATORY";
static const string targetSamples = "SAMPLES";

enum class Mol
{
	A,
	B,
	C,
	D,
	E,
	Count
};

float rankHealthPoints[4] = { 0.0f, 5.5f, 20.f, 40.f };
int rankMoleculeCosts[4] = { 0, 5, 8, 14 };

struct playerStruct
{
	string target;
	int eta;
	int score;
	int storage[(int)Mol::Count];
	int expertise[(int)Mol::Count];

	bool isInDiagnosis() const { return target == targetDiagnosis; }
	bool isInMolecules() const { return target == targetMolecules; }
	bool isInLaboratory() const { return target == targetLaboratory; }
	bool isInSamples() const { return target == targetSamples; }
};

struct myPlayerStruct : playerStruct
{
	int choosenSample;

	myPlayerStruct() : choosenSample(-1) { }
};
  
ostream& operator << (ostream& out, const playerStruct& player);
istream& operator >> (istream& input, playerStruct& player);

struct suppliesStruct
{
	int available[(int)Mol::Count];

	bool isAvaiable(int molecule) const { return available[molecule] > 0; }
};

ostream& operator << (ostream& out, const suppliesStruct& supplies);
istream& operator >> (istream& input, suppliesStruct& supplies);

struct sampleStruct
{
	int sampleId;
	int carriedBy;
	int rank;
	string expertiseGain;
	int health;
	int cost[(int)Mol::Count];

	bool hasAllMolecules(const playerStruct& player) const
	{
		for (int i = 0; i < (int)Mol::Count; ++i)
		{
			if (cost[i] > player.storage[i] + player.expertise[i])
				return false;
		}
		return true;
	}

	bool hasMolecules(const playerStruct& player, int molecule) const
	{
		if (cost[molecule] <= player.storage[molecule] + player.expertise[molecule])
			return true;
		return false;
	}

	int getMissingMoleculesCount(const playerStruct& player) const
	{
		int missing = 0;
		for (int i = 0; i < (int)Mol::Count; ++i)
		{
			missing += max(cost[i] - (player.storage[i] + player.expertise[i]), 0);
		}
		return missing;
	}

	float getCost(const playerStruct& player) const
	{
		return (float)health / (float)(getMissingMoleculesCount(player) + 1);
	}

	bool isDiagnosed() const
	{
		return cost[0] >= 0;
	}
};

ostream& operator << (ostream& out, const sampleStruct& sample);
istream& operator >> (istream& input, sampleStruct& sample);

struct samplesCollectionStruct
{
	vector<sampleStruct> samples;
};

ostream& operator << (ostream& out, const samplesCollectionStruct& collection);
istream& operator >> (istream& input, samplesCollectionStruct& collection);

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

	myPlayerStruct player{};
	playerStruct enemy{};

	samplesCollectionStruct collection{};

	suppliesStruct supplies{};

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
		vector<sampleStruct> samplesOfPlayer;
		copy_if(collection.samples.begin(), collection.samples.end(), back_inserter(samplesOfPlayer), [](const sampleStruct& sample) { return sample.carriedBy == 0; });

		if (samplesOfPlayer.empty())
		{
			collection.samples.erase(remove_if(collection.samples.begin(), collection.samples.end(), [](const sampleStruct& sample) { return sample.carriedBy != -1; }), collection.samples.end());
			sort(collection.samples.begin(), collection.samples.end(), [&player](const sampleStruct& first, const sampleStruct& second)
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
				for (int i = 0; i < (int)Mol::Count; ++i)
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


ostream& operator << (ostream& out, const playerStruct& player)
{
	out << player.target << " " << player.eta << " " << player.score;
	for (int i = 0; i < (int)Mol::Count; ++i)
		out << " " << player.storage[i];
	for (int i = 0; i < (int)Mol::Count; ++i)
		out << " " << player.expertise[i];
	return out;
}
istream& operator >> (istream& input, playerStruct& player)
{
	input >> player.target >> player.eta >> player.score;
	for (int i = 0; i < (int)Mol::Count; ++i)
		input >> player.storage[i];
	for (int i = 0; i < (int)Mol::Count; ++i)
		input >> player.expertise[i];
	return input;
}


ostream& operator << (ostream& out, const suppliesStruct& supplies)
{
	for (int i = 0; i < (int)Mol::Count; ++i)
	{
		if (i > 0)
			out << " ";
		out << supplies.available[i];
	}
	return out;
}
istream& operator >> (istream& input, suppliesStruct& supplies)
{
	for (int i = 0; i < (int)Mol::Count; ++i)
		input >> supplies.available[i];
	return input;
}


ostream& operator << (ostream& out, const sampleStruct& sample)
{
	out << sample.sampleId << " " << sample.carriedBy << " " << sample.rank << " " << sample.expertiseGain << " " << sample.health;
	for (int i = 0; i < (int)Mol::Count; ++i)
		out << " " << sample.cost[i];
	return out;
}
istream& operator >> (istream& input, sampleStruct& sample)
{
	input >> sample.sampleId >> sample.carriedBy >> sample.rank >> sample.expertiseGain >> sample.health;
	for (int i = 0; i < (int)Mol::Count; ++i)
		input >> sample.cost[i];
	return input;
}


ostream& operator << (ostream& out, const samplesCollectionStruct& collection)
{
	out << collection.samples.size();

	for (int i = 0; i < (int)collection.samples.size(); ++i)
	{
		out << endl;
		out << collection.samples[i];
	}

	return out;
}
istream& operator >> (istream& input, samplesCollectionStruct& collection)
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
