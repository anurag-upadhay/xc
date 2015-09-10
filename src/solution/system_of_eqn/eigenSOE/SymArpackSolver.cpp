//----------------------------------------------------------------------------
//  programa XC; cálculo mediante el método de los elementos finitos orientado
//  a la solución de problemas estructurales.
//
//  Copyright (C)  Luis Claudio Pérez Tato
//
//  El programa deriva del denominado OpenSees <http://opensees.berkeley.edu>
//  desarrollado por el «Pacific earthquake engineering research center».
//
//  Salvo las restricciones que puedan derivarse del copyright del
//  programa original (ver archivo copyright_opensees.txt) este
//  software es libre: usted puede redistribuirlo y/o modificarlo 
//  bajo los términos de la Licencia Pública General GNU publicada 
//  por la Fundación para el Software Libre, ya sea la versión 3 
//  de la Licencia, o (a su elección) cualquier versión posterior.
//
//  Este software se distribuye con la esperanza de que sea útil, pero 
//  SIN GARANTÍA ALGUNA; ni siquiera la garantía implícita
//  MERCANTIL o de APTITUD PARA UN PROPÓSITO DETERMINADO. 
//  Consulte los detalles de la Licencia Pública General GNU para obtener 
//  una información más detallada. 
//
// Debería haber recibido una copia de la Licencia Pública General GNU 
// junto a este programa. 
// En caso contrario, consulte <http://www.gnu.org/licenses/>.
//----------------------------------------------------------------------------
// File: ~/system_of_eqn/linearSOE/LawSolver/SymArpackSolver.C
//
// Written: Jun Peng
// Created: 2/1999
// Revision: A
//
// Description: This file contains the class definition for
// SymArpackSolver. It solves the XC::SymArpackSOE object by calling
// some "C" functions. The solver used here is generalized sparse
// solver. The user can choose three different ordering schema.
//
// What: "@(#) SymArpackSolver.C, revA"


#include "solution/system_of_eqn/eigenSOE/SymArpackSolver.h"
#include "solution/system_of_eqn/eigenSOE/SymArpackSOE.h"
#include <solution/analysis/model/AnalysisModel.h>
#include <solution/analysis/model/DOF_GrpIter.h>
#include <solution/analysis/model/dof_grp/DOF_Group.h>
#include <solution/analysis/model/FE_EleIter.h>
#include <solution/analysis/model/fe_ele/FE_Element.h>
#include <solution/analysis/integrator/Integrator.h>
#include "utility/matrix/Vector.h"

extern "C" {
  #include <solution/system_of_eqn/linearSOE/sparseSYM/FeStructs.h>
  #include <solution/system_of_eqn/linearSOE/sparseSYM/globalVars.h>
  #include <solution/system_of_eqn/linearSOE/sparseSYM/tim.h>
  #include "solution/system_of_eqn/linearSOE/sparseSYM/nmat.h"
}


extern "C" int dsaupd_(int *ido, char* bmat, int *n, char *which, int *nev,
                       double *tol, double *resid, int *ncv, double *v, int *ldv,
                       int *iparam, int *ipntr, double *workd, double *workl,
                       int *lworkl, int *info);

extern "C" int dseupd_(bool *rvec, char *howmny, long int *select, double *d, double *z,
                       int *ldz, double *sigma, char *bmat, int *n, char *which,
                       int *nev, double *tol, double *resid, int *ncv, double *v,
                       int *ldv, int *iparam, int *ipntr, double *workd,
                       double *workl, int *lworkl, int *info);

//! @brief Constructor.
XC::SymArpackSolver::SymArpackSolver(int numE)
 :EigenSolver(EigenSOLVER_TAGS_SymArpackSolver, numE),
theSOE(nullptr), factored(false)
  {
    // nothing to do.
  }

