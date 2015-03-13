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
#include "Cell.h"

template <int DIMS>
class CellDict: protected map<CellId<DIMS>, Cell<DIMS>* > {
  using sourceMap = map<CellId<DIMS>, Cell<DIMS>* >;
public:
  typename sourceMap::const_iterator begin() const {return sourceMap::begin();}
  typename sourceMap::const_iterator end() const {return sourceMap::end();}
  void addCell(Cell<DIMS> * cellPtr) {
    this->insert(make_pair(cellPtr->getId(), cellPtr));
  };
  
  Cell<DIMS>* getCell(const CellId<DIMS>& id) const {
    auto it = this->find(id);
    if(it == this->end()) return NULL;
    return it->second;
  }

};

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
void Cell<DIMS>::gatherIn(CellDict<DIMS> & dict) {
  dict.addCell(this);
  if(!this->isLeaf()) {
    FOR(i, this->subsCount()) {
      subs[i]->gatherIn(dict);
    }
  }
}


template <int DIMS>
void ensureSplit(CellDict<DIMS> & dict, CellId<DIMS> id) {

  cout << "ENS " << id << endl;
  if(id.isRoot()) return;
  if(dict.getCell(id) != NULL) return;
  
  auto parent = id.getParentId();

  cout << "ENS " << id << endl;
  ensureSplit(dict, parent);

  cout << "ENS " << id << endl;
  
  Cell<DIMS> * c = dict.getCell(parent);
  
  c->split();
  c->gatherIn(dict);
  
  FOR(dim, DIMS) {
    auto a = id.getMovedId(dim, -1).getParentId();
    auto b = id.getMovedId(dim, +1).getParentId();
    cout << "nei: " << a << "'" << a.isValid() << " " << endl;
    cout << "nei: " << b << "'" << b.isValid() << " " << endl;
    if(a.isValid()) ensureSplit(dict, a);
    if(b.isValid()) ensureSplit(dict, b);
  }
}

template<int DIMS>
void printGrid(CellDict<DIMS> & dict) {
  const int MAXP = 130;
  char buff [MAXP+1][MAXP+1];
  FOR(i, MAXP+1) {
    FOR(j, MAXP+1) buff[i][j]=0;
  }
  int lvl = dict.getCell(CellId<DIMS>())->getMaxLevel();
  cout << lvl << endl;
  
  int offset = 1<<lvl;
  
  for (auto el : dict) {
    Cell<DIMS> * c = el.second;
    if(!c->isLeaf()) continue;
    
    auto c1 = c->getCornerId(0, lvl);
    auto c2 = c->getCornerId(3, lvl);
    
    cout << *c << " " << c1 << " " << c2 << endl;
    
  }
  
}


const int DIM = 2;
int main(int argc, char** argv) {

  Cell<DIM> c;
  cout << c.getId() << " <> " << c.getId().getChildId(0) << endl;
  c.print(cout);
  c.split();
  c.print(cout);

  CellDict<DIM> D;
  c.gatherIn(D);
  cout << D << endl;
  cout << "####" << endl;
  
  auto target = c.getId().getChildId(0).getChildId(1).getChildId(0);
  cout << c.getId().getChildId(0) << endl;
  cout << c.getId().getChildId(0).getChildId(1) << endl;
  cout << target << " -> " << target.getParentId() << endl;
  
  ensureSplit(D, target);
  cout << "####" << endl;
  cout << D;
  cout << endl;
  cout << "####" << endl;
  c.print(cout);
  cout << c.countLeaves() << ' ' << c.countCells() << ' ' << c.getMaxLevel() << endl;;
  printGrid(D);
  return 0;
}