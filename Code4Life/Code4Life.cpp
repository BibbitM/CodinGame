#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <cassert>
#include <numeric>

#define LOG_MESSAGES 0

using namespace std;

static const string targetStartPos = "START_POS";
static const string targetSamples = "SAMPLES";
static const string targetDiagnosis = "DIAGNOSIS";
static const string targetMolecules = "MOLECULES";
static const string targetLaboratory = "LABORATORY";

static const int maxSamplesPerPlayer = 3;
static const int maxMoleculePerType = 5;
static const int maxMoleculesPerPlayer = 10;
static const int projectHealthPoints = 50;
static const int maxRounds = 200;

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
eArea getAreaFromString(const string& target);
int getAreaMoveCost(eArea start, eArea end);

float rankHealthPoints[4] = { 0.0f, 2.125f, 18.62068966f, 40.f };
int rankHealthPointsMin[4] = { 0,  1, 10, 30 };
int rankHealthPointsMax[4] = { 0, 10, 30, 50 };
int rankMinMoleculeCosts[4] = { 0, 3, 5,  7 };
int rankMaxMoleculeCosts[4] = { 0, 5, 8, 14 };

struct sProjectsCollection;
struct sSample;
struct sSamplesCollection;
struct sSupplies;

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

	int getMostWantedMoleculesWeight(const sSample& sample, const sSupplies& supplies, int molecule) const;
};


enum class eState
{
	start,
	collectSamples,
	analyzeSamples,
	chooseSamples,
	gatherMolecules,
	returnSamples,
	produceMedicines,
	randomMove,
};

string toString(eState state)
{
	switch (state)
	{
	case eState::start:				return "Start";
	case eState::collectSamples:	return "Collect samples";
	case eState::analyzeSamples:	return "Analyze samples";
	case eState::chooseSamples:		return "Choose samples";
	case eState::gatherMolecules:	return "Gather molecules";
	case eState::returnSamples:		return "Return samples";
	case eState::produceMedicines:	return "Produce medicines";
	case eState::randomMove:		return "Random move";
	default: return "";
	}
}

string toShortString(eState state)
{
	switch (state)
	{
	case eState::start:				return "ST";
	case eState::collectSamples:	return "CS";
	case eState::analyzeSamples:	return "AS";
	case eState::chooseSamples:		return "CS";
	case eState::gatherMolecules:	return "GM";
	case eState::returnSamples:		return "RS";
	case eState::produceMedicines:	return "PM";
	case eState::randomMove:		return "RM";
	default: return "";
	}
}

ostream& operator << (ostream& out, eState state)
{
	out << toString(state);
	return out;
}

struct sLocalPlayer : sPlayer
{
	eState state;
	bool shouldLog;

	string message;

	int round;

	sLocalPlayer() : state(eState::start), shouldLog(false), round(0) { }

	bool cmdGoTo(eArea area, const sSamplesCollection& collection);
	void update(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies, const sProjectsCollection& projects);
	bool updateState(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies, const sProjectsCollection& projects);
	bool updateStart(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies, const sProjectsCollection& projects);
	bool updateCollectSamples(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies, const sProjectsCollection& projects);

	int getCollectSampleRank(const sProjectsCollection &projects, const int mySamplesNum) const;

	bool updateAnalyzeSamples(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies, const sProjectsCollection& projects);
	bool updateChooseSamples(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies, const sProjectsCollection& projects);
	bool updateGatherMolecules(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies, const sProjectsCollection& projects);
	bool updateReturnSamples(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies, const sProjectsCollection& projects);
	bool updateProduceMedicines(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies, const sProjectsCollection& projects);
	bool updateRandomMove(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies, const sProjectsCollection& projects);

	int getRoundsNeededToProduceMedicines(const sSamplesCollection& collection, bool withMove);

	void getMostWantedMoleculesIdx(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies, const sProjectsCollection& projects, int(&moleculeIdx)[(int)eMol::count]) const;

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