//! @brief Resuelve el problema de autovalores.
int XC::SymArpackSolver::solve(void)
  {
    if(!theSOE)
      {
        std::cerr << "WARNING XC::SymArpackSolver::solve(void)- ";
        std::cerr << " No XC::EigenSOE object has been set\n";
        return -1;
      }

    int      nblks = theSOE->nblks;
    int      *xblk = theSOE->xblk;
    //    int      *invp = theSOE->invp;
    double   *diag = theSOE->diag;
    double   **penv = theSOE->penv;
    int      *rowblks = theSOE->rowblks;
    OFFDBLK  **begblk = theSOE->begblk;
    OFFDBLK  *first = theSOE->first;

    int n = theSOE->size;

    // check for XC::quick return
    if(n == 0)
      return 0;

//        timer (FACTOR);
    if(factored == false)
      {
        //factor the matrix
        //call the "C" function to do the numerical factorization.
        int factor;
        factor = pfsfct(n, diag, penv, nblks, xblk, begblk, first, rowblks);
        if(factor>0)
          {
            std::cerr << "In XC::SymArpackSolver: error in factorization.\n";
            return -1;
          }
        factored = true;
      }

    int nev = numModes;
    int ncv = getNCV(n, nev);

    // set up the space for ARPACK functions.
    int ldv = n;

    Vector v(ldv * ncv);
    std::vector<double> workl(ncv * (ncv+8));
    std::vector<double> workd(3 * n);
    Vector d(ncv * 2);
    std::vector<double> resid(n);

    char bmat = 'G';

    int iparam[11];
    //int iwork[n];

    int lworkl = ncv*ncv + 8*ncv;
    int maxitr, mode; //, nconv;

    double tol = 0.0;
    int info = 0;
    maxitr = 1000;
    mode = 3;

    iparam[2] = maxitr;
    iparam[6] = mode;

    bool rvec = true;
    char howmy = 'A';
    std::vector<long int> select(ncv);

    iparam[0] = 1;
    int ido = 0;
    int ipntr[11];
    //int ierr = 0;
    //unsigned int sizeWhich =2;
    //unsigned int sizeBmat =1;
    //unsigned int sizeHowmany =1;
    char eleeme[] = "LM"; 
    while(1)
      {
        dsaupd_(&ido, &bmat, &n, eleeme, &nev, &tol, &resid[0], &ncv, v.getDataPtr(), &ldv,
                iparam, ipntr, &workd[0], &workl[0], &lworkl, &info);
        if(ido == -1)
          {
            myMv(n, &workd[ipntr[0]-1], &workd[ipntr[1]-1]);
            pfsslv(n, diag, penv, nblks, xblk, &workd[ipntr[1] - 1], begblk);
            continue;
          }
        else if(ido == 1)
          {
            //double ratio = 1.0;
            myCopy(n, &workd[ipntr[2]-1], &workd[ipntr[1]-1]);
            pfsslv(n, diag, penv, nblks, xblk, &workd[ipntr[1] - 1], begblk);
            continue;
          }
        else if(ido == 2)
          {
            myMv(n, &workd[ipntr[0]-1], &workd[ipntr[1]-1]);
            continue;
          }
        break;
      }

    if(info < 0)
      {
        std::cerr << "BandArpackSolver::Error with _saupd info = " << info <<std::endl;
        return info;
      }
    else
      {
        if(info == 1)
         { std::cerr << "BandArpackSolver::Maximum number of iteration reached." << std::endl; }
        else if(info == 3)
          {
            std::cerr << "XC::BandArpackSolver::No Shifts could be applied during implicit," << std::endl;
            std::cerr << "Arnoldi update, try increasing NCV." << std::endl;
          }
        double sigma = theSOE->shift;
        if(iparam[4] > 0)
          {
            dseupd_(&rvec, &howmy, &select[0], d.getDataPtr(), v.getDataPtr(), &ldv, &sigma, &bmat, &n, eleeme,
                    &nev, &tol, &resid[0], &ncv, v.getDataPtr(), &ldv, iparam, ipntr, &workd[0],
                    &workl[0], &lworkl, &info);
            if(info != 0)
              {
                std::cerr << "BandArpackSolver::Error with dseupd_" << info;
                return -1;
              }
          }
      }
    value= d;
    vector= v;

    theSOE->factored = true;

    return 0;
  }

