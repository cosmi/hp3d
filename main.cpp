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

template<int DIMS>
class Supernode;
#include "CellId.h"
#include "CellBounds.h"
#include "Cell.h"
#include "CellDict.h"
#include "Supernode.h"
typedef long long result_int;

result_int flopsFun(result_int a, result_int b) {
  // a - eliminated
  // b - total
  result_int res = (a * (6*b*b - 6*a*b + 6*b + 2*a*a - 3*a + 1))/6;
  // cout << "CALC f(" << a << "," << b << ") = " <<
  //   res << endl;
  return res;
}

template <int DIMS>
using NodeCounter = map<Supernode<DIMS>, int>;
template <int DIMS>
using CellNodes = map<CellId<DIMS>, vector<Supernode<DIMS> > >;

template<int DIMS>
struct CalcStruct {
  const CellDict<DIMS>& dict;
  const CellNodes<DIMS> nodes;
  const NodeCounter<DIMS> counter;
};



template <int DIMS>
CalcStruct<DIMS> prepareStruct(const CellDict<DIMS>& dict) {
  CellNodes<DIMS> nodes;
  NodeCounter<DIMS> counter;
  for(auto it: dict) {
    auto & cell = *it.second;
    if(!cell.isLeaf()) continue;
    auto & vec = nodes[cell.getId()];
    
    auto S = getDependentNodes(cell.getId(), dict);
    vec.insert(vec.end(), S.begin(), S.end());
    for(auto node: S) {
      counter[node]++;
    }
  }
  return CalcStruct<DIMS>({dict, nodes, counter});
  
}

template<int DIMS>
struct RetStruct {
  result_int cost;
  NodeCounter<DIMS> selected;
  
};

template<int DIMS>
RetStruct<DIMS> calculateSingleElementCost(const CalcStruct<DIMS>& str, const CellDict<DIMS>& dict) {
  using RetStruct = RetStruct<DIMS>;
  assert(dict.size() == 1);
  auto * cell = dict.begin()->second;
  auto & nodes = str.nodes.find(cell->getId())->second;
  RetStruct R;
  
  int eliminated = 0;
  for(auto node : nodes) {
    int v = 1;
    if(v == str.counter.find(node)->second) {
      eliminated++;
    } else {
      R.selected[node] = v;
    }
  }
  R.cost = flopsFun(eliminated, R.selected.size() + eliminated);
  return R;
}

template<int DIMS>
RetStruct<DIMS> calculateHalfDivCost(const CalcStruct<DIMS>& str, const CellDict<DIMS>& dict) {
  using RetStruct = RetStruct<DIMS>;
  
  if(dict.size() <= 1) {
    return calculateSingleElementCost(str, dict);
  }
  
  auto& bounds = dict.getBounds();
  auto hp = bounds.getHalvingPlane();
  cerr << endl << "IN " << bounds << " DIV " << hp << endl;
  printGrid(dict, 5);

  
  auto dicts = dict.divideByHyperplane(hp);

  RetStruct a = calculateHalfDivCost(str, dicts.first);
  RetStruct b = calculateHalfDivCost(str, dicts.second);

  RetStruct R;
  for(auto el: a.selected) {
    R.selected[el.first] += el.second;
  }
  for(auto el: b.selected) {
    R.selected[el.first] += el.second;
  }
  int eliminated = 0;
  for(auto it = R.selected.begin(); it != R.selected.end();) {
    if(str.counter.find(it->first)->second == it->second) {
      it = R.selected.erase(it);
      eliminated++;
    } else {
      it++;
    }
  }

  result_int res = flopsFun(eliminated, eliminated + R.selected.size());
  cerr << "ELIMINATION-COST:" << res << endl;

  R.cost = res + a.cost + b.cost;

  // cerr << "DICTS " << dicts.first << " AND " << dicts.second << endl;
//   cerr << "NODES " << nds.first << " AND " << nds.second << endl;
//
  

  return R;
}
//
//
template <int DIMS>
result_int calculateNestedDissection(const CellDict<DIMS>& dict) {
  auto str = prepareStruct(dict);
  return calculateHalfDivCost(str, dict).cost;
}
//
//
// template <int DIMS>
// void ensureSingularity(CellId<DIMS> id, CellDict<DIMS>& D, int singDims, int lvl, int dim) {
//   if(dim == singDims) {
//     cerr << "ENSURE " << id << endl;
//     ensureSplit(D, id);
//   } else {
//     FOR(i, 1<<lvl) {
//       ensureSingularity(id.getMovedId(dim, i), D, singDims, lvl, dim+1);
//     }
//   }
// }
//
// template <int DIMS>
// result_int calcCost(int singDims, int lvl) {
//    using Id = CellId<DIMS>;
//    Cell<DIMS> c;
//    CellDict<DIMS> D;
//    c.gatherIn(D);
//
//    Id id = c.getId();
//
//    FOR(i, lvl) {
//      id = id.getChildId(0);
//    }
//    ensureSingularity(id, D, singDims, lvl, 0);
//    return calculateHalfDivCost(D);
// }
//
// void outputResultsForSingularities() {
//
//   //
//   // FOR(dim, 1) {
//   //   FOR(lvl, 5) {
//   //     cout << "D:\t1\t";
//   //     cout << "\tSINGULARITY-DIM:\t" << dim << "\tLVL:\t" << lvl << "\t" << calcCost<1>(dim,lvl) << endl;
//   //   }
//   // }
//   //
//   cout << endl;
//   FOR(dim, 2) {
//     FOR(lvl, 7) {
//       cout << "D:\t2\t";
//       cout << "\tSINGULARITY-DIM:\t" << dim << "\tLVL:\t" << lvl << "\t" << calcCost<2>(dim,lvl) << endl;
//     }
//   }
//    //
//   // cout << endl;
//   // FOR(dim, 4) {
//   //   FOR(lvl, min(5, 7-dim)) {
//   //     cout << "D:\t3\t";
//   //     cout << "\tSINGULARITY-DIM:\t" << dim << "\tLVL:\t" << lvl << "\t" << calcCost<3>(dim,lvl) << endl;
//   //   }
//   // }
//   //
//   // cout << endl;
//   // FOR(dim, 5) {
//   //   FOR(lvl, min(5, 6-dim)) {
//   //     cout << "D:\t4\t";
//   //     cout << "\tSINGULARITY-DIM:\t" << dim << "\tLVL:\t" << lvl << "\t" << calcCost<4>(dim,lvl) << endl;
//   //   }
//   // }
// }
//
// void sampleCalc() {
//   const int DIMS = 2;
//   Cell<DIMS> c;
//   CellDict<DIMS> D;
//   c.split();
//   c.getChild(0).gatherIn(D);
//   c.getChild(1).gatherIn(D);
//   cout << "SET: " <<D.size() << " " << D << "!" << endl;
//   cout << "EXAMPLE RES: " << calculateHalfDivCost(D) << endl;
// }
//

