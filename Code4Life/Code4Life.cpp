#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <cassert>
#include <numeric>

using namespace std;

static const string targetStartPos = "START_POS";
static const string targetSamples = "SAMPLES";
static const string targetDiagnosis = "DIAGNOSIS";
static const string targetMolecules = "MOLECULES";
static const string targetLaboratory = "LABORATORY";

static const int maxSamplesPerPlayer = 3;
static const int maxMoleculesPerPlayer = 10;
static const int pointsToWin = 170;

enum class eMol
{
	A,
	B,
	C,
	D,
	E,
	count
};

int getMoleculeFromString(const string& moleculeName);

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

	int getMoleculeNum(int molecule) const { return storage[molecule] + expertise[molecule]; }
	int getStorageMoleculesNum() const
	{
		return accumulate(begin(storage), end(storage), 0);
	}
	int getExpretiseMoleculesNum() const
	{
		return accumulate(begin(expertise), end(expertise), 0);
	}

	bool isInSamples() const { return target == targetSamples; }
	bool isInDiagnosis() const { return target == targetDiagnosis; }
	bool isInMolecules() const { return target == targetMolecules; }
	bool isInLaboratory() const { return target == targetLaboratory; }
};


enum class eState
{
	start,
	collectSamples,
	analizeSamples,
	chooseSamples,
	gatherMolecules,
	returnSamples,
	produceMedicines,
};

string toString(eState state)
{
	switch (state)
	{
	case eState::start:				return "Start";
	case eState::collectSamples:	return "Collect samples";
	case eState::analizeSamples:	return "Analize samples";
	case eState::chooseSamples:		return "Choose samples";
	case eState::gatherMolecules:	return "Gather molecules";
	case eState::returnSamples:		return "Return samples";
	case eState::produceMedicines:	return "Produce medicines";
	default: return "";
	}
}

ostream& operator << (ostream& out, eState state)
{
	out << toString(state);
	return out;
}

struct sSamplesCollection;
struct sSupplies;

struct sLocalPlayer : sPlayer
{
	eState state;
	bool shouldLog;

	string message;

	sLocalPlayer() : state(eState::start), shouldLog(false) { }

	void update(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies);
	bool updateState(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies);
	bool updateStart(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies);
	bool updateCollectSamples(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies);
	bool updateAnalizeSamples(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies);
	bool updateChooseSamples(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies);
	bool updateGatherMolecules(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies);
	bool updateReturnSamples(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies);
	bool updateProduceMedicines(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies);

	void setState(eState newState)
	{
		if (shouldLog)
			cerr << "Set state " << newState << " from " << state << "." << endl;
		state = newState;
	}

	string getMessage() const
	{
		if (!shouldLog)
			return string();

		string outMessage = toString(state);
		if (!message.empty())
		{
			outMessage += " ";
			outMessage += message;
		}

		return outMessage;
	}
	void addMessage(const string& msg)
	{
		if (message.empty())
			message = msg;
		else
		{
			message += " ";
			message += msg;
		}
	}
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

	void sortByHealth();
	int getNumSamplesCarriedBy(int carriedBy) const;
	sSamplesCollection getSamplesCarriedBy(int carriedBy) const;
	sSamplesCollection getSamplesToAnalize() const;
	sSamplesCollection getDiagnosedSamples() const;
};

ostream& operator << (ostream& out, const sSamplesCollection& collection);
istream& operator >> (istream& input, sSamplesCollection& collection);

namespace cmd
{
	void send(const string& message = string()) {  if (!message.empty()) cout << " " << message; cout << endl; }
	void goTo(const string& target, const string& message = string()) { cout << "GOTO " << target; send(message); }
	void goToDiagnosis(const string& message = string()) { goTo(targetDiagnosis, message); }
	void goToMolecules(const string& message = string()) { goTo(targetMolecules, message); }
	void goToLaboratory(const string& message = string()) { goTo(targetLaboratory, message); }
	void goToSamples(const string& message = string()) { goTo(targetSamples, message); }

	void wait(const string& message = string()) { cout << "WAIT"; send(message);	}
	void null(const string& message = string()) { send(message); }

	void connectId(int id, const string& message = string()) { cout << "CONNECT " << id; send(message); }
	void connectType(int type, const string& message = string()) { cout << "CONNECT " << (char)('A' + type); send(message); }
	void connectRank(int rank, const string& message = string()) { cout << "CONNECT " << rank; send(message); }
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


