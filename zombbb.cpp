// identifier: 9504853406CBAC39EE89AA3AD238AA12CA198043

#include <getopt.h>
#include <cstdio>
#include <iostream>
#include <queue>
#include <string>
#include <vector>
#include "P2random.h"
using namespace std;

class Terminate {
   public:
    string info;
    Terminate()
        : info("") {}
    Terminate(string s)
        : info(s) {}
};

class SimulatorSettings {
   public:
    bool isVerbose;
    bool isStatistics;
    uint32_t statN;
    bool isMedian;
    uint32_t qCap;
    uint32_t randSeed;
    uint32_t maxRandDis;
    uint32_t maxRandV;
    uint32_t maxRandHp;
    SimulatorSettings()
        : isVerbose(false), isStatistics(false), statN(0), isMedian(false), qCap(0), randSeed(0), maxRandDis(0), maxRandV(0), maxRandHp(0){};
    SimulatorSettings(int argc, char** argv)
        : isVerbose(false), isStatistics(false), statN(0), isMedian(false), qCap(0), randSeed(0), maxRandDis(0), maxRandV(0), maxRandHp(0) {
        getSettingsFromArgs(argc, argv);
        readSettings();
    }
    void print() {
        cout << "isVerbose: " << isVerbose << "\n"
             << "isStatistics: " << isStatistics << " " << statN << "\n"
             << "isMedian: " << isMedian << "\n";
    }

   private:
    void getSettingsFromArgs(int argc, char** argv);
    void readSettings();
};

class StatisticsData;

class Zombie {
   public:
    string name;
    uint32_t dis;
    uint32_t v;
    uint32_t hp;
    uint32_t eta;
    uint32_t round;
    Zombie(uint32_t round = 0, string name = "", uint32_t dis = 0, uint32_t v = 1, uint32_t hp = 0)
        : name(name), dis(dis), v(v), hp(hp), eta(round + dis / v), round(round){};
    Zombie(const Zombie& t)
        : name(t.name), dis(t.dis), v(t.v), hp(t.hp), eta(t.eta), round(t.round) {}
    Zombie& operator=(const Zombie& t) {
        Zombie temp(t);
        swap(name, temp.name);
        swap(dis, temp.dis);
        swap(v, temp.v);
        swap(hp, temp.hp);
        swap(eta, temp.eta);
        swap(round, temp.round);
        return *this;
    }
    void print();
    bool moveAndTryAttack(bool isVerbose);
    bool takeDamageAndIsDead(uint32_t& quiverCap);
    void isDead(uint32_t curRound, StatisticsData& stat, SimulatorSettings& simSets);
    void addToStat(StatisticsData& stat, SimulatorSettings& simSets);
    struct LessEtaFirst {
        bool operator()(const Zombie* const a, const Zombie* const b) {
            return a->eta > b->eta ||
                   (a->eta == b->eta && a->hp > b->hp) ||
                   (a->eta == b->eta && a->hp == b->hp && a->name > b->name);
        }
    };
    struct MoreRoundFirst {
        /**
         * @brief if zombie a is more likely to have less round, a has lower priority
         *
         * @param a
         * @param b
         * @return true
         * @return false
         */
        bool operator()(const Zombie* const a, const Zombie* const b) {
            return a->round < b->round || (a->round == b->round && a->name < b->name);
        }
    };
    struct LessRoundFirst {
        /**
         * @brief  if zombie a is more likely to have more round, a has lower priority
         *
         * @param a
         * @param b
         * @return true
         * @return false
         */
        bool operator()(const Zombie* const a, const Zombie* const b) {
            return a->round > b->round || (a->round == b->round && a->name < b->name);
        }
    };
};

class RollingQueue {
   public:
    int32_t cap;   // 0~cap-1
    int32_t size;  // head~tail-1  if size==cap-2, must double cap, ptr==tail->end
    int32_t head;
    int32_t tail;
    Zombie** data;
    int32_t RIndex;
    int32_t LIndex;
    RollingQueue()
        : cap(10), size(0), head(0), tail(0), data(new Zombie*[10]()), RIndex(0), LIndex(0) {}
    ~RollingQueue() {
        delete[] data;
    }
    void rewind() {
        RIndex = LIndex = head;
    }
    /**
     * @brief return nullptr if there's no living zombies
     *
     * @return Zombie*
     */
    Zombie* getNext();
    void push(Zombie* z);
};

