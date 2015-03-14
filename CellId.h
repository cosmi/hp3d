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
const id_int ROOT_ID = 1;
using namespace std;


string printBinary(id_int x, int digits) {
  string s(digits, '#');
  FOR(i, digits) {
    s[digits-i-1] = (x&(1<<i)?'1':'0');
  }
  return s;
}




template<int DIMS>
class CellId {
  id_int id[DIMS];
  size_t lvl;
public:
  const id_int& operator[](size_t p) const {
    return id[p];
  }
  id_int& operator[](size_t p) {
    return id[p];
  }
  
  CellId(): lvl(0) {
    FOR(i, DIMS) id[i]=1;
  }
  CellId(std::initializer_list<id_int> list ):lvl(0) {
    int i = 0;
    for( auto elem: list) {
      id[i++] = elem;
    }
    int k = id[0];
    while(k>1) {
      lvl++;
      k>>=1;
    }
  }
  // CellId(const CellId& c):lvl(c.lvl) {
  //   FOR(i, DIMS) id[i] = c[i];
  // }
  CellId(const CellId& c, int dim, int move):lvl(c.lvl) {
    FOR(i, DIMS) {
      id[i] = c[i];
    }
    id[dim] += move;
  }

  size_t getLevel() const {
    return lvl;
  }
  bool isRoot() const {
    return getLevel() == 0;
  }
  
  CellId offsetBinary(size_t moveBy) const {
    CellId c;
    FOR(i, DIMS) {
      int offset = (moveBy&(1<<i))!=0?1:0;
      c[i] = id[i] + offset;
    }
    c.lvl = lvl;
    return c;
  }
  CellId getChildId(size_t childId) const {
    // CellId c;
//     FOR(i, DIMS) {
//       int offset = (childId&(1<<i))!=0?1:0;
//       c[i] = 2 * id[i] + offset;
//     }
//     c.lvl = lvl + 1;
    // return c;
    return this->increaseLevelBy(1).offsetBinary(childId);
  }
  CellId getMovedId(int dim, int move) const {
    return CellId(*this, dim, move);
  }
  bool isAncestorOf(const CellId& cell) const {
    int delta = cell.getLevel() - getLevel();
    if(delta <= 0) return false;
    FOR(i, DIMS) {
      if(cell[i]>>delta != id[i]) return false;
    }
    return true;
  }
  CellId increaseLevelBy(int lvls) const {
    CellId c = *this;
    c.lvl+=lvls;
    FOR(i, DIMS) {
      c[i] = this->id[i] << lvls;
    }
    return c;
  }
  
  CellId withLevel(int newLvl) const {
    return increaseLevelBy(newLvl - lvl);
  }
  
  bool isValidCellId() const {
    id_int minv = 1<<getLevel();
    id_int maxv = 1<<(getLevel()+1);
    FOR(i, DIMS) {
      if(id[i] < minv || id[i] >= maxv) {
        return false;
      }
    }
    return true;
  }
  
  bool isValidCornerId() const {
    id_int minv = 1<<getLevel();
    id_int maxv = 1<<(getLevel()+1);
    FOR(i, DIMS) {
      if(id[i] < minv || id[i] > maxv) {
        return false;
      }
    }
    return true;
  }
  
  bool isValidHyperplaneId() const {
    id_int minv = 1<<getLevel();
    id_int maxv = 1<<(getLevel()+1);
    int zeros = 0;
    FOR(i, DIMS) {
      if(id[i] == 0) {
        zeros++;
      } else if(id[i] < minv || id[i] >= maxv) {
        return false;
      }
    }
    return zeros == DIMS-1;
  }
  CellId getHyperplane(int dim) {
    CellId c;
    c.lvl = this->lvl;
    FOR(i, DIMS) {
      c[i] = 0;
    }
    c[dim] = id[dim];
    return c;
  }
  
  CellId getParentId() const {
    CellId c;
    FOR(i, DIMS) {
      c[i] = id[i] >> 1;
    }
    c.lvl = getLevel() - 1;
    return c;
  }
  void print(ostream& cout) const {
    int digits = getLevel() + 2;
    FOR(i, DIMS) {
      if(i > 0) cout << ":";
      cout << printBinary(id[i], digits);
    }
  }
  bool operator< (const CellId<DIMS>& d) const {
    FOR(i, DIMS) {
      if(id[i] < d[i]) return true;
      else if(id[i] > d[i]) return false;
    }
    return false;
  }
  bool operator==(const CellId<DIMS>& d) const {
    FOR(i, DIMS) {
      if(id[i] != d[i]) return false;
    }
    return true;
  }
  
  
};



template <int DIMS>
ostream& operator<<(ostream& os, const CellId<DIMS>& cell) {
  cell.print(os);
  return os;
}