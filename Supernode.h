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






template<int DIMS>
class Supernode {
  using Id = CellId<DIMS>;
  using Plane = Hyperplane<DIMS>;
  Id a, b;
  int boundsDim;
public:
  Supernode(const Id& a,const Id& b):a(a.toCanonical()), b(b.toCanonical()), boundsDim(0) {
    assert(dimtype <= DIMS);
    
    int lvl = min(a.getLevel(), b.getLevel());
    a = a.withLevel(lvl);
    b = b.withLevel(lvl);
    
  }
  Supernode withBoundsDim(int dim) {
    Supernode s(*this);
    s.boundsDim = dim;
    return s;
  }
  
  int getType() const {
    int count = DIMS;
    FOR(i,DIMS) {
      if(a[i] == b[i]) {
        count--;
      }
    }
    return count;
  }
  const Id& getFrom() const {
    return a;
  }
  const Id& getTo() const {
    return a;
  }
  int getBoundsDim() const {
    return boundsDim;
  }
  int getLevel() const {
    return id.getLevel();
  }
  int isComplete(int hits) const {
    return (hits<<boundsDim == 1<<DIMS);
  }
  
  bool operator<(const Supernode& n) const {
    if(a != n.a) return a < n.a;
    if(b != n.b) return b < n.b;
    return boundsDim < n.boundsDim;
  }
  
  int countBoundsHyperplanes(const CellBounds<DIMS>& b) const {
    return b.countHyperplanes(id);
  }
  vector<Id> getElements() const {
    vector<Id> V1 = from.getNeighborsToCorner();
    vector<Id> V2 = to.getNeighborsToCorner();
    sort(V1.begin(), V1.end());
    sort(V2.begin(), V2.end());
    vector<Id> res(1<<(DIMS-getType()));
    auto end = set_intersection(V1.begin(), V1.end(), V2.begin(), V2.end(), res);
    assert(end == res.end());
    return res;
  }
  Supernode getParent() const {
    return Supernode(from.getParentId(), to.offsetBinary((1<<DIMS)-1).getParentId());
  }
};





template <int DIMS>
ostream& operator<<(ostream& os, const Supernode<DIMS>& node) {
  os << '{' << node.getType() << '#' << node.getFrom() << "->" << node.getTo() << '(' << node.getBoundsDim() << ')' <<'}';
  return os;
}

// template<DIMS>
// vector<Supernode<DIMS> > && inducedNodes(Cell<DIMS> cell);

vector<Supernode<1> > inducedNodes(Cell<1> cell) {
  vector<Supernode<1> > V;
  auto zero = cell.getZeroCornerAt(cell.getLevel());
  auto last = cell.getLastCornerAt(cell.getLevel())
  V.push_back(Supernode<1>(zero, zero));
  V.push_back(Supernode<1>(last, last));
  V.push_back(
    Supernode<1>(
      zero, last
    )
  );
             
  return V;
}

template<int DIMS>
vector<Supernode<DIMS> > inducedNodes(Cell<DIMS> cell) {
  using Node = Supernode<DIMS>;
  using Cell = Cell<DIMS>;
  using Bounds = CellBounds<DIMS>;
  using Id = CellId<DIMS>;
  
  vector<Supernode<DIMS> > V;
  
  
  int len = cell.countCorners();
  FOR(i, len) {
    const Id& c = cell.getCornerId(i);
    FOR(j, len){
      const Id& d = cell.getCornerId(j);
      if(!c.isNotLargerThan(d)) continue;
      int dimtype = DIMS - countCommonHyperplanes(c,d);
      V.push_back(Node(c, d));
    }
  }
  
  return V; 
}

template <typename Container>
void printCollectionToStream(ostream& os, const Container& nodes, const char* prefix) {
  os << "[[" << prefix << "(" << nodes.size() << ")" <<":";
  for(auto el: nodes) {
    os << el << " ";
  }
  os << "]]";
}

template<int DIMS>
ostream& operator<<(ostream& os, const set<Supernode<DIMS> > S) {
  printCollectionToStream(os, S, "SET");
  return os;
}
template<int DIMS>
ostream& operator<<(ostream& os, const vector<Supernode<DIMS> > S) {
  printCollectionToStream(os, S, "VEC");
  return os;
}

template <int DIMS>
set<Supernode<DIMS> > calculateNodes(const CellDict<DIMS>& dict) {
  auto bounds = dict.getBounds();
  
  map<CellId<DIMS>, pair<int,int> > M;
  cerr << "BOUNDS " << bounds << endl;
  for(auto it : dict) if(it.second->isLeaf()) {
    auto & cell = *(it.second);
    auto nodes = inducedNodes(cell);
    for(auto node : nodes) {
      // int boundsDim = node.countBoundsHyperplanes(bounds);
      // auto n = node.withBoundsDim(boundsDim);
      // cerr<< "ADD " << node << " " << node.boundsDim << endl;
//      hits[]+=;
      auto& p = M[node.getId()];
      p.first += 1<<node.getType(); // number of hits 
      p.second = max(p.second, node.getType()); // type of node
    }
  }
  set<Supernode<DIMS> > S;
  for(auto it : M) {
    auto id = it.first;
    int hits = it.second.first;
    int type = it.second.second;
    int boundaryHits = bounds.countHyperplanes(id);
    auto node = Supernode<DIMS>(id, type).withBoundsDim(boundaryHits);
    if(node.isComplete(hits)) {
      S.insert(node);
    } else {
      // S.insert(node);
      cerr << "ELIMINATED: " << node << " ONLY " << hits << " HITS "<< endl;
    }
  }
  return S;
}


template <int DIMS>
pair<set<Supernode<DIMS> >, set<Supernode<DIMS> > > divideByHyperplane(
    const set<Supernode<DIMS> > & nodes, 
    Hyperplane<DIMS> hp) {
  pair<set<Supernode<DIMS> >, set<Supernode<DIMS> > > res;
  for(auto node: nodes) {
    int cmp = node.getId().compareWithHyperplane(hp);
    
    if(cmp >= 0) res.second.insert(node);
    if(cmp <= 0) res.first.insert(node);
  }
  
  return res;
}


template<int DIMS>
void printGrid(set<Supernode<DIMS> > & S, int lvl) {
  const int MAXP = 34;
  char buff [MAXP+1][MAXP+1];
  FOR(i, MAXP+1) {
    FOR(j, MAXP+1) buff[i][j]=(j==MAXP || i==MAXP)?0:' ';
  }

  int offset = 1<<lvl;
  for (auto el : S) {
    auto c = el.getId().withLevel(lvl);
    
    // cerr << *c << " " << c1 << " " << c2 << endl;
    
    int x = c[0]-offset;
    int y = c[1]-offset;
    if(x>=MAXP || y >= MAXP) continue;
    // cerr << "+ " << x << ", " << y <<endl;
    buff[x][y] = '*';    
  }
  FOR(i, MAXP) {
    cerr << buff[i] << endl;
  }
}

