#include <iostream>
#include <bitset>
#include <string>
#include <algorithm>
#include <map>
#define FOR(x, n) for(int x = 0, __n = (n); x < __n; x++)
#define FORI(x, a, n) for(int x = (a), __n = (n); x < __n; x++)
#define FORR(x, n) for(int x = (n)-1; x >= 0; x--)
#define R(a) (a).begin(), (a).end()

using namespace std;
#include "CellId.h"

template <int DIMS>
class CellDict;


template<int DIMS>
class Cell {
  typedef CellId<DIMS> Id;
  Id id;
  int lvl;
  Cell<DIMS>* subs[1<<DIMS];
public:
  
  void init() {
    FOR(i, 1<<DIMS) {
      subs[i] = NULL;
    }
  }
  Cell() : lvl(1) {
    init();
  }
  
  Cell(int cell_lvl, Id cell_id) : lvl(cell_lvl), id(cell_id) {
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
  
  
  int subsCount() const {
    return isLeaf()?0:(1<<DIMS);
  }
  
  bool isLeaf() const {
    return subs[0] == NULL;
  }
  void printId(ostream& cout) const {
    FOR(i, DIMS) {
      if(i > 0) cout << ":";
      cout << printBinary(id[i], lvl);
    }
  }
  void print(ostream& cout) const {
    FOR(i, lvl) cout << ' ';
    printId(cout);
    if(isLeaf()) cout << " LEAF";
    cout << endl;
    FOR(i, subsCount()) {
      subs[i]->print(cout);
    }
  }
  
  void split() {
    Id chid;
    FOR(j, 1<<DIMS) {
      FOR(i, DIMS) {
        int offset = (j&(1<<i))!=0?1:0;
        chid[i] = 2 * id[i] + offset;
      }
      subs[j] = new Cell<DIMS>(lvl + 1, chid);
    }
    
  }
  
  void gather(CellDict<DIMS>& map);
  
};

template <int DIMS>
class CellDict: protected map<CellId<DIMS>, Cell<DIMS>* > {
  using sourceMap = map<CellId<DIMS>, Cell<DIMS>* >;
public:
  typename sourceMap::const_iterator begin() const {return sourceMap::begin();}
  typename sourceMap::const_iterator end() const {return sourceMap::end();}
  void addCell(Cell<DIMS> * cellPtr) {
    this->insert(make_pair(cellPtr->getId(), cellPtr));
  };

};


template <int DIMS>
ostream& operator<<(ostream& os, const Cell<DIMS>& cell) {
  os << '[';
  cell.getId().print(os);
  os << ']';
  return os;
}


template <int DIMS>
ostream& operator<<(ostream& os, const CellDict<DIMS>& m)
{
  for(auto el : m) {
    cout << *el.second;
    cout << ' ';
  }
  return os;
}


template <int DIMS>
void Cell<DIMS>::gather(CellDict<DIMS> & dict) {
  dict.addCell(this);
  if(!isLeaf()) {
    FOR(i, subsCount()) {
      subs[i]->gather(dict);
    }
  }
}

int main(int argc, char** argv) {
  
  Cell<3> c;
  c.print(cout);
  cout << "####" << endl;
  c.split();
  c.print(cout);
  CellDict<3> D;

  c.gather(D);
  D.addCell(&c);
  cout << "####" << endl;
  cout << D;
  cout << endl;
  return 0;
}