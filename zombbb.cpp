// identifier: 9504853406CBAC39EE89AA3AD238AA12CA198043

#include <getopt.h>
// #include <cstdio>
#include <deque>
#include <iostream>
#include <queue>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include "P2random.h"
using namespace std;
#define RollingQueueInitialSize 1000

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
    void print(uint32_t curRound, deque<Zombie>& zombieVec, SimulatorSettings& simSets);
};

inline Zombie getRandomZombie(uint32_t curRound);

void readNextRound(uint32_t& nextRound, bool& hasNextRound);

void addNewZombie(Zombie zombie,
                  deque<Zombie>& zombieVec,
                  priority_queue<Zombie*, vector<Zombie*>, Zombie::LessEtaFirst>& pqEta,
                  bool isVerbose);

void readNewZombies(uint32_t curRound,
                    deque<Zombie>& zombieVec,
                    priority_queue<Zombie*, vector<Zombie*>, Zombie::LessEtaFirst>& pqEta,
                    bool isVerbose);

Zombie* moveAllLivingZombies(deque<Zombie>& zombieVec, bool isVerbose);

Zombie* shootZombies(uint32_t curRound,
                     priority_queue<Zombie*, vector<Zombie*>, Zombie::LessEtaFirst>& pqEta,
                     StatisticsData& stat,
                     SimulatorSettings& simSets,
                     uint32_t& killedCnt);