class StatisticsData {
   public:
    vector<Zombie*> firstKilled;
    vector<Zombie*> lastKilled;
    uint32_t lastKilledHead;
    uint32_t lastKilledTail;
    priority_queue<Zombie*, vector<Zombie*>, Zombie::MoreRoundFirst> pqLessRoundStat;
    priority_queue<Zombie*, vector<Zombie*>, Zombie::LessRoundFirst> pqMoreRoundStat;
    priority_queue<Zombie*, vector<Zombie*>, Zombie::LessRoundFirst> pqLessRoundMedian;
    priority_queue<Zombie*, vector<Zombie*>, Zombie::MoreRoundFirst> pqMoreRoundMedian;
    StatisticsData()
        : lastKilledHead(1), lastKilledTail(0) {
        lastKilled.push_back(nullptr);
    }
    void addZombie(Zombie* zombie, SimulatorSettings& simSets);
    void print(uint32_t curRound, RollingQueue& livingZombies, SimulatorSettings& simSets);
};

Zombie* getRandomZombie(uint32_t curRound);

void deleteZombieInstances(vector<Zombie*>& vec);

void readNextRound(uint32_t& nextRound, bool& hasNextRound);

void addNewZombie(Zombie* zombie,
                  vector<Zombie*>& zombieVec,
                  RollingQueue& livingZombies,
                  priority_queue<Zombie*, vector<Zombie*>, Zombie::LessEtaFirst>& pqEta,
                  bool isVerbose);

void readNewZombies(uint32_t curRound,
                    vector<Zombie*>& zombieVec,
                    RollingQueue& livingZombies,
                    priority_queue<Zombie*, vector<Zombie*>, Zombie::LessEtaFirst>& pqEta,
                    bool isVerbose);

void printLivingZombies(RollingQueue& livingZombies);

Zombie* moveAllLivingZombies(RollingQueue& livingZombies, bool isVerbose);

Zombie* shootZombies(uint32_t curRound,
                     priority_queue<Zombie*, vector<Zombie*>, Zombie::LessEtaFirst>& pqEta,
                     StatisticsData& stat,
                     SimulatorSettings& simSets,
                     uint32_t& killedCnt);

