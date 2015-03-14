#include <iostream>
#include <bitset>
#include <string>
#include <algorithm>
#include <map>
#include <vector>
#include <set>
#include <assert.h>

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
  
  Id getCornerId(int cornerId, int atLevel) const {
    int delta = atLevel - getLevel();
    auto c = getId().offsetBinary(cornerId);
    return c.increaseLevelBy(delta);
  }
  
  Id getZeroCornerAt(int atLevel) const {
    return getCornerId(0, atLevel);
  }
  Id getLastCornerAt(int atLevel) const {
    return getCornerId((1<<DIMS)-1, atLevel);
  }
  
  void gatherIn(CellDict<DIMS>& map);
  
  
  bool isHyperplaneOffBounds(const Id& c) const {
    Id c1 = getZeroCornerAt(c.getLevel());
    Id c2 = getLastCornerAt(c.getLevel());
    int zeros = 0;  
    FOR(i, DIMS) {
      if(c[i] == 0) {
        zeros++;
      } else {
        if(c[i]>c1[i] && c[i]<c2[i]) return false;
      }
    }
    assert(zeros == DIMS-1);
    return true;
  }
  bool isValidHyperplane(const Id& c) const{
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
  
  void gatherHyperplanes(set<Id>& V, int lvl) const {
    cout << "REC " << *this << " " << lvl << endl;
    if(isLeaf() || lvl == this->getLevel()) {
      // Id c1 = getZeroCornerAt(lvl);
      // Id c2 = getLastCornerAt(lvl);
      // FOR(i, DIMS) {
      //   cout << "INS " << c1.getHyperplane(i) << " " << c2.getHyperplane(i) << endl;
      //   V.insert(c1.getHyperplane(i));
      //   V.insert(c2.getHyperplane(i));
      // }
    } else {
      set<Id> V1;
      Id c = getId().increaseLevelBy(1).offsetBinary((1<<DIMS)-1);
      FOR(i, DIMS) {
        V1.insert(c.getHyperplane(i).withLevel(lvl));
      }
      FOR(i, subsCount()) {
        subs[i]->gatherHyperplanes(V1, lvl);
      }
      for(auto el : V1) {
        cout << "CHECK " << *this << ": " << el << " = " << isValidHyperplane(el) << endl;
        if(isValidHyperplane(el)) {
          V.insert(el);
        }
      }
    }
  }
  
};

template <int DIMS>
ostream& operator<<(ostream& os, const Cell<DIMS>& cell) {
  os << '[';
  cell.getId().print(os);
  os << ']';
  return os;
}

