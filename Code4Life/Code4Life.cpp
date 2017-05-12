#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

static const string targetDiagnosis = "DIAGNOSIS";
static const string targetMolecules = "MOLECULES";
static const string targetLaboratory = "LABORATORY";

struct playerStuct
{
	string target;
	int eta;
	int score;
	int storageA;
	int storageB;
	int storageC;
	int storageD;
	int storageE;
	int expertiseA;
	int expertiseB;
	int expertiseC;
	int expertiseD;
	int expertiseE;

	bool isInDiagnosis() const { return target == targetDiagnosis; }
	bool isInMolecules() const { return target == targetMolecules; }
	bool isInLaboratory() const { return target == targetLaboratory; }
};

ostream& operator << (ostream& out, const playerStuct& player)
{
	out << player.target << " " << player.eta << " " << player.score << " " << player.storageA << " " << player.storageB << " " << player.storageC << " " << player.storageD << " " << player.storageE << " " << player.expertiseA << " " << player.expertiseB << " " << player.expertiseC << " " << player.expertiseD << " " << player.expertiseE;
	return out;
}
istream& operator >> (istream& input, playerStuct& player)
{
	input >> player.target >> player.eta >> player.score >> player.storageA >> player.storageB >> player.storageC >> player.storageD >> player.storageE >> player.expertiseA >> player.expertiseB >> player.expertiseC >> player.expertiseD >> player.expertiseE;
	return input;
}

struct sampleStruct
{
	int sampleId;
	int carriedBy;
	int rank;
	string expertiseGain;
	int health;
	int costA;
	int costB;
	int costC;
	int costD;
	int costE;
};

ostream& operator<<(ostream& out, const sampleStruct& sample)
{
	out << sample.sampleId << " " << sample.carriedBy << " " << sample.rank << " " << sample.expertiseGain << " " << sample.health << " " << sample.costA << " " << sample.costB << " " << sample.costC << " " << sample.costD << " " << sample.costE;
	return out;
}
istream& operator >> (istream& input, sampleStruct& sample)
{
	input >> sample.sampleId >> sample.carriedBy >> sample.rank >> sample.expertiseGain >> sample.health >> sample.costA >> sample.costB >> sample.costC >> sample.costD >> sample.costE;
	return input;
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

	playerStuct player;
	playerStuct enemy;

	vector<sampleStruct> samples;

	// game loop
	while (1)
	{
		cin >> player; cin.ignore();
		cerr << player << endl;
		cin >> enemy; cin.ignore();
		cerr << enemy << endl;

		int availableA;
		int availableB;
		int availableC;
		int availableD;
		int availableE;
		cin >> availableA >> availableB >> availableC >> availableD >> availableE; cin.ignore();
		cerr << availableA << " " << availableB << " " << availableC << " " << availableD << " " << availableE << endl;
		int sampleCount;
		cin >> sampleCount; cin.ignore();
		cerr << sampleCount << endl;

		samples.resize(sampleCount);

		for (int i = 0; i < sampleCount; i++)
		{
			cin >> samples[i]; cin.ignore();
			cerr << samples[i] << endl;
		}

		if (player.isInDiagnosis())
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
	}
}