int main(int argc, char** argv) {
    deque<Zombie> zombieVec;
    try {
        ios_base::sync_with_stdio(false);
        SimulatorSettings simSets(argc, argv);                                 // owner of all zombies
        priority_queue<Zombie*, vector<Zombie*>, Zombie::LessEtaFirst> pqEta;  // pq for zombie by eta

        StatisticsData stat;
        uint32_t curRound = 1;
        uint32_t nextRound = 0;
        bool hasNextRound = true;
        bool defeated = false;
        readNextRound(nextRound, hasNextRound);
        Zombie *theOneAndOnlyZombieKing = nullptr, *lastKilled = nullptr;
        uint32_t killedCnt = 0;
        while ((!defeated) && (killedCnt < zombieVec.size() || hasNextRound)) {
            if (simSets.isVerbose)
                // printf("Round: %d\n", curRound);
                cout << "Round: " << curRound << "\n";
            // printLivingZombies(livingZombies);
            /*    move zombies forward    */
            theOneAndOnlyZombieKing = moveAllLivingZombies(zombieVec, simSets.isVerbose);
            if (theOneAndOnlyZombieKing != nullptr) {
                /*    brain got eaten, GG    */
                defeated = true;
                break;
            }
            /*    generate new zombies from input    */
            if (curRound == nextRound) {
                readNewZombies(curRound, zombieVec, pqEta, simSets.isVerbose);
                /*    read next round    */
                readNextRound(nextRound, hasNextRound);
            }
            /*    shoot    */
            Zombie* lastKilledInRound = shootZombies(curRound, pqEta, stat, simSets, killedCnt);
            if (lastKilledInRound != nullptr)
                lastKilled = lastKilledInRound;
            if (simSets.isMedian && killedCnt > 0) {
                uint32_t median = (killedCnt & 1) == 1 ? stat.pqMoreRoundMedian.top()->round : (stat.pqMoreRoundMedian.top()->round + stat.pqLessRoundMedian.top()->round) >> 1;
                // printf("At the end of round %d, the median zombie lifetime is %d\n", curRound, median);
                cout << "At the end of round " << curRound << ", the median zombie lifetime is " << median << "\n";
            }
            curRound++;
        }
        if (defeated) {
            cout << "DEFEAT IN ROUND " << curRound << "! " << theOneAndOnlyZombieKing->name << " ate your brains!\n";
        } else {
            cout << "VICTORY IN ROUND " << curRound - 1 << "! " << lastKilled->name << " was the last zombie.\n";
        }
        if (simSets.isStatistics)
            stat.print(curRound, zombieVec, simSets);
        // printLivingZombies(livingZombies);
        // main ends here
    } catch (const Terminate& err) {
        return 0;
    } catch (std::runtime_error& e) {
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

inline Zombie getRandomZombie(uint32_t curRound) {
    string name = P2random::getNextZombieName();
    uint32_t dis = P2random::getNextZombieDistance();
    uint32_t v = P2random::getNextZombieSpeed();
    uint32_t hp = P2random::getNextZombieHealth();
    return Zombie(curRound, name, dis, v, hp);
}

void Zombie::print() {
    // printf("%s (distance: %d, speed: %d, health: %d)\n", name.c_str(), dis, v, hp);
    cout << name
         << " (distance: " << dis
         << ", speed: " << v
         << ", health: " << hp << ")\n";
}

bool Zombie::moveAndTryAttack(bool isVerbose) {
    dis -= min(dis, v);
    if (isVerbose) {
        // printf("Moved?: ");
        cout << "Moved: ";
        print();
    }
    return dis == 0;
}

bool Zombie::takeDamageAndIsDead(uint32_t& quiverCap) {
    uint32_t damage = min(hp, quiverCap);
    hp -= damage;
    quiverCap -= damage;
    return hp == 0;
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

void addNewZombie(Zombie zombie,
                  deque<Zombie>& zombieVec,
                  priority_queue<Zombie*, vector<Zombie*>, Zombie::LessEtaFirst>& pqEta,
                  bool isVerbose) {
    zombieVec.push_back(zombie);
    pqEta.push(&(zombieVec.back()));
    if (isVerbose) {
        // printf("Created: ");
        cout << "Created: ";
        zombie.print();
    }
}

void readNewZombies(uint32_t curRound,
                    deque<Zombie>& zombieVec,
                    priority_queue<Zombie*, vector<Zombie*>, Zombie::LessEtaFirst>& pqEta,
                    bool isVerbose) {
    string s;
    uint32_t randNum = 0;
    uint32_t nameNum = 0;
    cin >> s >> randNum >> s >> nameNum;
    for (uint32_t i = 0; i < randNum; i++) {
        zombieVec.push_back(getRandomZombie(curRound));
        pqEta.push(&(zombieVec.back()));
        if (isVerbose) {
            // printf("Created: ");
            cout << "Created: ";
            zombieVec.back().print();
        }
    }
    // addNewZombie(getRandomZombie(curRound), zombieVec, pqEta, isVerbose);
    for (uint32_t i = 0; i < nameNum; i++) {
        string name;
        uint32_t dis;
        uint32_t v;
        uint32_t hp;
        cin >> name >> s >> dis >> s >> v >> s >> hp;
        zombieVec.push_back(Zombie(curRound, name, dis, v, hp));
        pqEta.push(&(zombieVec.back()));
        if (isVerbose) {
            // printf("Created: ");
            cout << "Created: ";
            zombieVec.back().print();
        }
        // addNewZombie(Zombie(curRound, name, dis, v, hp), zombieVec, pqEta, isVerbose);
    }
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

Zombie* moveAllLivingZombies(deque<Zombie>& zombieVec, bool isVerbose) {
    Zombie* theOneAndOnlyZombie = nullptr;
    for (Zombie& zombie : zombieVec) {
        if (zombie.hp == 0)
            continue;
        if (zombie.moveAndTryAttack(isVerbose)) {
            if (theOneAndOnlyZombie == nullptr)
                theOneAndOnlyZombie = &zombie;
        }
    }
    return theOneAndOnlyZombie;
}

void Zombie::isDead(uint32_t curRound, StatisticsData& stat, SimulatorSettings& simSets) {
    round = curRound - round + 1;
    if (simSets.isVerbose) {
        // printf("Destroyed: ");
        cout << "Destroyed: ";
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

// void StatisticsData::print(uint32_t curRound, RollingQueue& livingZombies, SimulatorSettings& simSets) {
//     livingZombies.rewind();
//     uint32_t livingZombieCnt = 0;
//     for (Zombie* zombie = livingZombies.getNext(); zombie != nullptr; zombie = livingZombies.getNext()) {
//         livingZombieCnt++;
//         zombie->round = curRound - zombie->round + 1;
//         addZombie(zombie, simSets);
//     }
//     cout << "Zombies still active: " << livingZombieCnt << "\n";
//     cout << "First zombies killed:\n";
//     for (uint32_t i = 0; i < min(simSets.statN, uint32_t(firstKilled.size())); i++)
//         // printf("%s %d\n", firstKilled[i]->name.c_str(), i + 1);
//         cout << firstKilled[i]->name << " " << i + 1 << "\n";
//     cout << "Last zombies killed:\n";
//     uint32_t ii = lastKilledTail == 0 ? uint32_t(lastKilled.size()) - 1 : lastKilledTail - 1;
//     uint32_t jj = lastKilledTail > lastKilledHead ? lastKilledTail - lastKilledHead : lastKilledTail + uint32_t(lastKilled.size()) - lastKilledHead;
//     for (; jj > 0; jj--) {
//         // printf("%s %d\n", lastKilled[ii]->name.c_str(), jj);
//         cout << lastKilled[ii]->name << " " << jj << "\n";
//         ii = ii == 0 ? uint32_t(lastKilled.size()) - 1 : ii - 1;
//     }
//     vector<Zombie*> ans;
//     ans.reserve(max(pqMoreRoundStat.size(), pqLessRoundStat.size()));
//     cout << "Most active zombies:\n";
//     while (!pqMoreRoundStat.empty()) {
//         ans.push_back(pqMoreRoundStat.top());
//         pqMoreRoundStat.pop();
//     }
//     while (!ans.empty()) {
//         // printf("%s %d\n", ans.back()->name.c_str(), ans.back()->round);
//         cout << ans.back()->name << " " << ans.back()->round << "\n";
//         ans.pop_back();
//     }
//     cout << "Least active zombies:\n";
//     while (!pqLessRoundStat.empty()) {
//         ans.push_back(pqLessRoundStat.top());
//         pqLessRoundStat.pop();
//     }
//     while (!ans.empty()) {
//         // printf("%s %d\n", ans.back()->name.c_str(), ans.back()->round);
//         cout << ans.back()->name << " " << ans.back()->round << "\n";
//         ans.pop_back();
//     }
// }

void StatisticsData::print(uint32_t curRound, deque<Zombie>& zombieVec, SimulatorSettings& simSets) {
    uint32_t livingZombieCnt = 0;

    for (Zombie& zombie : zombieVec) {
        if (zombie.hp == 0)
            continue;
        livingZombieCnt++;
        zombie.round = curRound - zombie.round + 1;
        addZombie(&zombie, simSets);
    }
    stringstream output;
    cout << "Zombies still active: " << livingZombieCnt << "\n";
    cout << "First zombies killed:\n";

    uint32_t statN = simSets.statN;
    uint32_t firstKilledSz = uint32_t(firstKilled.size());
    for (uint32_t i = 0; i < min(statN, firstKilledSz); i++)
        cout << firstKilled[i]->name << " " << i + 1 << "\n";

    cout << "Last zombies killed:\n";
    uint32_t lastKilledSz = uint32_t(lastKilled.size());
    uint32_t ii = lastKilledTail == 0 ? lastKilledSz - 1 : lastKilledTail - 1;
    uint32_t jj = lastKilledTail > lastKilledHead ? lastKilledTail - lastKilledHead : lastKilledTail + lastKilledSz - lastKilledHead;
    for (; jj > 0; jj--) {
        cout << lastKilled[ii]->name << " " << jj << "\n";
        ii = (ii == 0 ? lastKilledSz : ii) - 1;
    }

    vector<Zombie*> ans;
    ans.reserve(pqMoreRoundStat.size());
    cout << "Most active zombies:\n";
    while (!pqMoreRoundStat.empty()) {
        ans.push_back(pqMoreRoundStat.top());
        pqMoreRoundStat.pop();
    }
    while (!ans.empty()) {
        // printf("%s %d\n", ans.back()->name.c_str(), ans.back()->round);
        cout << ans.back()->name << " " << ans.back()->round << "\n";
        ans.pop_back();
    }
    cout << "Least active zombies:\n";
    while (!pqLessRoundStat.empty()) {
        ans.push_back(pqLessRoundStat.top());
        pqLessRoundStat.pop();
    }
    while (!ans.empty()) {
        // printf("%s %d\n", ans.back()->name.c_str(), ans.back()->round);
        cout << ans.back()->name << " " << ans.back()->round << "\n";
        ans.pop_back();
    }
}
