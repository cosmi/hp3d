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
result_int calculateHalfDivCost(const CellDict<DIMS>& dict, const set<Supernode<DIMS> >& nodes) {
  // using Dict = CellDict<DIMS>;
  auto& bounds = dict.getBounds();
  auto hp = bounds.getHalvingPlane();  
  cerr << endl << "IN " << bounds << " DIV " << hp << endl;
  printGrid(dict, 5);
 cerr << "NODES " << nodes << endl;
  
  int external = 0;
  int reduced = 0;
  int internal = 0;
  
  
  if(dict.size() == 1) {
    for(auto& el : nodes) {

      if(bounds.isInternal(el.getId())) {
        // reduced here at this point
        reduced++;
      } else /* is on boundary */ {      
        int faces = bounds.countHyperplanes(el.getId());
        if(faces == el.getBoundsDim()) {
          // is on external boundary and reduced at this point
          reduced++;
        } else {
          // is on not external boundary and cannot be reduced here
          external++;
        }
      }
    }
    cerr <<"LEAF === " << " REDUCED:" << reduced << " EXTERNAL:" << external << endl;
  
    result_int res = flopsFun(reduced, reduced + external);
    cerr << "ELIMINATION-COST:" << res << endl;
    return res;
  }
  
  for(auto& el : nodes) {
    int cmp = el.getId().compareWithHyperplane(hp);
    if(bounds.isInternal(el.getId())) {
      if(cmp == 0) {
        // internal node that is reduced in this step
        reduced++;
      } else {
        // internal node reduced recursively
        internal++;
      }
    } else /* is on boundary */ {
      
      int faces = bounds.countHyperplanes(el.getId());
      if(faces == el.getBoundsDim()) {
        if(cmp == 0) {
          // is on external boundary and reduced here
          reduced++;
        } else {
          // is on external boundary and reduced recursively
          internal++;
        }
      } else {
        if(cmp == 0) {
          // is on not external boundary and cannot be reduced here
          external++;
        } else {
          // is on not external boundary and cannot be reduced here
          external++;
        }
      }
    }
  }
  
  cerr << "INTERNAL:" << internal << " REDUCED:" << reduced << " EXTERNAL:" << external << endl;
  
  result_int res = flopsFun(reduced, reduced + external);
  cerr << "ELIMINATION-COST:" << res << endl;
  
  auto dicts = dict.divideByHyperplane(hp);
  auto nds = divideByHyperplane(nodes, hp);
  
  // cerr << "DICTS " << dicts.first << " AND " << dicts.second << endl;
//   cerr << "NODES " << nds.first << " AND " << nds.second << endl;
//
  auto a = calculateHalfDivCost(dicts.first, nds.first);
  auto b = calculateHalfDivCost(dicts.second, nds.second);
  
  return res + a + b;
}


template <int DIMS>
result_int calculateHalfDivCost(const CellDict<DIMS>& dict) {
  
  auto nodes = calculateNodes(dict);
  
  cout << "NODES:\t" << nodes.size() ;
  // cerr << nodes << endl;
  printGrid(nodes, 6);
  return calculateHalfDivCost(dict, nodes); 

  
  
}


template <int DIMS>
void ensureSingularity(CellId<DIMS> id, CellDict<DIMS>& D, int singDims, int lvl, int dim) {
  if(dim == singDims) {
    cerr << "ENSURE " << id << endl;
    ensureSplit(D, id);
  } else {
    FOR(i, 1<<lvl) {
      ensureSingularity(id.getMovedId(dim, i), D, singDims, lvl, dim+1);
    }
  }
}

template <int DIMS>
result_int calcCost(int singDims, int lvl) {
   using Id = CellId<DIMS>;
   Cell<DIMS> c;
   CellDict<DIMS> D;
   c.gatherIn(D);

   Id id = c.getId();
   
   FOR(i, lvl) {
     id = id.getChildId(0);
   }
   ensureSingularity(id, D, singDims, lvl, 0);
   return calculateHalfDivCost(D);
}

void outputResultsForSingularities() {
  
  //
  // FOR(dim, 1) {
  //   FOR(lvl, 5) {
  //     cout << "D:\t1\t";
  //     cout << "\tSINGULARITY-DIM:\t" << dim << "\tLVL:\t" << lvl << "\t" << calcCost<1>(dim,lvl) << endl;
  //   }
  // }
  //
  cout << endl;
  FOR(dim, 2) {
    FOR(lvl, 7) {
      cout << "D:\t2\t";
      cout << "\tSINGULARITY-DIM:\t" << dim << "\tLVL:\t" << lvl << "\t" << calcCost<2>(dim,lvl) << endl;
    }
  }
   //
  // cout << endl;
  // FOR(dim, 4) {
  //   FOR(lvl, min(5, 7-dim)) {
  //     cout << "D:\t3\t";
  //     cout << "\tSINGULARITY-DIM:\t" << dim << "\tLVL:\t" << lvl << "\t" << calcCost<3>(dim,lvl) << endl;
  //   }
  // }
  //
  // cout << endl;
  // FOR(dim, 5) {
  //   FOR(lvl, min(5, 6-dim)) {
  //     cout << "D:\t4\t";
  //     cout << "\tSINGULARITY-DIM:\t" << dim << "\tLVL:\t" << lvl << "\t" << calcCost<4>(dim,lvl) << endl;
  //   }
  // }
}

void sampleCalc() {
  const int DIMS = 2;
  Cell<DIMS> c;
  CellDict<DIMS> D;
  c.split();
  c.getChild(0).gatherIn(D);
  c.getChild(1).gatherIn(D);
  cout << "SET: " <<D.size() << " " << D << "!" << endl;
  cout << "EXAMPLE RES: " << calculateHalfDivCost(D) << endl;
}


int main(int argc, char** argv) {
  // FOR(i , 11) {
  //   FOR(j, 11) {
  //     cout << flopsFun(i, j) << '\t';
  //   }
  //   cout << endl;
  // }
  //
  // cout << "MAM:" << flopsFun(6, 9) << endl;
  // sampleCalc();
  outputResultsForSingularities();
  //
  // Cell<DIM> c;
  // cerr << c.getId() << " <> BNDS:" << c.getBounds() << endl;
  
  
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