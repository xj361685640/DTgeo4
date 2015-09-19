#ifndef _PARAMS_H
#define _PARAMS_H
#include <stdio.h>
#include "cuda_math.h"
#include <assert.h>
#include <vector>
#include <string>
#define STATIC_ASSERT( x ) typedef char __STATIC_ASSERT__[( x )?1:-1]

#define USE_UVM 2

#ifndef USE_UVM
#define USE_UVM 0
#endif

#ifdef USE_DOUBLE
typedef double ftype;
#define MPI_FTYPE MPI_DOUBLE
#define FTYPESIZE 8
typedef double2 ftype2;
typedef double3 ftype3;
typedef double4 ftype4;
template<typename T1,typename T2> __host__ __device__ ftype2 make_ftype2(const T1& f1, const T2& f2) { return make_double2(f1,f2); }
#else//use float
typedef float ftype;
#define MPI_FTYPE MPI_FLOAT
#define FTYPESIZE 4
typedef float2 ftype2;
typedef float3 ftype3;
typedef float4 ftype4;
template<typename T1,typename T2> __host__ __device__ ftype2 make_ftype2(const T1& f1, const T2& f2) { return make_float2(f1,f2); }
#endif

#include "py_consts.h"

#if defined MPI_ON && defined TEST_RATE
#error MPI_ON and TEST_RATE are defined simultaneously
#endif

#ifndef NA
#define NA (gridNz/3)
#endif
#ifndef NV
#define NV (gridNy)
#endif
#ifndef NTIME
#define NTIME (NS-3)
#endif
#ifndef DYSH
#define DYSH
#endif

const int Ns=NS;
const int Na=NA;
const int Nv=NV;

const int Ntime=NTIME;

#if NDev==1
#define Nstrp0 (NA)
#define STRIPES {Nstrp0,}
#elif NDev==2
#define Nstrp0 (NA/2 DYSH)
#define STRIPES {Nstrp0,NA-Nstrp0}
#else
//#define Nstrp0 (NA/3 )
#define STRIPES {NA/3,NA/3,NA-NA/3-NA/3}
#endif

//static_assert(NX>=WX, "Error: NX<=Window size");  // c++11
#if defined USE_AIVLIB_MODEL && defined COFFS_DEFAULT
#error AIVLIB_MODEL and COFFS_DEFAULT are defined simultaneously
#endif

#ifdef MPI_ON
#include <mpi.h>
#endif

#define NStripe(i) (devNStripe[i])
const int NStripe[NDev] = STRIPES;
extern __constant__ int devNStripe[NDev];
const int Nx=((USE_UVM==2)?Np:Ns)*NDT+2;
const int Ny=Na*NDT+1;
//const int Ny=Nstrp0*NDT+1;
const int Nz=Nv;
const int NT=Nz;
const int Nover=0;
extern int* mapNodeSize;

const int KNpmlz=2*Npmlz;//128;
const int KNpmly=2*Npmly*NDT;//128;
const int KNpmlx=2*Npmlx*NDT;//128;

#define SquareGrid 0
const ftype dr=5*5./512.;
//const ftype dx=dr, dy=dr, dz=dr, dt=1*5./512.;
const ftype dx=ds, dy=da, dz=dv;
const ftype dtdx=dt/dx, dtdy=dt/dy, dtdz=dt/dz;
//STATIC_ASSERT((dx==dy) && (SquareGrid==1));
//STATIC_ASSERT((dx==dz) && (SquareGrid==1));

#define DEBUG_PRINT(debug) ;//printf debug;
#define DEBUG_MPI(debug)   ;//printf debug;
#define CHECK_ERROR(err) CheckError( err, __FILE__,__LINE__)
static void CheckError( cudaError_t err, const char *file, int line) {
  if(err!=cudaSuccess){
    fprintf(stderr, "%s in %s at line %d\n", cudaGetErrorString(err), file, line);
    exit(EXIT_FAILURE);
  }
}

