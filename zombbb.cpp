// identifier: 9504853406CBAC39EE89AA3AD238AA12CA198043

#include <getopt.h>
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

class Zombie {
   public:
    string name;
    uint32_t dis;
    uint32_t v;
    uint32_t hp;
    uint32_t eta;
    uint32_t round;
    Zombie(uint32_t round = 0, string name = "", uint32_t dis = 0, uint32_t v = 1, uint32_t hp = 0)
        : name(name), dis(dis), v(v), hp(hp), eta(round + (dis % v == 0 ? dis / v + 1 : dis / v)), round(round){};
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
    bool moveAndTryAttack(const SimulatorSettings& simSets);
    uint32_t takeDamage(uint32_t quiverCap, uint32_t curRound);
    struct LessEtaFirst {
        bool operator()(const Zombie* const a, const Zombie* const b) {
            return a->eta > b->eta ||
                   (a->eta == b->eta && a->hp > b->hp) ||
                   (a->eta == b->eta && a->hp == b->hp && a->name > b->name);
        }
    };
    struct LessRoundFirst {
        bool operator()(const Zombie* const a, const Zombie* const b) {
            return a->round > b->round ||
                   (a->round == b->round && a->hp > b->hp) ||
                   (a->round == b->round && a->hp == b->hp && a->name > b->name);
        }
    };
    struct MoreRoundFirst {
        bool operator()(const Zombie* const a, const Zombie* const b) {
            return a->eta > b->eta ||
                   (a->eta == b->eta && a->hp > b->hp) ||
                   (a->eta == b->eta && a->hp == b->hp && a->name > b->name);
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
    priority_queue<Zombie*, vector<Zombie*>, Zombie::LessRoundFirst> pqLessRound;
    priority_queue<Zombie*, vector<Zombie*>, Zombie::MoreRoundFirst> pqMoreRound;
    StatisticsData()
        : lastKilledHead(0), lastKilledTail(0) {}
};

Zombie* getRandomZombie();

void deleteZombieInstances(vector<Zombie*>& vec);

int main(int argc, char** argv) {
    vector<Zombie*> zombieVec;
    try {
        ios_base::sync_with_stdio(false);
        SimulatorSettings simSets(argc, argv);                                 // owner of all zombies
        priority_queue<Zombie*, vector<Zombie*>, Zombie::LessEtaFirst> pqEta;  // pq for zombie by eta
        RollingQueue livingZombies;                                            // rolling queue for moving zombies forward
        StatisticsData stat;
        // TODO: read in file, iterate rounds, move zombies alive and try attack, check brain, generate new zombies, shoot zombies
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

Zombie* getRandomZombie() {
    string name = P2random::getNextZombieName();
    uint32_t dis = P2random::getNextZombieDistance();
    uint32_t v = P2random::getNextZombieSpeed();
    uint32_t hp = P2random::getNextZombieHealth();
    Zombie* res = new Zombie(0, name, dis, v, hp);
    return res;
}

void Zombie::print() {
    cout << name << " " << dis << " " << v << " " << eta << " " << hp << "\n";
}

bool Zombie::moveAndTryAttack(const SimulatorSettings& simSets) {
    dis -= min(dis, v);
    if (simSets.isVerbose)
        cout << "Moved: " << name << " (distance: " << dis << ", speed: " << v << ", health: " << hp << ")\n";
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

uint32_t Zombie::takeDamage(uint32_t quiverCap, uint32_t curRound) {
    uint32_t damage = min(hp, quiverCap);
    hp -= damage;
    if (hp == 0) {
        round = curRound - round;
        // TODO: handle death
    }
    return damage;
}

void deleteZombieInstances(vector<Zombie*>& vec) {
    for (Zombie*& item : vec)
        delete item;
}
