#include <iostream>
#include <bitset>
#include <string>
#include <algorithm>
#include <map>
#include <set>

#include <assert.h>
#define FOR(x, n) for(int x = 0, __n = (n); x < __n; x++)
#define FORI(x, a, n) for(int x = (a), __n = (n); x < __n; x++)
#define FORR(x, n) for(int x = (n)-1; x >= 0; x--)
#define R(a) (a).begin(), (a).end()

using namespace std;
#include "CellId.h"
#include "CellBounds.h"
#include "Cell.h"
#include "CellDict.h"

const int DIM = 2;
int main(int argc, char** argv) {

  Cell<DIM> c;
  cout << c.getId() << " <> BNDS:" << c.getBounds() << endl;
  c.print(cout);

  CellDict<DIM> D;
  c.gatherIn(D);
  cout << D << endl;
  cout << "####" << endl;
  
  auto target = c.getId().getChildId(0).getChildId(0).getChildId(0);
  cout << c.getId().getChildId(0) << endl;
  cout << c.getId().getChildId(0).getChildId(1) << endl;
  cout << target << " -> " << target.getParentId() << endl;
  int lvl = 4;
  int offset = 1<<lvl;
  FOR(i, offset) {
    ensureSplit(D, CellId<DIM>({offset+i, offset+4}));
  }
  
  // ensureSplit(D, target);
  cout << "####" << endl;
  cout << D;
  cout << endl;
  cout << "####" << endl;
  c.print(cout);
  cout << c.countLeaves() << ' ' << c.countCells() << ' ' << c.getMaxLevel() << endl;;
  int maxLvl = c.getMaxLevel() + 1;
  printGrid(D, lvl);
  
  set<Hyperplane<DIM> > S;
  map<CellId<DIM>, set<Hyperplane<DIM> > > cache;
  c.gatherHyperplanes(S, c.getMaxLevel(), cache);
  for(auto el : S) {
    cout << ")" << el << endl;
  }
  CellDict<DIM> D1;
  D1.addCell(&c);
  
  pair<CellDict<DIM>, CellDict<DIM> > r = D1.divideByHyperplane(Hyperplane<DIM>(1, 0x15, 4));
  cout << "1] " << r.first << endl;
  cout << "2] " << r.second << endl;
  
  r.first.complete();
  r.second.complete();
  cout << "1] " << r.first << endl;
  cout << "2] " << r.second << endl;
  printGrid(r.first, lvl);
  printGrid(r.second, lvl);
  return 0;
}