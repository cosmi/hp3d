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
public:
  
  void normalize() {
    assert(a.isNotLargerThan(b));
    a = a.toCanonical();
    b = b.toCanonical();
    int lvl = max(a.getLevel(), b.getLevel());
    a = a.withLevel(lvl);
    b = b.withLevel(lvl);
  }
  Supernode(const Id& _a,const Id& _b):a(_a), b(_b) {
    normalize();
  }  //
  // Supernode withBoundsDim(int dim) {
  //   Supernode s(*this);
  //   s.boundsDim = dim;
  //   return s;
  // }
  
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
    return b;
  }
  // int getBoundsDim() const {
  //   return boundsDim;
  // }
  int getLevel() const {
    return a.getLevel();
  }
  // int isComplete(int hits) const {
  //   return (hits<<boundsDim == 1<<DIMS);
  // }
  
  Supernode withLevel(int lvl) const {
    Supernode s = *this;
    s.a = s.a.withLevel(lvl);
    s.b = s.b.withLevel(lvl);
    return s;
  }
  
  bool operator<(const Supernode& n) const {
    if(a != n.a) return a < n.a;
    return b < n.b;
  }
  
  // int countBoundsHyperplanes(const CellBounds<DIMS>& b) const {
  //   return b.countHyperplanes(a);
  // }
  vector<Id> getElements() const {
    // Returns ids of all potential elements
    vector<Id> V1 = a.getNeighborsToCorner();
    vector<Id> V2 = b.getNeighborsToCorner();
    sort(V1.begin(), V1.end());
    sort(V2.begin(), V2.end());
    vector<Id> res(1<<(DIMS-getType()));
    // cerr << V1 << " ?" << is_sorted(V1.begin(), V1.end()) << endl;
    // cerr << V2 << " ?" << is_sorted(V2.begin(), V2.end()) << endl;
    typename vector<Id>::iterator end = set_intersection(V1.begin(), V1.end(), V2.begin(), V2.end(), res.begin());
    // cerr << "ELEMENTS: " << res << endl;
    
    assert(end == res.end());
    return res;
  }
  Supernode getParent() const {
    return Supernode(a.getParentId(), b.offsetBinary((1<<DIMS)-1).getParentId());
  }
  
  vector<Supernode> getHigherDimNodes() const {
    vector<Supernode> V;
    
    FOR(i, DIMS) if(a[i] != b[i]) {
      auto s1 = Supernode(*this);
      auto s2 = Supernode(*this);
      s1.b[i] = a[i];
      s2.a[i] = b[i];
      s1.normalize();
      s2.normalize();
      V.push_back(s1);
      V.push_back(s2);
    }
    
    assert(V.size() == getType() * 2);
    return V;
  }
  
  int boundDims(CellBounds<DIMS> bounds) const {
    if(bounds.getLevel() > getLevel()) {
      return withLevel(bounds.getLevel()).boundDims(bounds);
    } else if(bounds.getLevel() < getLevel()) {
      bounds = bounds.withLevel(getLevel());
    }
    
    int cnt = 0;
    FOR(i, DIMS) if(a[i] == b[i]) {
      if(a[i] == bounds.getFrom()[i] || a[i] == bounds.getTo()[i]) {
        cnt++;
      }
    }
    return cnt;
  }
};

template <int DIMS>
bool CellDict<DIMS>::isConstrained(const Supernode<DIMS>& node) const {
  // cerr << "CONSTR? " << node << endl;
  auto bounds = getBounds();
  int boundDims = node.boundDims(bounds);
  
  int elCnt = 0;
  
  vector<CellId<DIMS> > V = node.getElements();
  for(auto id : V) {
    if(getCell(id)) elCnt++;
  }
  
  bool constrained = !(elCnt == 1<<(DIMS-node.getType()-boundDims));
  // cerr << "CONSTR: " << constrained << " " << "TYPE:" << node.getType() << " BOUND:" << boundDims << endl;
  return constrained;
}

template <int DIMS>
set<Supernode<DIMS> > getDependentNodes(const CellId<DIMS>& id, const CellDict<DIMS>& dict) {
  using NodeSet = set<Supernode<DIMS> >;
  auto* cell = dict.getCell(id);
  assert(cell != NULL);
  NodeSet nodes[DIMS+1];
  NodeSet res;
  // cerr << "TU" << endl;
  auto innerNode = cell->getInnerNode();
  nodes[0].insert(innerNode);
  res.insert(innerNode);
  // cerr << "UOO " << res << endl;
  for(int dim = 0; dim < DIMS; dim++) {
    // cerr << endl << "##### DIM: " << dim << endl;
    for(auto node : nodes[dim]) {
      // cerr << "NODE: " << node << endl;
      vector<Supernode<DIMS> > subnodes = node.getHigherDimNodes();
      // cerr << "SUBS: " << subnodes << endl;
      for(auto subnode : subnodes) {
        while(dict.isConstrained(subnode)) {
          subnode = subnode.getParent();
        }
        nodes[dim+1].insert(subnode);
        res.insert(subnode);
      }
    }
  }  
  return res;
}



