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
class CellBounds {
  using Id = CellId<DIMS>;
  using Plane = Hyperplane<DIMS>;
  Id a, b;
public:
  CellBounds(const Id& a, const Id& b):a(a),b(b) {
    assert(a.getLevel() == b.getLevel());
  }
  CellBounds():a(Id()), b(Id()){}
  const Id& getZeroCornerId() const {
    return a;
  }
  const Id& getLastCornerId() const {
    return b;
  }
  
  int getLevel() const {
    return a.getLevel();
  }
  
  bool isInternal(const Id& id) const {
    FOR(i, DIMS) {
      if(a[i] >= id[i] || b[i] <= id[i]) return false;
    }
    return true;
  }
  bool isInBounds(const Id& id) const {
    FOR(i, DIMS) {
      if(a[i] > id[i] || b[i] < id[i]) return false;
    }
    return true;
  }
  
  CellBounds withLevel(int lvl) const {
    return CellBounds(a.withLevel(lvl), b.withLevel(lvl));
  }
  
  pair<CellBounds, CellBounds> divideByHyperplane(Plane hp) const {
    auto thisAt = this->withLevel(hp.getLevel());
    auto p = make_pair(thisAt, thisAt);
    int dim = hp.getDim();
    auto val = hp.getVal();
    p.first.b[dim] = val;
    p.second.a[dim] = val;
  }
  
  CellBounds join(const CellBounds& cb) const {
    if(isNull()) return cb;
    if(cb.isNull()) return *this;
    int tgtLvl = max(getLevel(), cb.getLevel());
    CellBounds c = this->withLevel(tgtLvl);
    CellBounds d = cb.withLevel(tgtLvl);
    
    FOR(i, DIMS) {
      c.a[i] = min(d.a[i], c.a[i]);
      c.b[i] = max(d.b[i], c.b[i]);
    }
    return c;
  }
  
  bool isNull() const {
    FOR(i, DIMS) {
      if(a[i] != b[i]) return false;
    }
    return true;
  }
  
  int countHyperplanes(Id id) const {
    int count = 0;
    FOR(i, DIMS) {
      if(a[i] == id[i] || b[i] == id[i]) count++;
    }
    return count;
  }
};



template <int DIMS>
ostream& operator<<(ostream& os, const CellBounds<DIMS>& cb) {
  os << '<' << (cb.isNull()?"NULL:":"")<< '{' << cb.getZeroCornerId().getLevel() << '}'<< cb.getZeroCornerId() << '&' << cb.getLastCornerId() << '>';
  return os;
}