int main(int argc, char** argv) {
    vector<Zombie*> zombieVec;
    try {
        // ios_base::sync_with_stdio(false);
        SimulatorSettings simSets(argc, argv);                                 // owner of all zombies
        priority_queue<Zombie*, vector<Zombie*>, Zombie::LessEtaFirst> pqEta;  // pq for zombie by eta
        RollingQueue livingZombies;                                            // rolling queue for moving zombies forward
        StatisticsData stat;
        // TODO: read in file, iterate rounds, move zombies alive and try attack, check brain, generate new zombies, shoot zombies
        uint32_t curRound = 1;
        uint32_t nextRound = 0;
        bool hasNextRound = true;
        bool defeated = false;
        readNextRound(nextRound, hasNextRound);
        Zombie *theOneAndOnlyZombieKing = nullptr, *lastKilled = nullptr;
        uint32_t killedCnt = 0;
        while ((!defeated) && (killedCnt < zombieVec.size() || hasNextRound)) {
            if (simSets.isVerbose)
                printf("Round: %d\n", curRound);
            // cout << "Round: " << curRound << "\n";
            // printLivingZombies(livingZombies);
            /*    move zombies forward    */
            theOneAndOnlyZombieKing = moveAllLivingZombies(livingZombies, simSets.isVerbose);
            if (theOneAndOnlyZombieKing != nullptr) {
                /*    brain got eaten, GG    */
                defeated = true;
                break;
            }
            /*    generate new zombies from input    */
            if (curRound == nextRound) {
                readNewZombies(curRound, zombieVec, livingZombies, pqEta, simSets.isVerbose);
                /*    read next round    */
                readNextRound(nextRound, hasNextRound);
            }
            /*    shoot    */
            Zombie* lastKilledInRound = shootZombies(curRound, pqEta, stat, simSets, killedCnt);
            if (lastKilledInRound != nullptr)
                lastKilled = lastKilledInRound;
            if (simSets.isMedian && killedCnt > 0) {
                uint32_t median = (killedCnt & 1) == 1 ? stat.pqMoreRoundMedian.top()->round : (stat.pqMoreRoundMedian.top()->round + stat.pqLessRoundMedian.top()->round) >> 1;
                printf("At the end of round %d, the median zombie lifetime is %d\n", curRound, median);
                // cout << "At the end of round " << curRound << ", the median zombie lifetime is " << median << "\n";
            }
            curRound++;
        }
        if (defeated) {
            cout << "DEFEAT IN ROUND " << curRound << "! " << theOneAndOnlyZombieKing->name << " ate your brains!\n";
        } else {
            cout << "VICTORY IN ROUND " << curRound - 1 << "! " << lastKilled->name << " was the last zombie.\n";
        }
        if (simSets.isStatistics)
            stat.print(curRound, livingZombies, simSets);
        // printLivingZombies(livingZombies);
        // main ends here
        deleteZombieInstances(zombieVec);
    } catch (const Terminate& err) {
        deleteZombieInstances(zombieVec);
        return 0;
    } catch (std::runtime_error& e) {
        deleteZombieInstances(zombieVec);
        cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}

/**
 * @brief get simulator settings from args
 *
 * @param argc
 * @param argv
 */
void SimulatorSettings::getSettingsFromArgs(int argc, char** argv) {
    int optionIdx = 0;
    int option = 0;
    opterr = 0;
    struct option longOpts[] = {{"help", no_argument, nullptr, 'h'},
                                {"verbose", no_argument, nullptr, 'v'},
                                {"statistics", required_argument, nullptr, 's'},
                                {"median", no_argument, nullptr, 'm'}};
    while ((option = getopt_long(argc, argv, "vs:mh", longOpts, &optionIdx)) != -1) {
        switch (option) {
            case 'h':
                cout << "I'm too lazy to write help. Check out https://eecs281staff.github.io/p2-the-walking-deadline/\n";
                throw(Terminate("help found in args"));
            case 's':
                isStatistics = true;
                statN = atoi(optarg);
                break;
            case 'v':
                isVerbose = true;
                break;
            case 'm':
                isMedian = true;
                break;
            default:
                break;
        }
    }
}

void SimulatorSettings::readSettings() {
    string s;
    getline(cin, s);
    cin >> s >> qCap >> s >> randSeed >> s >> maxRandDis >> s >> maxRandV >> s >> maxRandHp;
    P2random::initialize(randSeed, maxRandDis, maxRandV, maxRandHp);
}

Zombie* getRandomZombie(uint32_t curRound) {
    string name = P2random::getNextZombieName();
    uint32_t dis = P2random::getNextZombieDistance();
    uint32_t v = P2random::getNextZombieSpeed();
    uint32_t hp = P2random::getNextZombieHealth();
    Zombie* res = new Zombie(curRound, name, dis, v, hp);
    return res;
}

void Zombie::print() {
    printf("%s (distance: %d, speed: %d, health: %d)\n", name.c_str(), dis, v, hp);
    // cout << name
    //      << " (distance: " << dis
    //      << ", speed: " << v
    //      << ", health: " << hp << ")\n";
}

bool Zombie::moveAndTryAttack(bool isVerbose) {
    dis -= min(dis, v);
    if (isVerbose) {
        printf("Moved: ");
        // cout << "Moved: ";
        print();
    }
    return dis == 0;
}

Zombie* RollingQueue::getNext() {
    while (LIndex != tail && data[LIndex]->hp == 0) {
        LIndex++;
        if (LIndex == cap)
            LIndex = 0;
        size--;
    }
    if (LIndex == tail) {
        tail = RIndex;
        return nullptr;
    }
    data[RIndex] = data[LIndex];
    Zombie* res = data[RIndex];
    RIndex++;
    LIndex++;
    if (RIndex == cap)
        RIndex = 0;
    if (LIndex == cap)
        LIndex = 0;
    return res;
}

void RollingQueue::push(Zombie* z) {
    size++;
    if (size == cap - 2) {
        Zombie** newData = new Zombie*[2 * cap]();
        int32_t newTail = 0;
        for (int32_t i = head; i != tail; i = (i < cap - 1 ? i + 1 : 0), newTail++)
            newData[newTail] = data[i];
        swap(data, newData);
        delete[] newData;
        head = 0;
        tail = newTail;
        cap = cap << 1;
    }
    data[tail] = z;
    tail++;
}

bool Zombie::takeDamageAndIsDead(uint32_t& quiverCap) {
    uint32_t damage = min(hp, quiverCap);
    hp -= damage;
    quiverCap -= damage;
    return hp == 0;
}

void deleteZombieInstances(vector<Zombie*>& vec) {
    for (Zombie*& item : vec)
        delete item;
}

void readNextRound(uint32_t& nextRound, bool& hasNextRound) {
    string s;
    if (cin >> s) {
        hasNextRound = true;
        cin >> s >> nextRound;
    } else {
        hasNextRound = false;
    }
}

void addNewZombie(Zombie* zombie,
                  vector<Zombie*>& zombieVec,
                  RollingQueue& livingZombies,
                  priority_queue<Zombie*, vector<Zombie*>, Zombie::LessEtaFirst>& pqEta,
                  bool isVerbose) {
    zombieVec.push_back(zombie);
    livingZombies.push(zombie);
    pqEta.push(zombie);
    if (isVerbose) {
        printf("Created: ");
        // cout << "Created: ";
        zombie->print();
    }
}

void readNewZombies(uint32_t curRound,
                    vector<Zombie*>& zombieVec,
                    RollingQueue& livingZombies,
                    priority_queue<Zombie*, vector<Zombie*>, Zombie::LessEtaFirst>& pqEta,
                    bool isVerbose) {
    string s;
    uint32_t randNum = 0;
    uint32_t nameNum = 0;
    cin >> s >> randNum >> s >> nameNum;
    for (uint32_t i = 0; i < randNum; i++)
        addNewZombie(getRandomZombie(curRound), zombieVec, livingZombies, pqEta, isVerbose);
    for (uint32_t i = 0; i < nameNum; i++) {
        string name;
        uint32_t dis;
        uint32_t v;
        uint32_t hp;
        cin >> name >> s >> dis >> s >> v >> s >> hp;
        addNewZombie(new Zombie(curRound, name, dis, v, hp), zombieVec, livingZombies, pqEta, isVerbose);
    }
}

void printLivingZombies(RollingQueue& livingZombies) {
    cout << "----- living zombies -----\n";
    livingZombies.rewind();
    for (Zombie* item = livingZombies.getNext(); item != nullptr; item = livingZombies.getNext())
        item->print();
}

void StatisticsData::addZombie(Zombie* zombie, SimulatorSettings& simSets) {
    if (zombie->hp == 0) {
        if (firstKilled.size() < simSets.statN)
            firstKilled.push_back(zombie);
        if (lastKilled.size() < simSets.statN + 5) {
            lastKilled.push_back(zombie);
            if (lastKilled.size() - 1 > simSets.statN)
                lastKilledHead = uint32_t(lastKilled.size()) - simSets.statN;
        } else {
            lastKilled[lastKilledTail] = zombie;
            lastKilledTail++;
            if (lastKilledTail == lastKilled.size())
                lastKilledTail = 0;
            lastKilledHead++;
            if (lastKilledHead == lastKilled.size())
                lastKilledHead = 0;
        }
    }
    pqLessRoundStat.push(zombie);
    pqMoreRoundStat.push(zombie);
    if (pqLessRoundStat.size() > simSets.statN) {
        pqLessRoundStat.pop();
        pqMoreRoundStat.pop();
    }
}

void Zombie::isDead(uint32_t curRound, StatisticsData& stat, SimulatorSettings& simSets) {
    round = curRound - round + 1;
    if (simSets.isVerbose) {
        printf("Destroyed: ");
        // cout << "Destroyed: ";
        print();
    }
    if (simSets.isMedian) {
        if ((stat.pqMoreRoundMedian.empty()) || stat.pqMoreRoundMedian.top()->round >= round)
            stat.pqMoreRoundMedian.push(this);
        else
            stat.pqLessRoundMedian.push(this);

        // more.size >= less.size
        if (stat.pqMoreRoundMedian.size() > stat.pqLessRoundMedian.size() + 1) {
            stat.pqLessRoundMedian.push(stat.pqMoreRoundMedian.top());
            stat.pqMoreRoundMedian.pop();
        }
        if (stat.pqMoreRoundMedian.size() < stat.pqLessRoundMedian.size()) {
            stat.pqMoreRoundMedian.push(stat.pqLessRoundMedian.top());
            stat.pqLessRoundMedian.pop();
        }
    }
    if (simSets.isStatistics) {
        stat.addZombie(this, simSets);
    }
}

Zombie* moveAllLivingZombies(RollingQueue& livingZombies, bool isVerbose) {
    livingZombies.rewind();
    Zombie* theOneAndOnlyZombie = nullptr;
    for (Zombie* zombie = livingZombies.getNext(); zombie != nullptr; zombie = livingZombies.getNext()) {
        if (zombie->moveAndTryAttack(isVerbose)) {
            if (theOneAndOnlyZombie == nullptr)
                theOneAndOnlyZombie = zombie;
        }
    }
    return theOneAndOnlyZombie;
}

Zombie* shootZombies(uint32_t curRound,
                     priority_queue<Zombie*, vector<Zombie*>, Zombie::LessEtaFirst>& pqEta,
                     StatisticsData& stat,
                     SimulatorSettings& simSets,
                     uint32_t& killedCnt) {
    uint32_t qCap = simSets.qCap;
    Zombie* res = nullptr;
    while (qCap > 0 && (!pqEta.empty())) {
        Zombie* zombie = pqEta.top();
        if (zombie->takeDamageAndIsDead(qCap)) {
            killedCnt++;
            pqEta.pop();
            zombie->isDead(curRound, stat, simSets);
            res = zombie;
        }
    }
    return res;
}

void StatisticsData::print(uint32_t curRound, RollingQueue& livingZombies, SimulatorSettings& simSets) {
    livingZombies.rewind();
    uint32_t livingZombieCnt = 0;
    for (Zombie* zombie = livingZombies.getNext(); zombie != nullptr; zombie = livingZombies.getNext()) {
        livingZombieCnt++;
        zombie->round = curRound - zombie->round + 1;
        addZombie(zombie, simSets);
    }
    cout << "Zombies still active: " << livingZombieCnt << "\n";
    cout << "First zombies killed:\n";
    for (uint32_t i = 0; i < min(simSets.statN, uint32_t(firstKilled.size())); i++)
        printf("%s %d\n", firstKilled[i]->name.c_str(), i + 1);
    // cout << firstKilled[i]->name << " " << i + 1 << "\n";
    cout << "Last zombies killed:\n";
    uint32_t ii = lastKilledTail == 0 ? uint32_t(lastKilled.size()) - 1 : lastKilledTail - 1;
    uint32_t jj = lastKilledTail > lastKilledHead ? lastKilledTail - lastKilledHead : lastKilledTail + uint32_t(lastKilled.size()) - lastKilledHead;
    for (; jj > 0; jj--) {
        printf("%s %d\n", lastKilled[ii]->name.c_str(), jj);
        // cout << lastKilled[ii]->name << " " << jj << "\n";
        ii = ii == 0 ? uint32_t(lastKilled.size()) - 1 : ii - 1;
    }
    vector<Zombie*> ans;
    cout << "Most active zombies:\n";
    while (!pqMoreRoundStat.empty()) {
        ans.push_back(pqMoreRoundStat.top());
        pqMoreRoundStat.pop();
    }
    while (!ans.empty()) {
        printf("%s %d\n", ans.back()->name.c_str(), ans.back()->round);
        // cout << ans.back()->name << " " << ans.back()->round << "\n";
        ans.pop_back();
    }
    cout << "Least active zombies:\n";
    while (!pqLessRoundStat.empty()) {
        ans.push_back(pqLessRoundStat.top());
        pqLessRoundStat.pop();
    }
    while (!ans.empty()) {
        printf("%s %d\n", ans.back()->name.c_str(), ans.back()->round);
        // cout << ans.back()->name << " " << ans.back()->round << "\n";
        ans.pop_back();
    }
}