		string outMessage = toShortString(state);
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

	bool operator==(const sSample& right) const
	{
		return sampleId == right.sampleId;
	}

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

	int getProductionCost(const sPlayer& player) const
	{
		int productionCost = 0;
		if (!isDiagnosed())
			productionCost += 1;
		if (carriedBy != 0)
			productionCost += 2;

		if (isDiagnosed())
		{
			for (int i = 0; i < (int)eMol::count; ++i)
			{
				productionCost += max(cost[i] - player.getMoleculeNum(i), 0);
			}
		}
		else
		{
			int moleculesToGet = max(rankMaxMoleculeCosts[rank] + rankMinMoleculeCosts[rank] - player.getExpretiseMoleculesNum() - player.getStorageMoleculesNum(), 0);
			productionCost += moleculesToGet;
		}

		return productionCost;
	}

	bool areAllMoleculesAvailable(const sPlayer& player, const sSupplies& supplies) const
	{
		int totalMissingSupplies = 0;

		for (int i = 0; i < (int)eMol::count; ++i)
		{
			const int moleculeToGather = max(cost[i] - player.getMoleculeNum(i), 0);
			totalMissingSupplies += moleculeToGather;
			if (moleculeToGather > supplies.available[i])
			{
				return false;
			}
		}

		if (player.getStorageMoleculesNum() + totalMissingSupplies > maxMoleculesPerPlayer)
			return false;

		return true;
	}
};

ostream& operator << (ostream& out, const sSample& sample);
istream& operator >> (istream& input, sSample& sample);

struct sProjectsCollection;

struct sSamplesCollection
{
	vector<sSample> samples;

	void sortByHealthAndRoundsToProduce(const sPlayer& player, const sProjectsCollection& projects, const sSupplies& supplies);
	int getNumSamplesCarriedBy(int carriedBy) const;
	sSamplesCollection getSamplesCarriedBy(int carriedBy) const;
	sSamplesCollection getSamplesNotCarriedBy(int carriedBy) const;
	sSamplesCollection getSamplesToAnalyze() const;
	sSamplesCollection getDiagnosedSamples() const;
	sSamplesCollection getSamplesWithAvailableMoleculses(const sPlayer& player, const sSupplies& supplies) const;
};

ostream& operator << (ostream& out, const sSamplesCollection& collection);
istream& operator >> (istream& input, sSamplesCollection& collection);

struct sProject
{
	int required[(int)eMol::count];

	bool isGained(const sPlayer& player) const;
	int getMissingMolecules(const sPlayer& player, int molecule) const;
	int getTotalMissing(const sPlayer& player) const;
	float getHealthBonus(const sPlayer& player, int molecule) const;
};

ostream& operator << (ostream& out, const sProject& project);
istream& operator >> (istream& input, sProject& project);

struct sProjectsCollection
{
	vector<sProject> projects;

	float getHealtBonus(const sPlayer& player, int molecule) const;
	int getNotGainedProjectsNum(const sPlayer& player) const;
};

ostream& operator << (ostream& out, const sProjectsCollection& collection);
istream& operator >> (istream& input, sProjectsCollection& collection);


namespace cmd
{
	void send(const string& message = string()) {  if (!message.empty()) cout << " " << message; cout << endl; }
	void goTo(const string& target, const string& message = string()) { cout << "GOTO " << target; send(message); }
	void goTo(eArea area, const string& message = string()) { goTo(getAreaName(area), message); }
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

	sLocalPlayer player{};
	sPlayer enemy{};

	sSamplesCollection collection{};

	sSupplies supplies{};

	sProjectsCollection projects{};

	cin >> projects;