		collection.sortByHealth();

		/*
		cerr << player << endl;
		cerr << enemy << endl;
		cerr << supplies << endl;
		cerr << collection << endl;
		player.shouldLog = true;
		//*/

		bool update = true;
		if (update)
			player.update(enemy, collection, supplies);
		else
		{
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


void sSamplesCollection::sortByHealth()
{
	sort(samples.begin(), samples.end(), [](const sSample& first, const sSample& second)
	{
		float firstHealth = first.isDiagnosed() ? (float)first.health : rankHealthPoints[first.rank];
		float secondHealth = second.isDiagnosed() ? (float)second.health : rankHealthPoints[second.rank];

		if (firstHealth == secondHealth)
			return first.sampleId > second.sampleId;

		return firstHealth < secondHealth;
	});
}

int sSamplesCollection::getNumSamplesCarriedBy(int carriedBy) const
{
	int numSamples = 0;
	for (const auto& s : samples)
	{
		if (s.carriedBy == carriedBy)
			++numSamples;
	}
	return numSamples;
}

sSamplesCollection sSamplesCollection::getSamplesCarriedBy(int carriedBy) const
{
	sSamplesCollection outCollection{};
	outCollection.samples.reserve(samples.size());

	for (const auto& s : samples)
	{
		if (s.carriedBy == carriedBy)
			outCollection.samples.push_back(s);
	}

	return outCollection;
}

sSamplesCollection sSamplesCollection::getSamplesToAnalize() const
{
	sSamplesCollection outCollection{};
	outCollection.samples.reserve(samples.size());

	for (const auto& s : samples)
	{
		if (!s.isDiagnosed())
			outCollection.samples.push_back(s);
	}

	return outCollection;
}

sSamplesCollection sSamplesCollection::getDiagnosedSamples() const
{
	sSamplesCollection outCollection{};
	outCollection.samples.reserve(samples.size());

	for (const auto& s : samples)
	{
		if (s.isDiagnosed())
			outCollection.samples.push_back(s);
	}

	return outCollection;
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

int getMoleculeFromString(const string& moleculeName)
{
	if (moleculeName == "A")
		return (int)eMol::A;
	if (moleculeName == "B")
		return (int)eMol::B;
	if (moleculeName == "C")
		return (int)eMol::C;
	if (moleculeName == "D")
		return (int)eMol::D;
	if (moleculeName == "E")
		return (int)eMol::E;
	return -1;
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


void sLocalPlayer::update(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies)
{
	while (!updateState(enemy, collection, supplies))
		continue;
}

bool sLocalPlayer::updateState(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies)
{
	message.clear();

	if (eta > 0)
	{
		cmd::null(getMessage());
		return true;
	}

	switch (state)
	{
	case eState::start:
		return updateStart(enemy, collection, supplies);
	case eState::collectSamples:
		return updateCollectSamples(enemy, collection, supplies);
	case eState::analizeSamples:
		return updateAnalizeSamples(enemy, collection, supplies);
	case eState::chooseSamples:
		return updateChooseSamples(enemy, collection, supplies);
	case eState::gatherMolecules:
		return updateGatherMolecules(enemy, collection, supplies);
	case eState::returnSamples:
		return updateReturnSamples(enemy, collection, supplies);
	case eState::produceMedicines:
		return updateProduceMedicines(enemy, collection, supplies);
	default:
		cmd::wait("ERROR");
		return true;
	}
}

bool sLocalPlayer::updateStart(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies)
{
	setState(eState::collectSamples);
	return false;
}

bool sLocalPlayer::updateCollectSamples(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies)
{
	const int mySamplesNum = collection.getNumSamplesCarriedBy(0);
	if (mySamplesNum >= maxSamplesPerPlayer)
	{
		setState(eState::analizeSamples);
		return false;
	}

	if (isInSamples())
	{
		int sampleRank = 1;
		const int totalExpertise = getExpretiseMoleculesNum();
		if (totalExpertise >= rankMoleculeCosts[2])
			sampleRank = 2;
		if (totalExpertise >= rankMoleculeCosts[3])
			sampleRank = 3;

		cmd::connectRank(sampleRank, getMessage());
		return true;
	}

	cmd::goToSamples(getMessage());
	return true;
}

bool sLocalPlayer::updateAnalizeSamples(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies)
{
	auto mySamplesToAnalize = collection.getSamplesCarriedBy(0).getSamplesToAnalize();
	if (mySamplesToAnalize.samples.empty())
	{
		setState(eState::chooseSamples);
		return false;
	}

	if (isInDiagnosis())
	{
		cmd::connectId(mySamplesToAnalize.samples[0].sampleId, getMessage());
		return true;
	}

	cmd::goToDiagnosis(getMessage());
	return true;
}

bool sLocalPlayer::updateChooseSamples(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies)
{
	// TODO
	setState(eState::gatherMolecules);
	return false;
}

bool sLocalPlayer::updateGatherMolecules(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies)
{
	auto myDiagnosedSamples = collection.getSamplesCarriedBy(0).getDiagnosedSamples();

	int wantedType = -1;

	int usedStorage[(int)eMol::count] = {};
	int gainExpertise[(int)eMol::count] = {};

	for (const auto& sample : myDiagnosedSamples.samples)
	{
		bool missingSupplies = false;
		int totalMissingSupplies = 0;
		for (int i = 0; i < (int)eMol::count; ++i)
		{
			const int moleculeToGather = max(sample.cost[i] - (getMoleculeNum(i) - usedStorage[i] + gainExpertise[i]), 0);
			totalMissingSupplies += moleculeToGather;
			if (moleculeToGather > supplies.available[i])
			{
				missingSupplies = true;
				break;
			}
		}

		if (missingSupplies)
		{
			addMessage("missingSupplies");
			continue;
		}
		if (getStorageMoleculesNum() + totalMissingSupplies > maxMoleculesPerPlayer)
		{
			addMessage("toMuchTotal");
			continue;
		}

		for (int i = 0; i < (int)eMol::count; ++i)
		{
			const int moleculeToGather = max(sample.cost[i] - (getMoleculeNum(i) - usedStorage[i] + gainExpertise[i]), 0);
			if (moleculeToGather > 0)
			{
				wantedType = i;
				break;
			}
		}

		// We have all molecules.
		if (wantedType == -1)
		{
			addMessage("weHaveAll");
			for (int i = 0; i < (int)eMol::count; ++i)
			{
				usedStorage[i] += max(sample.cost[i] - expertise[i] - gainExpertise[i], 0);
			}

			int extertiseInMolecule = getMoleculeFromString(sample.expertiseGain);
			if (extertiseInMolecule >= 0 && extertiseInMolecule < (int)eMol::count)
				++gainExpertise[extertiseInMolecule];

			continue;
		}

		break;
	}

	if (wantedType == -1 || getStorageMoleculesNum() >= maxMoleculesPerPlayer)
	{
		setState(eState::produceMedicines);
		return false;
	}

	if (isInMolecules())
	{
		cmd::connectType(wantedType, getMessage());
		return true;
	}

	cmd::goToMolecules(getMessage());
	return true;
}

bool sLocalPlayer::updateReturnSamples(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies)
{
	auto mySamples = collection.getSamplesCarriedBy(0);
	if (mySamples.samples.empty())
	{
		setState(eState::collectSamples);
		return false;
	}

	if (isInDiagnosis())
	{
		cmd::connectId(mySamples.samples[0].sampleId, getMessage());
		return true;
	}

	cmd::goToDiagnosis(getMessage());
	return true;
}

bool sLocalPlayer::updateProduceMedicines(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies)
{
	auto myDiagnosedSamples = collection.getSamplesCarriedBy(0).getDiagnosedSamples();

	int toProduceId = -1;

	for (const auto& sample : myDiagnosedSamples.samples)
	{
		bool missingSupplies = false;
		for (int i = 0; i < (int)eMol::count; ++i)
		{
			const int moleculeToGather = max(sample.cost[i] - getMoleculeNum(i), 0);
			if (moleculeToGather > 0)
			{
				missingSupplies = true;
				break;
			}
		}

		if (missingSupplies)
			continue;

		toProduceId = sample.sampleId;
		break;
	}

	if (toProduceId == -1)
	{
		if (collection.getNumSamplesCarriedBy(0) == maxSamplesPerPlayer)
			setState(eState::returnSamples);
		else
			setState(eState::collectSamples);
		return false;
	}

	if (isInLaboratory())
	{
		cmd::connectId(toProduceId, getMessage());
		return true;
	}

	cmd::goToLaboratory(getMessage());
	return true;
}
