#include "pifus_types.h"
#include "pifus.h"
#include <time.h>
using namespace PIFUS;

void pifus::registerGridData(int btag, int nnodes, double *xyz)
{
  int iblk;
  auto idxit=tag_iblk_map.find(btag);
  if (idxit== tag_iblk_map.end()) {
    mtags.push_back(btag);
    mblocks.push_back(std::unique_ptr<MeshBlock>(new MeshBlock));
#ifdef GPU
    dmblocks.push_back(std::unique_ptr<dMeshBlock>(new dMeshBlock));
#endif
    nblocks=mblocks.size();
    iblk=nblocks-1;
    tag_iblk_map[btag]=iblk;
  } else {
    iblk=idxit->second;
  
  } 
  auto &mb = mblocks[iblk];
  mb->setData(btag,nnodes,xyz);
#ifdef GPU
  auto &dmb= dmblocks[iblk];
  dmb->setData(btag,nnodes,xyz);
#endif
}
  
void pifus::registerSolution(int btag, int nvar, double *q)
{
  auto idxit=tag_iblk_map.find(btag);
  int iblk=idxit->second;
  auto &mb = mblocks[iblk];
  mb->setQ(nvar,q);
#ifdef GPU
  auto &dmb= dmblocks[iblk];
  dmb->setQ(nvar,q);
#endif
}

void pifus::registerTargets(int btag, int nvar, int ntargets, double *targetxyz,
			    double *targetq)
{
  auto idxit=tag_iblk_map.find(btag);
  int iblk=idxit->second;
  auto &mb = mblocks[iblk];
  //TRACEI(ntargets);
  mb->setTargets(nvar,ntargets,targetxyz,targetq);
#ifdef GPU
  auto &dmb = dmblocks[iblk];
  dmb->setTargets(nvar,ntargets,targetxyz,targetq);
#endif
}

void pifus::searchAndInterpolate_gpu(int nvar)
{
#ifdef GPU
  for (int ib=0;ib<nblocks;ib++)
    {
      auto &mb = mblocks[ib];
      auto &dmb= dmblocks[ib];
      // TODO
      myTimer("Preprocessing",0);
      mb->preprocess();   // ADT still created on Host
      myTimer("Preprocessing",1);
      //
      dmb->preprocess(mb->getADT());
      myTimer("Search",0);
      dmb->search();
      myTimer("Search",1);
      myTimer("Interpolate",0);
      dmb->interpolate(nvar);
      myTimer("Interpolate",1);
    }
#else
  printf("####  Error ######\n");
  printf("GPU mode not enabled\n");
  printf("set BUILD_GPU_CODE to on in CMakelists.txt\n");
  printf("####  Error ######\n");
#endif
}

void pifus::searchAndInterpolate(int nvar)
{
  for (int ib=0;ib<nblocks;ib++)
    {
      auto &mb = mblocks[ib];
      myTimer("Preprocessing",0);
      mb->preprocess();
      myTimer("Preprocessing",1);
      myTimer("Search",0);
      mb->search();
      myTimer("Search",1); 
      myTimer("Interpolate",0);
      mb->interpolate(nvar);
      myTimer("Interpolate",1);
    }
}

void pifus::myTimer(char const *info,int type)
{
if (use_timer) {
  static clock_t t_start;
  clock_t t_end;

  if (type==0) {
    t_start=clock();
    printf("Begin %s\n",info);
  }
  if (type==1) {
   t_end=clock();
   printf("End %s, time taken=%lf\n",info,((double)(t_end-t_start))/CLOCKS_PER_SEC);
  }
}
}