int main(int argc, char** argv) {
  const int DIM = 2;
  // FOR(i , 11) {
  //   FOR(j, 11) {
  //     cout << flopsFun(i, j) << '\t';
  //   }
  //   cout << endl;
  // }
  //
  // cout << "MAM:" << flopsFun(6, 9) << endl;
  // sampleCalc();
  // outputResultsForSingularities();
  //
  Cell<DIM> c;
  cerr << c.getId() << " <> BNDS:" << c.getBounds() << endl;
  CellDict<DIM> D;
  c.split();
  c.getChild(0).split();
  c.gatherIn(D);
  cerr << D << endl;
  
  for(auto cell: D) {
    if(!cell.second->isLeaf()) continue;
    cerr << endl << endl << "CELL: " << *(cell.second) << endl;
    cerr << getDependentNodes(cell.second->getId(), D) << endl;
  }
  
  auto str = prepareStruct(D);
  cout << str.dict << endl;
  
  cout << calculateNestedDissection(D) << endl;
 //
//   int lvl = 2;
//   int offset = 1<<lvl;
//   // FOR(i, offset) {
//   //   cerr << "STEP " << i << endl;
//   //   ensureSplit(D, CellId<DIM>({offset+i, offset+0, offset+0}));
//   // }
//   ensureSplit(D, CellId<DIM>({offset, offset}));
//
//
//   // auto nodes = calculateNodes(D);
//
//   // cerr << nodes << endl;
//   // printGrid(nodes, lvl+2);
//
//   cerr << "DIVINE " << calculateHalfDivCost(D) << endl;
//
//   // ensureSplit(D, target);
//   cerr << "####" << endl;
//   cerr << D;
//   cerr << endl;
//   cerr << "####" << endl;
//   c.print(cerr);
//   cerr << c.countLeaves() << ' ' << c.countCells() << ' ' << c.getMaxLevel() << endl;;
//   int maxLvl = c.getMaxLevel() + 1;
//   printGrid(D, lvl);
//
//   set<Hyperplane<DIM> > S;
//   map<CellId<DIM>, set<Hyperplane<DIM> > > cache;
//   c.gatherHyperplanes(S, c.getMaxLevel(), cache);
//   for(auto el : S) {
//     cerr << ")" << el << endl;
//   }
//   CellDict<DIM> D1;
//   D1.addCell(&c);
//
//   // pair<CellDict<DIM>, CellDict<DIM> > r = D1.divideByHyperplane(Hyperplane<DIM>(1, 0x15, 4));
// //   cerr << "1] " << r.first << endl;
// //   cerr << "2] " << r.second << endl;
// //
// //   r.first.complete();
// //   r.second.complete();
// //   cerr << "1] " << r.first << endl;
//   cerr << "2] " << r.second << endl;
//   printGrid(r.first, lvl);
//   printGrid(r.second, lvl);
  return 0;
}