//! @brief Asigna el tamaño del problema.
int XC::SymArpackSolver::setSize(void)
  {
    // nothing to do
    // if iPiv not big enough, free it and get one large enough
    int size = theSOE->size;

    if(eigenV.Size() != size)
      eigenV.resize(size);

    return 0;
  }

//! @brief Devuelve la dimensión de los autovectores.
const int &XC::SymArpackSolver::getSize(void) const
  { return theSOE->size; }

//! @brief Asigna el problema de autovalores a resolver.
bool XC::SymArpackSolver::setEigenSOE(EigenSOE *soe)
  {
    bool retval= false;
    SymArpackSOE *tmp= dynamic_cast<SymArpackSOE *>(soe);
    if(tmp)
      {
        theSOE= tmp;
        retval= true;
      }
    else
      std::cerr << nombre_clase() << "::setEigenSOE: el sistema de ecuaciones no es del tipo adecuado para este solver." << std::endl;
    return retval;
  }

//! @brief Asigna el problema a resolver.
bool XC::SymArpackSolver::setEigenSOE(SymArpackSOE &theEigenSOE)
  { return setEigenSOE(&theEigenSOE); }


//! @brief ??
int XC::SymArpackSolver::getNCV(int n, int nev)
  {
    int result;
    if(2*nev > nev+8)
      { result = nev+8; }
    else
      { result = 2*nev; }

    if(result >= n)
      { result = n; }
    return result;
  }


void XC::SymArpackSolver::myMv(int n, double *v, double *result)
  {
    std::vector<double> tempX(n);
    int *invp = theSOE->invp;
    int i;

    for(i=0; i<n; i++)
      { tempX[i] = v[invp[i]]; }
    for(i=0; i<n; i++)
      { v[i] = tempX[i]; }

    Vector x(v, n);
    Vector y(result,n);

    y.Zero();
    AnalysisModel *theAnalysisModel = theSOE->getAnalysisModelPtr();

    // loop over the FE_Elements
    FE_Element *elePtr;
    FE_EleIter &theEles = theAnalysisModel->getFEs();
    while((elePtr = theEles()) != 0)
      {
        const Vector &a = elePtr->getM_Force(x,1.0);
        y.Assemble(a,elePtr->getID(),1.0);
      }

    // loop over the DOF_Groups
    //Integrator *theIntegrator = 0;
    DOF_Group *dofGroupPtr= nullptr;
    DOF_GrpIter &theDofs = theAnalysisModel->getDOFGroups();
    while ((dofGroupPtr= theDofs()) != 0)
      {
        const Vector &a = dofGroupPtr->getM_Force(x,1.0);
        y.Assemble(a,dofGroupPtr->getID(),1.0);
      }

    for(i=0; i<n; i++)
      { tempX[invp[i]] = result[i]; }
    for(i=0; i<n; i++)
      { result[i] = tempX[i]; }
  }

void XC::SymArpackSolver::myCopy(int n, double *v, double *result)
  {
    for(int i=0; i<n; i++)
      { result[i] = v[i]; }
  }

//! @brief Devuelve el autovector correspondiente al modo que se pasa como parámetro.
const XC::Vector &XC::SymArpackSolver::getEigenvector(int mode) const
  {
    int *invp = theSOE->invp;

    if(mode <= 0 || mode > numModes)
      {
        std::cerr << "BandArpackSOE::mode is out of range(1 - nev)";
        exit (0);
      }
    int size = theSOE->size;

    int index= (mode - 1) * size;
    for(int i=0; i<size; i++)
      { eigenV(i) = vector[index + invp[i]]; }
    return eigenV;
  }

//! @brief Devuelve el autovalor correspondiente al modo que se pasa como parámetro.
const double &XC::SymArpackSolver::getEigenvalue(int mode) const
  {
    if(mode <= 0 || mode > numModes)
      {
        std::cerr << "BandArpackSOE::mode is out of range(1 - nev)";
        exit (0);
      }
    return value[mode - 1];
  }


int XC::SymArpackSolver::sendSelf(CommParameters &cp)
  {
    // nothing to do
    return 0;
  }

int XC::SymArpackSolver::recvSelf(const CommParameters &cp)
  {
    // nothing to do
    return 0;
  }


