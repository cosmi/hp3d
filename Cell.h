#include <iostream>
#include <bitset>
#include <string>
#include <algorithm>
#include <map>
#include <vector>
#include <set>

#define FOR(x, n) for(int x = 0, __n = (n); x < __n; x++)
#define FORI(x, a, n) for(int x = (a), __n = (n); x < __n; x++)
#define FORR(x, n) for(int x = (n)-1; x >= 0; x--)
#define R(a) (a).begin(), (a).end()


typedef int id_int;
using namespace std;

template <int DIMS>
class CellDict;
template <int DIMS>
class Supernode;

template<int DIMS>
class Cell {
  using Id = CellId<DIMS>;
  using Dict = CellDict<DIMS>;
  using Plane = Hyperplane<DIMS>;
  using Bounds = CellBounds<DIMS>;
  Id id;
  Cell<DIMS>* subs[1<<DIMS];
  
public:
  
  void init() {
    FOR(i, 1<<DIMS) {
      subs[i] = NULL;
    }
  }
  Cell() {
    init();
  }
  
  Cell(Id cell_id) : id(cell_id) {
    init();
  }
  
  ~Cell() {
    if(!isLeaf()) {
      FOR(i, 1 << DIMS) {
        delete subs[i];
      }
    }
  }
  
  const Id& getId() const {
    return id;
  }
  
  const size_t getLevel() const {
    return id.getLevel();
  }
  
  
  int subsCount() const {
    return isLeaf()?0:(1<<DIMS);
  }
  
  
  
  bool isLeaf() const {
    return subs[0] == NULL;
  }
  void print(ostream& cerr) const {
    FOR(i, getLevel()) cerr << ' ';
    cerr << getId();
    if(isLeaf()) cerr << " LEAF";
    cerr << endl;
    FOR(i, subsCount()) {
      subs[i]->print(cerr);
    }
  }
  
  void split() {
    FOR(j, 1<<DIMS) {
      subs[j] = new Cell<DIMS>(getId().getChildId(j));
    }
    
  }
  
  size_t countLeaves() const {
    if(isLeaf()) return 1;
    size_t sum = 0;
    FOR(i, subsCount()) {
      sum += subs[i]->countLeaves();
    }
    return sum;
  }
  
  size_t countCells() const {
    if(isLeaf()) return 1;
    size_t sum = 1;
    FOR(i, subsCount()) {
      sum += subs[i]->countCells();
    }
    return sum;
  }
  
  int getMaxLevel() const {
    int m = getLevel();
    FOR(i, subsCount()) {
      int n = subs[i]->getMaxLevel();
      if(n > m) m = n;
    }
    return m;
  }
  Id getCornerId(int cornerId) const {
    return getId().offsetBinary(cornerId);
  }
  Id getCornerId(int cornerId, int atLevel) const {
    int delta = atLevel - getLevel();
    auto c = getId().offsetBinary(cornerId);
    return c.increaseLevelBy(delta);
  }
  
  Id getZeroCornerAt(int atLevel) const {
    return getCornerId(0, atLevel);
  }
  
  int countCorners() const {
    return (1<<DIMS);
  }
  Id getLastCornerAt(int atLevel) const {
    return getCornerId(countCorners()-1, atLevel);
  }
  
  void gatherIn(CellDict<DIMS>& map);
  
  void shallowGatherIn(CellDict<DIMS>& map);
  
  bool isHyperplaneOffBounds(const Plane& c) const {
    Id c1 = getZeroCornerAt(c.getLevel());
    Id c2 = getLastCornerAt(c.getLevel());
    int dim = c.getDim();
    auto val = c.getValue();
    return !(c1[dim] < val && val < c2[dim]);
  }
  bool isValidHyperplane(const Plane& c) const{
    if(isHyperplaneOffBounds(c)) {
      return true;
    }
    if(isLeaf()) {
      return false;
    } else {
      FOR(i, subsCount()) {
        if(!subs[i]->isValidHyperplane(c)) {
          return false;
        }
      }
      return true;
    }
  }
  
  void gatherHyperplanes(set<Plane>& V, int lvl, map<Id, set<Plane> >& cache) const {
    if(isLeaf() || lvl == this->getLevel()) {
    } else {
      set<Plane> & hyperplanes = cache[getId()];
      if(hyperplanes.empty()) {
        set<Plane> V1;
        Id c = getId().increaseLevelBy(1).offsetBinary((1<<DIMS)-1);
        FOR(i, DIMS) {
          V1.insert(c.getHyperplane(i).withLevel(lvl));
        }
        FOR(i, subsCount()) {
          subs[i]->gatherHyperplanes(V1, lvl, cache);
        }
        for(auto el : V1) {
          if(isValidHyperplane(el)) {
            hyperplanes.insert(el);
          }
        }
      }
      for(auto el: hyperplanes) {
        V.insert(el);
      }
    }
    
  }
  bool overHyperplane(Plane hyperplane) const {
    int dim = hyperplane.getDim();
    Id c = getId().withLevel(hyperplane.getLevel());
    return c[dim] >= hyperplane.getValue();
  }
  bool crossesHyperplane(Plane hyperplane) const {
    int targetLvl = hyperplane.getLevel();
    Id c1 = getZeroCornerAt(targetLvl);
    Id c2 = getLastCornerAt(targetLvl);
    int dim = hyperplane.getDim();
    auto val = hyperplane.getValue();
    return c1[dim] < val && val < c2[dim];
  }
  
  void divideByHyperplane(Plane hyperplane, Dict& a, Dict& b);
  
  Bounds getBounds(int targetLvl = -1) const {
    if(targetLvl == -1) targetLvl = getLevel();
    return Bounds(getZeroCornerAt(targetLvl), getLastCornerAt(targetLvl));
  }
  Cell& getChild(int childId) {
    return *(subs[childId]);
  }
};

template <int DIMS>
ostream& operator<<(ostream& os, const Cell<DIMS>& cell) {
  os << '[';
  cell.getId().print(os);
  if(cell.isLeaf()) {
    os << "+LEAF";
  }
  os << ']';
  return os;
}