class cuTimer {
  cudaEvent_t tstart,tend;
  cudaStream_t st;
  float diftime;
  public:
  cuTimer(const cudaStream_t& stream=0): diftime(0) {
    CHECK_ERROR( cudaEventCreate(&tstart) ); 
    CHECK_ERROR( cudaEventCreate(&tend  ) );
    CHECK_ERROR( cudaEventRecord(tstart,stream) ); st=stream;
  }
  ~cuTimer(){
    CHECK_ERROR( cudaEventDestroy(tstart) );
    CHECK_ERROR( cudaEventDestroy(tend) );
  }
  float gettime(){
    CHECK_ERROR( cudaEventRecord(tend,st) );
    CHECK_ERROR( cudaEventSynchronize(tend) );
    CHECK_ERROR( cudaEventElapsedTime(&diftime, tstart,tend) ); 
    return diftime;
  }
};

#include "im2D.h"
#include "im3D.hpp"
extern image2D im2D;
extern float calcTime, calcPerf; extern int TimeStep;
extern bool recalc_at_once;
extern char* FuncStr[];

struct DmdArraySe{
  ftype  fld[26][Nz];
  ftype2 fldPML[9][Npmlz];
  ftype2 fldPML1[4][Npmlz]; ftype fldPML2[4][Npmlz];
//  static const int2 SelfShift = {0,0};
//  static const int3 DataShift = DATAShifts;;
};
struct DmdArraySo{
  ftype  fld[28][Nz];
  ftype2 fldPML[9][Npmlz];
  ftype2 fldPML1[5][Npmlz]; ftype fldPML2[5][Npmlz];
//  static const int2 SelfShift = {-NDT,NDT};
//  static const int3 DataShift = DATAShifts;;
};
struct DmdArrayVe{
  ftype  fld[13][Nz]; ftype2 fldPML[13][Npmlz];
//  static const int2 SelfShift = {0,NDT};
//  static const int3 DataShift = DATAShifts;;
};
struct DmdArrayVo{
  ftype  fld[14][Nz]; ftype2 fldPML[14][Npmlz];
//  static const int2 SelfShift = {-NDT,0};
//  static const int3 DataShift = DATAShifts;;
};
struct RagArrayQuat{
  DmdArraySe dmdSe; 
  DmdArraySo dmdSo;
  DmdArrayVo dmdVo;
  DmdArrayVe dmdVe;
};
struct RagArray{
  ftype  fld[81][Nz];
  ftype2 fldPML[45][Npmlz];
  ftype2 fldPML1[9][Npmlz]; ftype fldPML2[9][Npmlz];
};
struct TwoDomS {
    //ftype fld[6][Nz];
    ftype2 duofld[3][Nz];
  ftype fldPML[5][Npmlz];
  //ftype2 fldPML1[Npmlz]; ftype fldPML2[Npmlz];
};
struct TwoDomV {
    //ftype fld[3][Nz];
    struct {
      ftype  one[Nz];
      ftype2 two[Nz];
    } trifld;
  ftype fldPML[3][Npmlz];
};
struct DiamondRag{
  TwoDomS Si[NDT*NDT];
  TwoDomV Vi[NDT*NDT];
  inline static void copyM(const int idev, int ixrag, cudaStream_t& stream);
  inline static void copyP(const int idev, int ixrag, cudaStream_t& stream);
  inline static void SendMPIm(const int node, int ixrag);
  inline static void SendMPIp(const int node, int ixrag);
  template<const int Did> inline static void copyDiamond(DiamondRag* dstRag, DiamondRag* srcRag, cudaStream_t& stream) {
    if(Did==0) CHECK_ERROR( cudaMemcpyAsync(&dstRag->Si[0        ].duofld[0], &srcRag->Si[0        ].duofld[0], sizeof(TwoDomS)*(NDT*NDT/2+1), cudaMemcpyDeviceToDevice, stream) );
    if(Did==1) CHECK_ERROR( cudaMemcpyAsync(&dstRag->Si[NDT*NDT/2].duofld[0], &srcRag->Si[NDT*NDT/2].duofld[0], sizeof(TwoDomS)*(NDT*NDT/2+1), cudaMemcpyDeviceToDevice, stream) );
    if(Did==2) CHECK_ERROR( cudaMemcpyAsync(&dstRag->Vi[0        ].trifld   , &srcRag->Vi[0        ].trifld   , sizeof(TwoDomV)*(NDT*NDT/2+1), cudaMemcpyDeviceToDevice, stream) );
    if(Did==3) CHECK_ERROR( cudaMemcpyAsync(&dstRag->Vi[NDT*NDT/2].trifld   , &srcRag->Vi[NDT*NDT/2].trifld   , sizeof(TwoDomV)*(NDT*NDT/2+1), cudaMemcpyDeviceToDevice, stream) );
/*    if(Did==0) CHECK_ERROR( cudaMemcpyAsync(&dstRag->Si[0        ].fld[0], &srcRag->Si[0        ].fld[0], sizeof(TwoDomS)*(NDT*NDT/2  )+sizeof(ftype)*4*Nz, cudaMemcpyDeviceToDevice, stream) );
    if(Did==1) CHECK_ERROR( cudaMemcpyAsync(&dstRag->Si[NDT*NDT/2].fld[4], &srcRag->Si[NDT*NDT/2].fld[4], sizeof(TwoDomS)*(NDT*NDT/2+1)-sizeof(ftype)*4*Nz, cudaMemcpyDeviceToDevice, stream) );
    if(Did==2) CHECK_ERROR( cudaMemcpyAsync(&dstRag->Vi[0        ].fld[0], &srcRag->Vi[0        ].fld[0], sizeof(TwoDomV)*(NDT*NDT/2  )+sizeof(ftype)*1*Nz, cudaMemcpyDeviceToDevice, stream) );
    if(Did==3) CHECK_ERROR( cudaMemcpyAsync(&dstRag->Vi[NDT*NDT/2].fld[1], &srcRag->Vi[NDT*NDT/2].fld[1], sizeof(TwoDomV)*(NDT*NDT/2+1)-sizeof(ftype)*1*Nz, cudaMemcpyDeviceToDevice, stream) );
*/  }
  template<const int Did> inline static void MPIsendDiamond(DiamondRag* dstRag, DiamondRag* srcRag, const int dstnode, const int srcnode) {
    #ifdef MPI_ON
    if(dstnode>=0 && dstnode<NasyncNodes-1) {
      if(Did==0) MPI_Send(&srcRag->Si[0        ].duofld[0], sizeof(TwoDomS)*(NDT*NDT/2+1)/sizeof(ftype), MPI_FTYPE, dstnode, 0, MPI_COMM_WORLD );
      if(Did==1) MPI_Send(&srcRag->Si[NDT*NDT/2].duofld[0], sizeof(TwoDomS)*(NDT*NDT/2+1)/sizeof(ftype), MPI_FTYPE, dstnode, 0, MPI_COMM_WORLD );
      if(Did==2) MPI_Send(&srcRag->Vi[0        ].trifld   , sizeof(TwoDomV)*(NDT*NDT/2+1)/sizeof(ftype), MPI_FTYPE, dstnode, 0, MPI_COMM_WORLD );
      if(Did==3) MPI_Send(&srcRag->Vi[NDT*NDT/2].trifld   , sizeof(TwoDomV)*(NDT*NDT/2+1)/sizeof(ftype), MPI_FTYPE, dstnode, 0, MPI_COMM_WORLD );
    }
    const int fromnode = srcnode+srcnode-dstnode;
    if(fromnode>=0 && fromnode<NasyncNodes-1) {
      MPI_Status stat;
      if(Did==0) MPI_Recv(&dstRag->Si[0        ].duofld[0], sizeof(TwoDomS)*(NDT*NDT/2+1)/sizeof(ftype), MPI_FTYPE, fromnode, 0, MPI_COMM_WORLD, &stat );
      if(Did==1) MPI_Recv(&dstRag->Si[NDT*NDT/2].duofld[0], sizeof(TwoDomS)*(NDT*NDT/2+1)/sizeof(ftype), MPI_FTYPE, fromnode, 0, MPI_COMM_WORLD, &stat );
      if(Did==2) MPI_Recv(&dstRag->Vi[0        ].trifld   , sizeof(TwoDomV)*(NDT*NDT/2+1)/sizeof(ftype), MPI_FTYPE, fromnode, 0, MPI_COMM_WORLD, &stat );
      if(Did==3) MPI_Recv(&dstRag->Vi[NDT*NDT/2].trifld   , sizeof(TwoDomV)*(NDT*NDT/2+1)/sizeof(ftype), MPI_FTYPE, fromnode, 0, MPI_COMM_WORLD, &stat );
    }
    #endif
  }
};
struct TwoDomSpml { ftype3 fld[6][Nz]; };
struct TwoDomVpml { ftype3 fld[3][Nz]; };
struct DiamondRagPML{
  TwoDomSpml Si[NDT*NDT];
  TwoDomVpml Vi[NDT*NDT];
};