	// game loop
	while (1)
	{
		cin >> player; cin.ignore();
		cin >> enemy; cin.ignore();

		cin >> supplies; cin.ignore();

		cin >> collection; cin.ignore();


		collection.sortByHealthAndRoundsToProduce(player, projects, supplies);

#if LOG_MESSAGES
		player.shouldLog = true;

		cerr << projects << endl;
		for (const auto& proj : projects.projects)
		{
			cerr << proj.getTotalMissing(player) << " ";
		}
		cerr << endl;
		//cerr << "NotGainedNum " << projects.getNotGainedProjectsNum(player) << endl;
		cerr << player << endl;
		cerr << enemy << endl;
		cerr << supplies << endl;
		cerr << collection << endl;
#endif

		player.update(enemy, collection, supplies, projects);
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


void sSamplesCollection::sortByHealthAndRoundsToProduce(const sPlayer& player, const sProjectsCollection& projects, const sSupplies& supplies)
{
	const int timeToMove = getAreaMoveCost(eArea::diagnosis, eArea::molecules) + getAreaMoveCost(eArea::molecules, eArea::laboratory);

	sort(samples.begin(), samples.end(), [&player, &projects, &supplies, timeToMove](const sSample& first, const sSample& second)
	{
		float firstHealth = first.isDiagnosed() ? (float)first.health : rankHealthPoints[first.rank];
		float secondHealth = second.isDiagnosed() ? (float)second.health : rankHealthPoints[second.rank];

		firstHealth += projects.getHealtBonus(player, getMoleculeFromString(first.expertiseGain));
		secondHealth += projects.getHealtBonus(player, getMoleculeFromString(second.expertiseGain));

		float firstGetFactor = firstHealth / (first.getProductionCost(player) + timeToMove + 1);
		float secondGetFactor = secondHealth / (second.getProductionCost(player) + timeToMove + 1);

		if (firstGetFactor == secondGetFactor)
			return first.sampleId < second.sampleId;

		return firstGetFactor > secondGetFactor;
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

sSamplesCollection sSamplesCollection::getSamplesNotCarriedBy(int carriedBy) const
{
	sSamplesCollection outCollection{};
	outCollection.samples.reserve(samples.size());

	for (const auto& s : samples)
	{
		if (s.carriedBy != carriedBy)
			outCollection.samples.push_back(s);
	}

	return outCollection;
}

sSamplesCollection sSamplesCollection::getSamplesToAnalyze() const
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

sSamplesCollection sSamplesCollection::getSamplesWithAvailableMoleculses(const sPlayer& player, const sSupplies& supplies) const
{
	sSamplesCollection outCollection{};
	outCollection.samples.reserve(samples.size());

	for (const auto& s : samples)
	{
		if (s.areAllMoleculesAvailable(player, supplies))
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

bool sProject::isGained(const sPlayer& player) const
{
	bool allGained = true;
	for (int i = 0; i < (int)eMol::count; ++i)
	{
		if (required[i] > player.expertise[i])
		{
			allGained = false;
			break;
		}
	}
	return allGained;
}

int sProject::getMissingMolecules(const sPlayer& player, int molecule) const
{
	return max(required[molecule] - player.expertise[molecule], 0);
}

int sProject::getTotalMissing(const sPlayer& player) const
{
	int totalMissing = 0;
	for (int i = 0; i < (int)eMol::count; ++i)
		totalMissing += getMissingMolecules(player, i);
	return totalMissing;
}

float sProject::getHealthBonus(const sPlayer& player, int molecule) const
{
	if (getMissingMolecules(player, molecule) == 0)
		return 0;

	return (float)projectHealthPoints / (float)((getTotalMissing(player) - 1) * 1 + 1);
}

ostream& operator << (ostream& out, const sProject& project)
{
	for (int i = 0; i < (int)eMol::count; ++i)
	{
		if (i > 0)
			out << " ";
		out << project.required[i];
	}
	return out;
}
istream& operator >> (istream& input, sProject& project)
{
	for (int i = 0; i < (int)eMol::count; ++i)
		input >> project.required[i];
	return input;
}


float sProjectsCollection::getHealtBonus(const sPlayer& player, int molecule) const
{
	float totalBonus = 0.0f;
	for (const auto& proj : projects)
		totalBonus += proj.getHealthBonus(player, molecule);
	return totalBonus;
}

int sProjectsCollection::getNotGainedProjectsNum(const sPlayer& player) const
{
	int notGainedNum = 0;
	for (const auto& proj : projects)
		if (!proj.isGained(player))
			++notGainedNum;
	return notGainedNum;
}

ostream& operator << (ostream& out, const sProjectsCollection& collection)
{
	out << collection.projects.size();

	for (int i = 0; i < (int)collection.projects.size(); ++i)
	{
		out << endl;
		out << collection.projects[i];
	}

	return out;
}
istream& operator >> (istream& input, sProjectsCollection& collection)
{
	int projectCount;
	input >> projectCount;

	collection.projects.resize(projectCount);

	for (int i = 0; i < projectCount; ++i)
	{
		input.ignore();
		input >> collection.projects[i];
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

eArea getAreaFromString(const string& target)
{
	if (target == targetStartPos)
		return eArea::start;
	if (target == targetSamples)
		return eArea::samples;
	if (target == targetDiagnosis)
		return eArea::diagnosis;
	if (target == targetMolecules)
		return eArea::molecules;
	if (target == targetLaboratory)
		return eArea::laboratory;

	return eArea::start;
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


bool sLocalPlayer::cmdGoTo(eArea area, const sSamplesCollection& collection)
{
	const int roundToProduce = getRoundsNeededToProduceMedicines(collection, false);
	if (roundToProduce > 0 && state != eState::produceMedicines)
	{
		int roundToProdceWithMove = roundToProduce + getAreaMoveCost(getAreaFromString(target), area) + getAreaMoveCost(area, eArea::laboratory);

		if (round + roundToProdceWithMove > maxRounds)
		{
			addMessage("GoProd");
			setState(eState::produceMedicines);
			return false;
		}
	}

	cmd::goTo(area, getMessage());
	return true;
}

void sLocalPlayer::update(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies, const sProjectsCollection& projects)
{
	++round;

	message.clear();

	if (eta > 0)
	{
		cmd::null(getMessage());
		return;
	}

	const int roundToProduce = getRoundsNeededToProduceMedicines(collection, true);
	if (shouldLog)
	{
		cerr << "R: " << round << " RTP: " << roundToProduce << endl;
	}

	if (roundToProduce > 0 && state != eState::produceMedicines && round + roundToProduce > maxRounds)
	{
		addMessage("UpProd");
		setState(eState::produceMedicines);
	}

	while (!updateState(enemy, collection, supplies, projects))
		continue;
}

bool sLocalPlayer::updateState(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies, const sProjectsCollection& projects)
{
	switch (state)
	{
	case eState::start:
		return updateStart(enemy, collection, supplies, projects);
	case eState::collectSamples:
		return updateCollectSamples(enemy, collection, supplies, projects);
	case eState::analyzeSamples:
		return updateAnalyzeSamples(enemy, collection, supplies, projects);
	case eState::chooseSamples:
		return updateChooseSamples(enemy, collection, supplies, projects);
	case eState::gatherMolecules:
		return updateGatherMolecules(enemy, collection, supplies, projects);
	case eState::returnSamples:
		return updateReturnSamples(enemy, collection, supplies, projects);
	case eState::produceMedicines:
		return updateProduceMedicines(enemy, collection, supplies, projects);
	case eState::randomMove:
		return updateRandomMove(enemy, collection, supplies, projects);
	default:
		cmd::wait("ERROR");
		return true;
	}
}

bool sLocalPlayer::updateStart(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies, const sProjectsCollection& projects)
{
	setState(eState::collectSamples);
	return false;
}

bool sLocalPlayer::updateCollectSamples(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies, const sProjectsCollection& projects)
{
	const int mySamplesNum = collection.getNumSamplesCarriedBy(0);
	if (mySamplesNum >= maxSamplesPerPlayer)
	{
		setState(eState::analyzeSamples);
		return false;
	}

	if (isInSamples())
	{
		int sampleRank = getCollectSampleRank(projects, mySamplesNum);

		cmd::connectRank(sampleRank, getMessage());
		return true;
	}
	else
	{
		auto myDiagnosedSamples = collection.getSamplesCarriedBy(0).getDiagnosedSamples().getSamplesWithAvailableMoleculses(*this, supplies);
		if (myDiagnosedSamples.samples.size() > 1 ||
			(myDiagnosedSamples.samples.size() > 0 && myDiagnosedSamples.samples[0].health + projects.getHealtBonus(*this, getMoleculeFromString(myDiagnosedSamples.samples[0].expertiseGain)) >= rankHealthPointsMin[3]))
		{
			setState(eState::gatherMolecules);
			return false;
		}
	}

	return cmdGoTo(eArea::samples, collection);
}

int sLocalPlayer::getCollectSampleRank(const sProjectsCollection &projects, const int mySamplesNum) const
{
	int sampleRank = 1;

	const int projectsToDevelop = projects.getNotGainedProjectsNum(*this);
	const int totalExpertise = getExpretiseMoleculesNum() - mySamplesNum * 2 - projectsToDevelop;
	if (totalExpertise >= rankMinMoleculeCosts[2])
		sampleRank = 2;
	if (totalExpertise >= (rankMinMoleculeCosts[3] + rankMaxMoleculeCosts[3]) / 2)
		sampleRank = 3;

	return sampleRank;
}

bool sLocalPlayer::updateAnalyzeSamples(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies, const sProjectsCollection& projects)
{
	auto mySamplesToAnalyze = collection.getSamplesCarriedBy(0).getSamplesToAnalyze();
	if (mySamplesToAnalyze.samples.empty())
	{
		setState(eState::chooseSamples);
		return false;
	}

	if (isInDiagnosis())
	{
		cmd::connectId(mySamplesToAnalyze.samples[0].sampleId, getMessage());
		return true;
	}

	return cmdGoTo(eArea::diagnosis, collection);
}

bool sLocalPlayer::updateChooseSamples(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies, const sProjectsCollection& projects)
{
	auto samplesForMe = collection.getSamplesNotCarriedBy(1).getDiagnosedSamples().getSamplesWithAvailableMoleculses(*this, supplies);
	if (samplesForMe.samples.size() > maxSamplesPerPlayer)
		samplesForMe.samples.resize(maxSamplesPerPlayer);

	auto mySamples = collection.getSamplesCarriedBy(0);

	if (shouldLog)
	{
		cerr << "Wanted:";
		for (const auto& sample : samplesForMe.samples)
		{
			cerr << " " << sample.sampleId;
		}
		cerr << " Carried: ";
		for (const auto& sample : mySamples.samples)
		{
			cerr << " " << sample.sampleId;
		}
		cerr << endl;
	}

	if (/*samplesForMe.samples != mySamples.samples && */getAreaFromString(target) == eArea::diagnosis)
	{
		// Check if there is wanted sample not owned by player.
		for (const auto& sample : samplesForMe.samples)
		{
			if (sample.carriedBy != 0)
			{
				if (mySamples.samples.size() < maxSamplesPerPlayer)
				{
					// Get the sample
					cmd::connectId(sample.sampleId, getMessage());
					return true;
				}
				else
				{
					for (int i = (int)mySamples.samples.size() - 1; i >= 0; --i)
					{
						if (find(samplesForMe.samples.begin(), samplesForMe.samples.end(), mySamples.samples[i]) == samplesForMe.samples.end())
						{
							cmd::connectId(mySamples.samples[i].sampleId, getMessage());
							return true;
						}
					}
				}
			}
		}

		// Check is there is owned sample but not owned.
		for (const auto& sample : mySamples.samples)
		{
			if (find(samplesForMe.samples.begin(), samplesForMe.samples.end(), sample) == samplesForMe.samples.end())
			{
				// Give back ownde sample.
				cmd::connectId(sample.sampleId, getMessage());
				return true;
			}
		}
	}

	if (mySamples.samples.empty())
		setState(eState::collectSamples);
	else
		setState(eState::gatherMolecules);
	return false;
}

bool sLocalPlayer::updateGatherMolecules(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies, const sProjectsCollection& projects)
{
	auto myDiagnosedSamples = collection.getSamplesCarriedBy(0).getDiagnosedSamples();

	int wantedType = -1;

	int usedStorage[(int)eMol::count] = {};
	int gainedExpertise[(int)eMol::count] = {};
	int gainedHealht = 0;


	int moleculeIdx[(int)eMol::count] = {};
	getMostWantedMoleculesIdx(enemy, collection, supplies, projects, moleculeIdx);


	for (const auto& sample : myDiagnosedSamples.samples)
	{
		bool missingSupplies = false;
		int totalMissingSupplies = 0;

		for (int mol = 0; mol < (int)eMol::count; ++mol)
		{
			int i = moleculeIdx[mol];

			const int moleculeToGather = max(sample.cost[i] - (getMoleculeNum(i) - usedStorage[i] + gainedExpertise[i]), 0);
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

		for (int mol = 0; mol < (int)eMol::count; ++mol)
		{
			int i = moleculeIdx[mol];

			const int moleculeToGather = max(sample.cost[i] - (getMoleculeNum(i) - usedStorage[i] + gainedExpertise[i]), 0);
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
				usedStorage[i] += max(sample.cost[i] - expertise[i] - gainedExpertise[i], 0);
			}

			int extertiseInMolecule = getMoleculeFromString(sample.expertiseGain);
			if (extertiseInMolecule >= 0 && extertiseInMolecule < (int)eMol::count)
				++gainedExpertise[extertiseInMolecule];

			continue;
		}

		break;
	}

	if (getStorageMoleculesNum() >= maxMoleculesPerPlayer)
	{
		setState(eState::produceMedicines);
		return false;
	}

	int totalSamplesRank = 0;
	for (const auto& sample : myDiagnosedSamples.samples)
		totalSamplesRank += sample.rank;

	// No molecule to gather. Choose random one.
	const int totalUsedMolecules = accumulate(begin(usedStorage), end(usedStorage), 0);
	const int wantedMolecules = min(max(totalUsedMolecules + min(max(maxMoleculesPerPlayer * 1 / 2, (getExpretiseMoleculesNum() - totalSamplesRank) * 1 / 2), maxMoleculesPerPlayer * 4 / 5), maxMoleculesPerPlayer * 3 / 4), maxMoleculesPerPlayer);

	if (wantedType == -1 && getStorageMoleculesNum() < wantedMolecules)
	{
		int startMolecule = rand() % (int)eMol::count;
		for (int mol = 0; mol < (int)eMol::count; ++mol)
		{
			int i = moleculeIdx[mol];

			int molecule = (startMolecule + i) % (int)eMol::count;
			if (supplies.available[molecule] > 0)
			{
				wantedType = molecule;
				break;
			}
		}
	}

	if (wantedType == -1)
	{
		setState(eState::produceMedicines);
		return false;
	}

	if (isInMolecules())
	{
		cmd::connectType(wantedType, getMessage());
		return true;
	}

	return cmdGoTo(eArea::molecules, collection);
}

bool sLocalPlayer::updateReturnSamples(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies, const sProjectsCollection& projects)
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

	return cmdGoTo(eArea::diagnosis, collection);
}

bool sLocalPlayer::updateProduceMedicines(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies, const sProjectsCollection& projects)
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

	return cmdGoTo(eArea::laboratory, collection);
}

bool sLocalPlayer::updateRandomMove(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies, const sProjectsCollection& projects)
{
	const int numValidAreas = (int)eArea::count - (int)eArea::start - 1;
	const eArea randomArea = (eArea)(rand() % numValidAreas + (int)eArea::start + 1);

	return cmdGoTo(randomArea, collection);
}

int sLocalPlayer::getRoundsNeededToProduceMedicines(const sSamplesCollection& collection, bool withMove)
{
	auto myDiagnosedSamples = collection.getSamplesCarriedBy(0).getDiagnosedSamples();

	int gainedExpertise[(int)eMol::count] = {};
	int readyProjects = 0;

	for (const auto& sample : myDiagnosedSamples.samples)
	{
		bool missingSupplies = false;
		for (int i = 0; i < (int)eMol::count; ++i)
		{
			const int moleculeToGather = max(sample.cost[i] - (getMoleculeNum(i) + gainedExpertise[i]), 0);
			if (moleculeToGather > 0)
			{
				missingSupplies = true;
				break;
			}
		}

		if (missingSupplies)
			continue;

		++readyProjects;

		int extertiseInMolecule = getMoleculeFromString(sample.expertiseGain);
		if (extertiseInMolecule >= 0 && extertiseInMolecule < (int)eMol::count)
			++gainedExpertise[extertiseInMolecule];
	}

	if (readyProjects == 0)
		return 0;

	int roundsToReturnProjects = readyProjects;
	if (withMove)
		roundsToReturnProjects += getAreaMoveCost(getAreaFromString(target), eArea::laboratory);

	return roundsToReturnProjects;
}

void sLocalPlayer::getMostWantedMoleculesIdx(const sPlayer& enemy, const sSamplesCollection& collection, const sSupplies& supplies, const sProjectsCollection& projects, int(&moleculeIdx)[(int)eMol::count]) const
{
	for (int i = 0; i < (int)eMol::count; ++i)
		moleculeIdx[i] = i;

	struct sMoleculeWithWeight
	{
		int molecule;
		int weight;
	} moleculesWithWeight[(int)eMol::count];

	for (int i = 0; i < (int)eMol::count; ++i)
	{
		moleculesWithWeight[i].molecule = moleculeIdx[i];
		moleculesWithWeight[i].weight = 0;
	}


	auto myDiagnosedSamples = collection.getSamplesCarriedBy(0).getDiagnosedSamples();
	auto enemyDiagnosedsamples = collection.getSamplesCarriedBy(0).getDiagnosedSamples();
	for (int i = 0; i < (int)eMol::count; ++i)
	{
		for (const auto& sample : myDiagnosedSamples.samples)
			moleculesWithWeight[i].weight += this->getMostWantedMoleculesWeight(sample, supplies, moleculesWithWeight[i].molecule);

		for (const auto& sample : enemyDiagnosedsamples.samples)
			moleculesWithWeight[i].weight += 100 * enemy.getMostWantedMoleculesWeight(sample, supplies, moleculesWithWeight[i].molecule);
	}


	sort(begin(moleculesWithWeight), end(moleculesWithWeight), [](const sMoleculeWithWeight& Left, const sMoleculeWithWeight& Right)
	{
		return Left.weight > Right.weight;
	});


	for (int i = 0; i < (int)eMol::count; ++i)
		moleculeIdx[i] = moleculesWithWeight[i].molecule;

	if (shouldLog)
	{
		for (int i = 0; i < (int)eMol::count; ++i)
		{
			if (i != 0)
				cerr << " ";
			cerr << moleculeIdx[i];
		}
		cerr << endl;
	}
}

int sPlayer::getMostWantedMoleculesWeight(const sSample& sample, const sSupplies& supplies, int molecule) const
{
	const int cost = sample.cost[molecule];
	const int have = this->getMoleculeNum(molecule);
	const int supp = supplies.available[molecule];
	const int need = cost - have;

	if (need <= 0)
		return 0;

	if (need <= supp)
		return need;

	return (maxMoleculePerType + 1 - need) * 10;
}
