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
  int relCount;
public:
  Supernode(const Id& a,int dimtype):id(a.toCanonical()),dimtype(dimtype), relCount(1<<(DIMS - dimtype)) {
    assert(dimtype <= DIMS);
    
  }
  
  int getType() const {
    return dimtype;
  }
  const Id& getId() const {
    return id;
  }
  int getRelCount() const {
    return relCount;
  }
  int getLevel() const {
    return id.getLevel();
  }
  
  bool operator<(const Supernode& n) const {
    if(id != n.id) return id < n.id;
    return dimtype < n.dimtype;
  }
  
  int countBoundaryHyperplanes(const CellBounds<DIMS>& b) const {
    return b.countHyperplanes(id);
  }
  
};





template <int DIMS>
ostream& operator<<(ostream& os, const Supernode<DIMS>& node) {
  os << '{' << node.getType() << '#' << node.getId() << '(' << node.getRelCount() << ')' <<'}';
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

template <int DIMS>
ostream& operator<<(ostream& os, const vector<Supernode<DIMS> >& nodes) {
  os << "[[VEC:";
  for(auto el: nodes) {
    os << el << " ";
  }
  os << "]]";
  return os;
}

template <int DIMS>
set<Supernode<DIMS> > calculateNodes(CellDict<DIMS> dict) {
  
  
}