const int tfsfH=1000*Ny;

struct Sensor;
#include "drop.cu"
#include "texmodel.cuh"
struct GeoParams{
  unsigned int nFunc, bgMat;
  int iStep; int wleft; int GPUx0;

  float Rz;
  float Ycnt,Zcnt;

  ModelRag* ragsInd[NDev];
  DiamondRag* rags[NDev];
  DiamondRagPML* ragsPMLa; 
  DiamondRagPML* ragsPMLsL[NDev]; 
  DiamondRagPML* ragsPMLsR[NDev]; 
  DiamondRag* data; 
  ModelRag* dataInd;
  DiamondRagPML* dataPMLa, *dataPMLs, *dataPMLsL, *dataPMLsR;
  ModelTexs texs;
  SeismoDrops drop;

  std::vector<Sensor>* sensors;
// Think about members!!!  WTF??

  __device__ DiamondRag& get_plaster(const int ix, const int iy) { 
    #ifdef USE_WINDOW
    return data[ix*Na+iy];
    #else
  // only for two devices;
    int idev = (iy<NStripe(0))?0:1; int iym= (idev==0)?0:NStripe(0);
    return rags[idev][ix*NStripe(idev)+iy-iym];
    #endif
  }
  __device__ ModelRag& get_index(const int ix, const int iy) { 
    return dataInd[ix*Na+iy];
  }

