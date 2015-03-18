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



template <int DIMS>
class CellDict: protected map<CellId<DIMS>, Cell<DIMS>* > {
  using Id = CellId<DIMS>;
  using sourceMap = map<Id, Cell<DIMS>* >;

  CellBounds<DIMS> bounds;
public:
  typename sourceMap::const_iterator begin() const {return sourceMap::begin();}
  typename sourceMap::const_iterator end() const {return sourceMap::end();}
  typename sourceMap::iterator begin() {return sourceMap::begin();}
  typename sourceMap::iterator end() {return sourceMap::end();}
  size_t size() const {return sourceMap::size();}

  void addCell(Cell<DIMS> * cellPtr) {
    bounds = bounds.join(cellPtr->getBounds());
    this->insert(make_pair(cellPtr->getId(), cellPtr));
    
  };
  
  Cell<DIMS>* getCell(const CellId<DIMS>& id) const {
    auto it = this->find(id);
    if(it == this->end()) return NULL;
    return it->second;
  }

  pair<CellDict, CellDict> divideByHyperplane(Hyperplane<DIMS> hyperplane) const {
    pair<CellDict, CellDict> res;
    for(auto el : *this) {
      el.second->divideByHyperplane(hyperplane, res.first, res.second);
    }
    return res;
  }
  
  void complete() {
    for(auto el : *this) {
      el.second->gatherIn(*this);
    }
  }
  const CellBounds<DIMS>& getBounds() const {
    return bounds;
  }
  
  void setBoundsLevel(int lvl) {
    bounds = bounds.withLevel(lvl);
  }
  
  bool isConstrained(const Supernode<DIMS>& node) const;
};

template <int DIMS>
ostream& operator<<(ostream& os, const CellDict<DIMS>& m)
{
  
  os << m.getBounds();
  for(auto el : m) {
    os << *el.second;
    os << ' ';
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
void Cell<DIMS>::shallowGatherIn(CellDict<DIMS> & dict) {
  if(!this->isLeaf()) {
    FOR(i, this->subsCount()) {
      dict.addCell(subs[i]);
    }
  }
}


template <int DIMS>
void ensureSplit(CellDict<DIMS> & dict, CellId<DIMS> id) {

  cerr << "?ENS " << id << endl;
  if(id.isRoot()) return;
  if(dict.getCell(id) != NULL) return;
  
  auto parent = id.getParentId();

  cerr << "ENS " << id << endl;
  ensureSplit(dict, parent);

  cerr << "ENS! " << id << endl;
  
  Cell<DIMS> * c = dict.getCell(parent);
  
  c->split();
  c->gatherIn(dict);
  
  FOR(dim, DIMS) {
    auto a = parent.getMovedId(dim, -1);
    auto b = parent.getMovedId(dim, +1);
    cerr << "nei: " << a << "'" << a.isValidCellId() << " " << endl;
    cerr << "nei: " << b << "'" << b.isValidCellId() << " " << endl;
    if(a.isValidCellId()) ensureSplit(dict, a);
    if(b.isValidCellId()) ensureSplit(dict, b);
  }
}

template<int DIMS>
void printGrid(const CellDict<DIMS> & dict, int lvl) {
  const int MAXP = 34;
  char buff [MAXP+1][MAXP+1];
  FOR(i, MAXP+1) {
    FOR(j, MAXP+1) buff[i][j]=(j==MAXP || i==MAXP)?0:' ';
  }

  int offset = 1<<lvl;
  int maxrow = 0;
  for (auto el : dict) {
    Cell<DIMS> * c = el.second;
    if(!c->isLeaf()) continue;
    
    auto c1 = c->getCornerId(0, lvl);
    auto c2 = c->getCornerId(3, lvl);
    
    // cerr << *c << " " << c1 << " " << c2 << endl;
    
    int x = c1[0]-offset;
    int y = c1[1]-offset;
    int x1 = c2[0]-offset;
    int y1 = c2[1]-offset;
    if(x>=MAXP || y >= MAXP) continue;
    // cerr << "+ " << x << ", " << y <<endl;
    buff[x][y] = '+';

    if(x1<MAXP) buff[x1][y] = '+';
    if(y1<MAXP) buff[x][y1] = '+';
    if(x1<MAXP && y1<MAXP) buff[x1][y1] = '+';
    maxrow = max(x1+1, maxrow);    
  }
  FOR(i, min(maxrow+2,MAXP)) {
    cerr << buff[i] << endl;
  }
}

template<int DIMS>
void Cell<DIMS>::divideByHyperplane(Plane hyperplane, Dict& a, Dict& b){
  cerr << *this << " hyp:" << hyperplane << " CROSS:" << crossesHyperplane(hyperplane) << endl;
  if(crossesHyperplane(hyperplane)) {
    assert(!isLeaf());
    FOR(i, subsCount()) {
      subs[i]->divideByHyperplane(hyperplane, a, b);
    }
  } else {
    if(overHyperplane(hyperplane)) {
      b.addCell(this);
    } else {
      a.addCell(this);
    }
  }
}