template <int DIMS>
ostream& operator<<(ostream& os, const Supernode<DIMS>& node) {
  os << '{' << node.getType() << '#' << node.getFrom() << "->" << node.getTo() <<'}';
  return os;
}
//
// // template<DIMS>
// // vector<Supernode<DIMS> > && inducedNodes(Cell<DIMS> cell);
//
// vector<Supernode<1> > inducedNodes(Cell<1> cell) {
//   vector<Supernode<1> > V;
//   auto zero = cell.getZeroCornerAt(cell.getLevel());
//   auto last = cell.getLastCornerAt(cell.getLevel())
//   V.push_back(Supernode<1>(zero, zero));
//   V.push_back(Supernode<1>(last, last));
//   V.push_back(
//     Supernode<1>(
//       zero, last
//     )
//   );
//
//   return V;
// }
//
// template<int DIMS>
// vector<Supernode<DIMS> > inducedNodes(Cell<DIMS> cell) {
//   using Node = Supernode<DIMS>;
//   using Cell = Cell<DIMS>;
//   using Bounds = CellBounds<DIMS>;
//   using Id = CellId<DIMS>;
//
//   vector<Supernode<DIMS> > V;
//
//
//   int len = cell.countCorners();
//   FOR(i, len) {
//     const Id& c = cell.getCornerId(i);
//     FOR(j, len){
//       const Id& d = cell.getCornerId(j);
//       if(!c.isNotLargerThan(d)) continue;
//       int dimtype = DIMS - countCommonHyperplanes(c,d);
//       V.push_back(Node(c, d));
//     }
//   }
//
//   return V;
// }

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

template<class TYPE>
ostream& operator<<(ostream& os, const vector<TYPE> S) {
  printCollectionToStream(os, S, "VEC");
  return os;
}
//
// template <int DIMS>
// set<Supernode<DIMS> > calculateNodes(const CellDict<DIMS>& dict) {
//   auto bounds = dict.getBounds();
//
//   map<CellId<DIMS>, pair<int,int> > M;
//   cerr << "BOUNDS " << bounds << endl;
//   for(auto it : dict) if(it.second->isLeaf()) {
//     auto & cell = *(it.second);
//     auto nodes = inducedNodes(cell);
//     for(auto node : nodes) {
//       // int boundsDim = node.countBoundsHyperplanes(bounds);
//       // auto n = node.withBoundsDim(boundsDim);
//       // cerr<< "ADD " << node << " " << node.boundsDim << endl;
// //      hits[]+=;
//       auto& p = M[node.getId()];
//       p.first += 1<<node.getType(); // number of hits
//       p.second = max(p.second, node.getType()); // type of node
//     }
//   }
//   set<Supernode<DIMS> > S;
//   for(auto it : M) {
//     auto id = it.first;
//     int hits = it.second.first;
//     int type = it.second.second;
//     int boundaryHits = bounds.countHyperplanes(id);
//     auto node = Supernode<DIMS>(id, type).withBoundsDim(boundaryHits);
//     if(node.isComplete(hits)) {
//       S.insert(node);
//     } else {
//       // S.insert(node);
//       cerr << "ELIMINATED: " << node << " ONLY " << hits << " HITS "<< endl;
//     }
//   }
//   return S;
// }

//
// template <int DIMS>
// pair<set<Supernode<DIMS> >, set<Supernode<DIMS> > > divideByHyperplane(
//     const set<Supernode<DIMS> > & nodes,
//     Hyperplane<DIMS> hp) {
//   pair<set<Supernode<DIMS> >, set<Supernode<DIMS> > > res;
//   for(auto node: nodes) {
//     int cmp = node.getId().compareWithHyperplane(hp);
//
//     if(cmp >= 0) res.second.insert(node);
//     if(cmp <= 0) res.first.insert(node);
//   }
//
//   return res;
// }
//
//
// template<int DIMS>
// void printGrid(set<Supernode<DIMS> > & S, int lvl) {
//   const int MAXP = 34;
//   char buff [MAXP+1][MAXP+1];
//   FOR(i, MAXP+1) {
//     FOR(j, MAXP+1) buff[i][j]=(j==MAXP || i==MAXP)?0:' ';
//   }
//
//   int offset = 1<<lvl;
//   for (auto el : S) {
//     auto c = el.getId().withLevel(lvl);
//
//     // cerr << *c << " " << c1 << " " << c2 << endl;
//
//     int x = c[0]-offset;
//     int y = c[1]-offset;
//     if(x>=MAXP || y >= MAXP) continue;
//     // cerr << "+ " << x << ", " << y <<endl;
//     buff[x][y] = '*';
//   }
//   FOR(i, MAXP) {
//     cerr << buff[i] << endl;
//   }
// }
//
