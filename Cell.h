#include <iostream>
#include <bitset>
#include <string>
#include <algorithm>
#include <map>

#define FOR(x, n) for(int x = 0, __n = (n); x < __n; x++)
#define FORI(x, a, n) for(int x = (a), __n = (n); x < __n; x++)
#define FORR(x, n) for(int x = (n)-1; x >= 0; x--)
#define R(a) (a).begin(), (a).end()


typedef int id_int;
using namespace std;

template <int DIMS>
class CellDict;


template<int DIMS>
class Cell {
  typedef CellId<DIMS> Id;
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
    cout << "MAKE CELL " << id << endl;
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
  void print(ostream& cout) const {
    FOR(i, getLevel()) cout << ' ';
    cout << getId();
    if(isLeaf()) cout << " LEAF";
    cout << endl;
    FOR(i, subsCount()) {
      subs[i]->print(cout);
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
  
  CellId<DIMS> getCornerId(int cornerId, int atLevel) const {
    int delta = atLevel - getLevel();
    auto c = getId().offsetBinary(cornerId);
    return c.increaseLevelBy(delta);
  }
  
  void gatherIn(CellDict<DIMS>& map);
};

template <int DIMS>
ostream& operator<<(ostream& os, const Cell<DIMS>& cell) {
  os << '[';
  cell.getId().print(os);
  os << ']';
  return os;
}

