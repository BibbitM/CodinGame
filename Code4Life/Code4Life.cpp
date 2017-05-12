#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>

using namespace std;

static const string targetDiagnosis = "DIAGNOSIS";
static const string targetMolecules = "MOLECULES";
static const string targetLaboratory = "LABORATORY";

enum class Mol
{
	A,
	B,
	C,
	D,
	E,
	Count
};

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
};

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
			if (cost[i] > player.storage[i] - player.expertise[i])
				return false;
		}
		return true;
	}

	bool hasMolecules(const playerStruct& player, int molecule) const
	{
		if (cost[molecule] <= player.storage[molecule] - player.expertise[molecule])
			return true;
		return false;
	}

	int getMissingMoleculesCount(const playerStruct& player) const
	{
		int missing = 0;
		for (int i = 0; i < (int)Mol::Count; ++i)
		{
			missing += max(cost[i] - (player.storage[i] - player.expertise[i]), 0);
		}
		return missing;
	}

	float getCost(const playerStruct& player) const
	{
		return (float)health / (float)(getMissingMoleculesCount(player) + 1);
	}
};

ostream& operator<<(ostream& out, const sampleStruct& sample)
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

namespace cmd
{
	void goTo(const string& target) { cout << "GOTO " << target << endl; }
	void goToDiagnosis() { goTo(targetDiagnosis); }
	void goToMolecules() { goTo(targetMolecules); }
	void goToLaboratory() { goTo(targetLaboratory); }

	void connectId(int id) { cout << "CONNECT " << id << endl; }
	void connectType(int type) { cout << "CONNECT " << (char)('A' + type) << endl; }
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

	playerStruct player;
	playerStruct enemy;

	vector<sampleStruct> samples;

	// game loop
	while (1)
	{
		cin >> player; cin.ignore();
		//cerr << player << endl;
		cin >> enemy; cin.ignore();
		//cerr << enemy << endl;

		int availableA;
		int availableB;
		int availableC;
		int availableD;
		int availableE;
		cin >> availableA >> availableB >> availableC >> availableD >> availableE; cin.ignore();
		//cerr << availableA << " " << availableB << " " << availableC << " " << availableD << " " << availableE << endl;
		int sampleCount;
		cin >> sampleCount; cin.ignore();
		//cerr << sampleCount << endl;

		samples.resize(sampleCount);

		for (int i = 0; i < sampleCount; i++)
		{
			cin >> samples[i]; cin.ignore();
			//cerr << samples[i] << endl;
		}

		// Collect sample.
		vector<sampleStruct> samplesOfPlayer;
		copy_if(samples.begin(), samples.end(), back_inserter(samplesOfPlayer), [](const sampleStruct& sample) { return sample.carriedBy == 0; });

		if (samplesOfPlayer.empty())
		{
			if (player.isInDiagnosis())
			{
				remove_if(samples.begin(), samples.end(), [](const sampleStruct& sample) { return sample.carriedBy != -1; });
				sort(samples.begin(), samples.end(), [&player](const sampleStruct& first, const sampleStruct& second)
				{
					return first.getCost(player) > second.getCost(player);
				});
				if (!samples.empty())
					cmd::connectId(samples[0].sampleId);
				else
					cmd::goToDiagnosis();
			}
			else
			{
				cmd::goToDiagnosis();
			}
		}
		else
		{
			const auto& sampleToCollect = samplesOfPlayer[0];
			if (sampleToCollect.hasAllMolecules(player))
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
				for (int i = 0; i < (int)Mol::Count; ++i)
				{
					if (!sampleToCollect.hasMolecules(player, i))
					{
						cmd::connectType(i);
						break;
					}
				}
			}
			else
			{
				cmd::goToMolecules();
			}
		}
		/*
		else if (player.isInDiagnosis())
		{
			cout << "GOTO " << targetMolecules << endl;
		}
		else if (player.isInMolecules())
		{
			cout << "GOTO " << targetLaboratory << endl;
		}
		else if (player.isInLaboratory())
		{
			cout << "GOTO " << targetDiagnosis << endl;
		}
		else // START_POS
		{
			cout << "GOTO " << targetDiagnosis << endl;
		}
		*/
	}
}