  bool vb;

};
const int MaxBlk=15;
extern struct GeoParamsHost: public GeoParams {
  Arr3D_pars arr4im;
  unsigned int MaxFunc;
  static const int Nstreams=10+Na/MaxBlk;
  bool isTFSF;
  std::string* dir;
  void set();
  void reset_im() {
    arr4im.reset(Nz,Ny,Nx);
    arr4im.BufSize = sizeof(float)*Nz*Ny*Nx;
    CHECK_ERROR( cudaMalloc((void**) (&arr4im.Arr3Dbuf), arr4im.BufSize) );
    CHECK_ERROR( cudaMemset(arr4im.Arr3Dbuf, 0, arr4im.BufSize) );
    arr4im.inGPUmem = true;
    //memcpy(&im3DHost, &arr4im, sizeof(im3DHost));
  }
  void clear() {
    cudaFree (rags);
    cudaFree (arr4im.Arr3Dbuf);
    arr4im.clear();
//    CHECK_ERROR(cudaFreeArray(eps_texArray));
  }
  void checkSizes() { }
} parsHost;
extern __constant__ GeoParams pars;

inline void DiamondRag::copyM(const int idev, int ixrag, cudaStream_t& stream){ //diamonds 0 and 3
  copyDiamond<0>( &parsHost.rags[idev-1][(ixrag%Ns+1)*NStripe[idev-1]-1], &parsHost.rags[idev  ][ ixrag%Ns   *NStripe[idev  ]  ], stream );
  copyDiamond<3>( &parsHost.rags[idev-1][(ixrag%Ns+1)*NStripe[idev-1]-1], &parsHost.rags[idev  ][ ixrag%Ns   *NStripe[idev  ]  ], stream );
}
inline void DiamondRag::copyP(const int idev, int ixrag, cudaStream_t& stream){ //diamonds 1 and 2
  copyDiamond<1>( &parsHost.rags[idev+1][ ixrag%Ns   *NStripe[idev+1]  ], &parsHost.rags[idev  ][(ixrag%Ns+1)*NStripe[idev  ]-1], stream );
  ixrag++;
  copyDiamond<2>( &parsHost.rags[idev+1][ ixrag%Ns   *NStripe[idev+1]  ], &parsHost.rags[idev  ][(ixrag%Ns+1)*NStripe[idev  ]-1], stream );
}
inline void DiamondRag::SendMPIm(const int node, int ixrag){ //diamonds 0 and 3
  MPIsendDiamond<0>( &parsHost.rags[NDev-1][(ixrag%Ns+1)*NStripe[NDev-1]-1], &parsHost.rags[0][ ixrag%Ns   *NStripe[0]  ], node-1, node);
  MPIsendDiamond<3>( &parsHost.rags[NDev-1][(ixrag%Ns+1)*NStripe[NDev-1]-1], &parsHost.rags[0][ ixrag%Ns   *NStripe[0]  ], node-1, node);
}
inline void DiamondRag::SendMPIp(const int node, int ixrag){ //diamonds 1 and 2
  MPIsendDiamond<1>( &parsHost.rags[0][ ixrag%Ns   *NStripe[0]  ], &parsHost.rags[NDev-1][(ixrag%Ns+1)*NStripe[NDev-1]-1], node+1, node );
  ixrag++;
  MPIsendDiamond<2>( &parsHost.rags[0][ ixrag%Ns   *NStripe[0]  ], &parsHost.rags[NDev-1][(ixrag%Ns+1)*NStripe[NDev-1]-1], node+1, node );
}
/*  CHECK_ERROR( cudaMemcpyAsync(&parsHost.rags[idev-1][(ixrag%Ns+1)*NStripe[idev-1]-1].Si[0].fld[0], 
                               &parsHost.rags[idev  ][ ixrag%Ns   *NStripe[idev  ]  ].Si[0].fld[0],
                               sizeof(TwoDomS)*(NDT*NDT/2  )+sizeof(ftype)*4*Nz, cudaMemcpyDeviceToDevice, stream) );
  CHECK_ERROR( cudaMemcpyAsync(&parsHost.rags[idev-1][(ixrag%Ns+1)*NStripe[idev-1]-1].Vi[NDT*NDT/2].fld[1], 
                               &parsHost.rags[idev  ][ ixrag%Ns   *NStripe[idev  ]  ].Vi[NDT*NDT/2].fld[1],
                               sizeof(TwoDomV)*(NDT*NDT/2+1)-sizeof(ftype)*1*Nz, cudaMemcpyDeviceToDevice, stream) );
  CHECK_ERROR( cudaMemcpyAsync(&parsHost.rags[idev+1][ ixrag%Ns   *NStripe[idev+1]  ].Si[NDT*NDT/2].fld[4], 
                               &parsHost.rags[idev  ][(ixrag%Ns+1)*NStripe[idev  ]-1].Si[NDT*NDT/2].fld[4],
                               sizeof(TwoDomS)*(NDT*NDT/2+1)-sizeof(ftype)*4*Nz, cudaMemcpyDeviceToDevice, stDo[idev]) );
                               ixrag++;
  CHECK_ERROR( cudaMemcpyAsync(&parsHost.rags[idev+1][ ixrag%Ns   *NStripe[idev+1]  ].Vi[0].fld[0], 
                               &parsHost.rags[idev  ][(ixrag%Ns+1)*NStripe[idev  ]-1].Vi[0].fld[0],
                               sizeof(TwoDomV)*(NDT*NDT/2  )+sizeof(ftype)*1*Nz, cudaMemcpyDeviceToDevice, stDo[idev]) );
*/

#include "sensor.h"

#include "drop_func.cu"
#include "defs.h"

int _main(int argc, char** argv);
extern int Tsteps;

#endif //_PARAMS_H
