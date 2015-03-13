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
public:
  const id_int& operator[](size_t p) const {
    return id[p];
  }
  id_int& operator[](size_t p) {
    return id[p];
  }
  CellId() {
    FOR(i, DIMS) id[i]=1;
  }
  CellId(std::initializer_list<id_int> list ) {
    int i = 0;
    for( auto elem: list) {
      id[i++] = elem;
    }
  }
  CellId&& getParent() const {
    CellId c;
    FOR(i, DIMS) {
      c[i] = id[i] >> 1;
    }
    return c;
  }
  void print(ostream& cout) const {
    int lvl = 0;
    int i = id[0];
    while(i) {
      lvl++;
      i>>=1;
    }
    FOR(i, DIMS) {
      if(i > 0) cout << ":";
      cout << printBinary(id[i], lvl);
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