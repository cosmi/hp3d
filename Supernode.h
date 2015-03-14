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
  Id id;
  int dimtype;
  int boundsDim;
public:
  Supernode(const Id& a,int dimtype):id(a.toCanonical()),dimtype(dimtype), boundsDim(0) {
    assert(dimtype <= DIMS);
    
  }
  Supernode withBoundsDim(int dim) {
    Supernode s(*this);
    s.boundsDim = dim;
    return s;
  }
  
  int getType() const {
    return dimtype;
  }
  const Id& getId() const {
    return id;
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
    if(id != n.id) return id < n.id;
    if(dimtype != n.dimtype) return dimtype < n.dimtype;
    return boundsDim < n.boundsDim;
  }
  
  int countBoundsHyperplanes(const CellBounds<DIMS>& b) const {
    return b.countHyperplanes(id);
  }
  
};





template <int DIMS>
ostream& operator<<(ostream& os, const Supernode<DIMS>& node) {
  os << '{' << node.getType() << '#' << node.getId() << '(' << node.getBoundsDim() << ')' <<'}';
  return os;
}

// template<DIMS>
// vector<Supernode<DIMS> > && inducedNodes(Cell<DIMS> cell);

vector<Supernode<1> > inducedNodes(Cell<1> cell) {
  vector<Supernode<1> > V;
  V.push_back(Supernode<1>(cell.getZeroCornerAt(cell.getLevel()), 0));
  V.push_back(Supernode<1>(cell.getLastCornerAt(cell.getLevel()), 0));
  V.push_back(
    Supernode<1>(
      middle(
        cell.getZeroCornerAt(cell.getLevel()),
        cell.getLastCornerAt(cell.getLevel())
      ),
      1
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
      V.push_back(
        Node(
          middle(
            c,d
          ),
          dimtype
        )
      );
    }
  }
  
  return V; 
}

template <typename Container>
void printCollectionToStream(ostream& os, const Container& nodes, const char* prefix) {
  os << "[[" << prefix << ":";
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
  cout << "BOUNDS " << bounds << endl;
  for(auto it : dict) if(it.second->isLeaf()) {
    auto & cell = *(it.second);
    auto nodes = inducedNodes(cell);
    for(auto node : nodes) {
      // int boundsDim = node.countBoundsHyperplanes(bounds);
      // auto n = node.withBoundsDim(boundsDim);
      // cout<< "ADD " << node << " " << node.boundsDim << endl;
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
      cout << "ELIMINATED: " << node << " ONLY " << hits << " HITS "<< endl;
    }
  }
  return S;
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
    
    // cout << *c << " " << c1 << " " << c2 << endl;
    
    int x = c[0]-offset;
    int y = c[1]-offset;
    if(x>=MAXP || y >= MAXP) continue;
    // cout << "+ " << x << ", " << y <<endl;
    buff[x][y] = '*';    
  }
  FOR(i, MAXP) {
    cout << buff[i] << endl;
  }
}

