//----------------------------------------------------------------------------
//  XC program; finite element analysis code
//  for structural analysis and design.
//
//  Copyright (C)  Luis Claudio Pérez Tato
//
//  This program derives from OpenSees <http://opensees.berkeley.edu>
//  developed by the  «Pacific earthquake engineering research center».
//
//  Except for the restrictions that may arise from the copyright
//  of the original program (see copyright_opensees.txt)
//  XC is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This software is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//
// You should have received a copy of the GNU General Public License
// along with this program.
// If not, see <http://www.gnu.org/licenses/>.
//----------------------------------------------------------------------------
//===============================================================================
//# COPYRIGHT (C): Woody's license (by BJ):
//                 ``This    source  code is Copyrighted in
//                 U.S.,  for  an  indefinite  period,  and anybody
//                 caught  using it without our permission, will be
//                 mighty good friends of ourn, cause we don't give
//                 a  darn.  Hack it. Compile it. Debug it. Run it.
//                 Yodel  it.  Enjoy it. We wrote it, that's all we
//                 wanted to do.''
//
//# PROJECT:           Object Oriented Finite XC::Element Program
//# PURPOSE:           Finite Deformation Hyper-Elastic classes
//# CLASS:
//#
//# VERSION:           0.6_(1803398874989) (golden section)
//# LANGUAGE:          C++
//# TARGET OS:         all...
//# DESIGN:            Zhao Cheng, Boris Jeremic (jeremic@ucdavis.edu)
//# PROGRAMMER(S):     Zhao Cheng, Boris Jeremic
//#
//#
//# DATE:              July 2004
//# UPDATE HISTORY:    July25th2004, Zhao added the Algorithmic Tangent Stiffness
//#                                     for nearly quadratic convergence rate
//===============================================================================

#ifndef FiniteDeformationEP3D_CPP
#define FiniteDefornationEP3D_CPP

#include "material/nD/FiniteDeformation/FDEPState.h"
#include "material/nD/FiniteDeformation/FiniteDeformationEP3D.h"
#include <material/nD/FiniteDeformation/fdYield/fdYield.h>
#include <material/nD/FiniteDeformation/fdFlow/fdFlow.h>
#include <material/nD/FiniteDeformation/fdEvolution/fdEvolution_S.h>
#include <material/nD/FiniteDeformation/fdEvolution/fdEvolution_T.h>
#include "utility/matrix/nDarray/BJmatrix.h"
#include <material/nD/NDMaterialType.h>

const int    Max_Iter  = 40;
const double tolerance = 1.0e-8;

XC::BJtensor tensorZ2(2, def_dim_2, 0.0);
XC::BJtensor tensorI2("I", 2, def_dim_2);
XC::BJtensor tensorZ4(4, def_dim_4, 0.0);

//! @brief Constructor 00
XC::FiniteDeformationEP3D::FiniteDeformationEP3D(int tag)
  : XC::NDMaterial(tag,ND_TAG_FiniteDeformationEP3D)
  {
    fde3d = 0;
    fdy = 0;
    fdf = 0;
    fdEvolutionS = 0;
    fdEvolutionT = 0;
    fdeps = 0;

        int err;
    err = this->revertToStart();
  }

//! @brief Constructor 01
XC::FiniteDeformationEP3D::FiniteDeformationEP3D(int tag,
                                         XC::NDMaterial *fde3d_in,
                                         fdYield *fdy_in,
                                         fdFlow *fdf_in,
                                         fdEvolution_S *fdEvolutionS_in,
                                         fdEvolution_T *fdEvolutionT_in)
  :XC::NDMaterial(tag, ND_TAG_FiniteDeformationEP3D)
  {
    if(fde3d_in)
      fde3d = fde3d_in->getCopy();
    else
      {
        std::cerr << getClassName() << "::" << __FUNCTION__
                  << "; failed to construct the fdElastic3D\n";
        exit(-1);
      }

    if(fdy_in)
      fdy = fdy_in->newObj();
    else
      {
        std::cerr << getClassName() << "::" << __FUNCTION__
                  << "; failed to construct the fdYield.\n";
        exit(-1);
      }

    if(fdf_in)
      fdf = fdf_in->newObj();
    else {
      std::cerr << getClassName() << "::" << __FUNCTION__
                << "; failed to construct the fdFlow\n";
      exit(-1);
    }

    if(fdEvolutionS_in)
      fdEvolutionS = fdEvolutionS_in->newObj();
    else
      fdEvolutionS = 0;

    if(fdEvolutionT_in)
      fdEvolutionT = fdEvolutionT_in->newObj();
    else
      fdEvolutionT = 0;

    //if(fdeps == 0)
    fdeps = new FDEPState();

}

//! @brief Constructor 02
XC::FiniteDeformationEP3D::FiniteDeformationEP3D(int tag,
                                         XC::NDMaterial *fde3d_in,
                                         fdYield *fdy_in,
                                         fdFlow *fdf_in,
                                         fdEvolution_S *fdEvolutionS_in)
:XC::NDMaterial(tag, ND_TAG_FiniteDeformationEP3D)
{
    if(fde3d_in)
      fde3d = fde3d_in->getCopy();
    else {
      std::cerr << getClassName() << "::" << __FUNCTION__
                << "; failed to construct the fdElastic3D\n";
      exit(-1);
    }

    if(fdy_in)
      fdy = fdy_in->newObj();
    else {
      std::cerr << getClassName() << "::" << __FUNCTION__
                << "; failed to construct the fdYield.\n";
      exit(-1);
    }

    if(fdf_in)
      fdf = fdf_in->newObj();
    else {
      std::cerr << getClassName() << "::" << __FUNCTION__
                << "; failed to construct the fdFlow.\n";
      exit(-1);
    }

    if(fdEvolutionS_in)
      fdEvolutionS = fdEvolutionS_in->newObj();
    else
      fdEvolutionS = 0;

    fdEvolutionT = 0;

    //if(fdeps == 0)
    fdeps = new FDEPState();

}

//! @brief Constructor 03
XC::FiniteDeformationEP3D::FiniteDeformationEP3D(int tag,
                                         NDMaterial *fde3d_in,
                                         fdYield *fdy_in,
                                         fdFlow *fdf_in,
                                         fdEvolution_T *fdEvolutionT_in)
:XC::NDMaterial(tag, ND_TAG_FiniteDeformationEP3D)
{
    if(fde3d_in)
      fde3d = fde3d_in->getCopy();
    else
      {
        std::cerr << getClassName() << "::" << __FUNCTION__
                << "; failed to construct the fdElastic3D.\n";
        exit(-1);
      }

    if(fdy_in)
      fdy = fdy_in->newObj();
    else
      {
        std::cerr << getClassName() << "::" << __FUNCTION__
                << "; failed to construct the fdYield\n";
        exit(-1);
      }

    if(fdf_in)
      fdf = fdf_in->newObj();
    else
      {
        std::cerr << getClassName() << "::" << __FUNCTION__
                << "; failed to construct the fdFlow\n";
        exit(-1);
      }

    fdEvolutionS = 0;

    if(fdEvolutionT_in)
      fdEvolutionT = fdEvolutionT_in->newObj();
    else
      fdEvolutionT = 0;

    //if(fdeps == 0)
    fdeps = new FDEPState();

  }

//! @brief Constructor 04
XC::FiniteDeformationEP3D::FiniteDeformationEP3D(int tag,
                                         NDMaterial *fde3d_in,
                                         fdYield *fdy_in,
                                         fdFlow *fdf_in)
  :XC::NDMaterial(tag, ND_TAG_FiniteDeformationEP3D)
  {
    if(fde3d_in)
      fde3d = fde3d_in->getCopy();
    else
      {
        std::cerr << getClassName() << "::" << __FUNCTION__
                << "; failed to construct the fdElastic3D\n";
        exit(-1);
      }

    if(fdy_in)
      fdy = fdy_in->newObj();
    else
      {
        std::cerr << getClassName() << "::" << __FUNCTION__
                  << "; failed to construct the XC::fdYield\n";
        exit(-1);
      }

    if(fdf_in)
      fdf = fdf_in->newObj();
    else
      {
        std::cerr << getClassName() << "::" << __FUNCTION__
                << "; failed to construct the fdFlow\n";
        exit(-1);
      }

    fdEvolutionS= nullptr;

    fdEvolutionT= nullptr;

    //if(fdeps == 0)
    fdeps = new FDEPState();

  }

// Destructor-------------------------------------------------------------------------------------
XC::FiniteDeformationEP3D::~FiniteDeformationEP3D(void)
  {
    if(fde3d)
      delete fde3d;
    if(fdy)
      delete fdy;

    if(fdf)
      delete fdf;

    if(fdEvolutionS)
      delete fdEvolutionS;

    if(fdEvolutionT)
      delete fdEvolutionT;

    if(fdeps)
      delete fdeps;
}

//----------------------------------------------------------------------
double XC::FiniteDeformationEP3D::getRho(void) const
  { return fde3d->getRho(); }

//----------------------------------------------------------------------
int XC::FiniteDeformationEP3D::setTrialF(const XC::straintensor &f)
{
        F = f;

    // Choose One:
    //return ImplicitAlgorithm();
    return SemiImplicitAlgorithm();

}

//----------------------------------------------------------------------
int XC::FiniteDeformationEP3D::setTrialFIncr(const XC::straintensor &df)
{
        return this->setTrialF(this->getF() + df);
}

//----------------------------------------------------------------------
const XC::BJtensor &XC::FiniteDeformationEP3D::getTangentTensor(void) const
  { return iniTangent; }

//----------------------------------------------------------------------
const XC::straintensor &XC::FiniteDeformationEP3D::getStrainTensor(void) const
  { return iniGreen; }

//----------------------------------------------------------------------
const XC::stresstensor &XC::FiniteDeformationEP3D::getStressTensor(void) const
  { return iniPK2; }

//----------------------------------------------------------------------
const XC::straintensor &XC::FiniteDeformationEP3D::getF(void) const
  { return F; }

//----------------------------------------------------------------------
const XC::straintensor XC::FiniteDeformationEP3D::getFp(void)
  { return fdeps->getFpInVar(); }

//----------------------------------------------------------------------
int XC::FiniteDeformationEP3D::commitState(void)
{
    return fdeps->commitState();
}

//----------------------------------------------------------------------
int XC::FiniteDeformationEP3D::revertToLastCommit(void)
{
    return fdeps->revertToLastCommit();
}

//----------------------------------------------------------------------
int XC::FiniteDeformationEP3D::revertToStart(void)
{
    return fdeps->revertToStart();
}

//@ brief Virtual constructor.
XC::NDMaterial* XC::FiniteDeformationEP3D::getCopy(void) const
  { return  new FiniteDeformationEP3D(*this); }

//----------------------------------------------------------------------
XC::NDMaterial* XC::FiniteDeformationEP3D::getCopy(const std::string &code) const
  {
    XC::NDMaterial *tmp= nullptr;
    if( (code==strTypeFiniteDeformationEP3D)
         || (code==strTypeFDEP3D) )
      tmp= getCopy();
    else
      std::cerr << getClassName()  << "::" << __FUNCTION__
		<< "; getCopy failed:" << code << "\n";
    return tmp;
  }

//----------------------------------------------------------------------
const std::string &XC::FiniteDeformationEP3D::getType(void) const
  { return strTypeThreeDimensionalFD; }


//----------------------------------------------------------------------
int XC::FiniteDeformationEP3D::getOrder(void) const
{
    return 6;
}

//----------------------------------------------------------------------
int XC::FiniteDeformationEP3D::sendSelf(CommParameters &cp)
{
    // Not yet implemented
    return 0;
}

//----------------------------------------------------------------------
int XC::FiniteDeformationEP3D::recvSelf(const CommParameters &)
{
    // Not yet implemented
    return 0;
}

//----------------------------------------------------------------------
void XC::FiniteDeformationEP3D::Print(std::ostream &s, int flag)
{
    s << (*this);
}

//----------------------------------------------------------------------
const XC::stresstensor XC::FiniteDeformationEP3D::getCauchyStressTensor(void)
{
    return cauchystress;
}

//----------------------------------------------------------------------
XC::NDMaterial * XC::FiniteDeformationEP3D::getFDE3D() const
{
    return fde3d;
}

//----------------------------------------------------------------------
 XC::fdYield * XC::FiniteDeformationEP3D::getFDY() const
{
    return fdy;
}

//----------------------------------------------------------------------
 XC::fdFlow * XC::FiniteDeformationEP3D::getFDF() const
{
    return fdf;
}

//----------------------------------------------------------------------
 XC::fdEvolution_S * XC::FiniteDeformationEP3D::getFDEvolutionS() const
{
    return fdEvolutionS;
}

//----------------------------------------------------------------------
 XC::fdEvolution_T * XC::FiniteDeformationEP3D::getFDEvolutionT() const
{
    return fdEvolutionT;
}

//----------------------------------------------------------------------
 XC::FDEPState * XC::FiniteDeformationEP3D::getFDEPState() const
{
    return fdeps;
}


//----------------------------------------------------------------------
///*//
//----------------------------------------------------------------------
int XC::FiniteDeformationEP3D::ImplicitAlgorithm()
{
    // Initializing
    double yieldfun = 0.0;              // trial value of yield function
    double D_gamma  = 0.0;              // consistency parameter /Delta_{gamma}
    double d_gamma  = 0.0;              // increment of consistency parameter /delta_{gamma}
    int         iter_counter = 0;

    XC::straintensor res_Ee = tensorZ2;     // residual of intermediate Ee
    XC::straintensor res_eta = tensorZ2;    // norm of residual of eta

    double  res_xi = 0.0;                // residual of strain like internal variable
    double  res_norm_eta = 0.0;         // norm of residual of eta
    double  res_norm_Ee  = 0.0;         // norm of residual of intermediate Ee
    double  residual = 0.0;                // residual of Ee and xi and eta

    XC::straintensor Fp = tensorI2;;        // Plastic deformation gradient
    double xi = 0.0;                        // strain like internal isotropic variable
    double q = 0.0;                        // stress like internal isotropic variable
    XC::straintensor eta;                        // strain like internal kinematic variable
    XC::stresstensor a;                        // stress like internal kinematic variable

    XC::straintensor Fpinv = tensorI2;      // inverse of Fp
    XC::straintensor Ce = tensorI2;                // intermediate C
    XC::straintensor Ee = tensorZ2;                // intermediate Ee

    XC::straintensor Fp_n = tensorI2;       // Plastic deformation gradient at time n
    XC::straintensor Fp_ninv = tensorI2;    // Plastic deformation gradient at time n
    XC::straintensor Ee_n = tensorZ2;       // Ee at the incremental step n, calculated from Fp_n
    double xi_n = 0.0;                        // xi at the incremental step n, known
    XC::straintensor eta_n = tensorZ2;        // eta at the incremental step n, known

    XC::stresstensor Mtensor = tensorZ2;    // --> dFl/dT
    XC::stresstensor MCtensor = tensorZ2;   // --> dFl/dS
    BJtensor Ltensor = tensorZ4 ;         // Tangent XC::BJtensor in the intermediate configuration
    BJtensor LATStensor = tensorZ4;        // Consistent tangent XC::BJtensor in the intermediate configuration

    double nscalar = 0.0;                // --> dFl/d(xi)
    double Kscalar = 0.0;                // Isotropic hardening modoulus
    XC::straintensor ntensor;                // --> dFl/d(eta)
    BJtensor Ktensor = tensorZ4;                // Kinematic hardening modoulus

    XC::stresstensor dyods = tensorZ2 ;     // --> dY/d(stress)
    double dyodq = 0.0;                        // --> dY/d(xi)
    XC::stresstensor dyoda = tensorZ2 ;     // --> dY/d(eta)

    BJtensor dMods = tensorZ4;                // --> dM/d(stress), from d2Fl/d(stress)d(stress)
    BJtensor dMCods = tensorZ4;                // --> dMs/d(stress)

    double dnsodq = 0.0;                // --> d2Fl/dqdq
    BJtensor dntoda = tensorZ4;           // --> d2Fl/dada

    XC::straintensor D_Ee = tensorZ2;
    double D_xi  = 0.0;
    XC::straintensor D_eta = tensorZ2;

    BJtensor tensorI4 = (tensorI2("ij")*tensorI2("kl")).transpose0110();

    BJtensor A11 = tensorI4;  BJtensor A12 = tensorZ2;  BJtensor A13 = tensorZ4;
    BJtensor A21 = tensorZ2;  double a22 = 1.0;            BJtensor A23 = tensorZ2;
    BJtensor A31 = tensorZ4;  BJtensor A32 = tensorZ2;  BJtensor A33 = tensorI4;

    BJmatrix C99(9, 9, 0.0);
    BJmatrix CCC(19, 19, 0.0);

    BJtensor tensorTemp0;
    BJtensor tensorTemp1;
    BJtensor tensorTemp2;
    BJtensor tensorTemp3;
    BJtensor tensorTemp4;
    BJtensor tensorTemp5;
    BJtensor tensorTemp6;
    BJtensor tensorTemp7;
    BJtensor tensorTemp8;
    BJtensor tensorTemp9;
    double temp1 = 0.0;
    double temp2 = 0.0;
    double temp3 = 0.0;
    double temp4 = 0.0;
    double temp5 = 0.0;

    BJtensor LM = tensorZ4;                 // For Mandel Tangent Stiffness
    XC::stresstensor  B_Mandel = tensorZ2;   // For Mandel stress

    // Read the previous incremental step history variables
    Fp = fdeps->getCommittedFpInVar();
    Fp_n = Fp;
    Fpinv = Fp.inverse();
    Fp_ninv  =  Fp_n.inverse();
    Fe = F("ij")*Fpinv("jk");   Fe.null_indices();
    Ce = Fe("ki")*Fe("kj");     Ce.null_indices();
    Ee = (Ce - tensorI2) * 0.5;
    Ee_n = Ee;

    if( fdEvolutionS ) {
      xi = fdeps->getCommittedStrainLikeInVar();
      xi_n = xi;
      q = fdeps->getCommittedStressLikeInVar();
    }

    if( fdEvolutionT ) {
      eta = fdeps->getCommittedStrainLikeKiVar();
      eta_n = eta;
      a = fdeps->getCommittedStressLikeKiVar();
    }

    // Return stress from finite deformation elastic model
    fde3d->setTrialC(Ce);         // Note: It is C, not F!!!
    B_PK2 = fde3d->getStressTensor();

    B_Mandel = Ce("ik")*B_PK2("kj");        // Mandel Stress
      B_Mandel.null_indices();

    // Evaluate the value of yield function
    yieldfun = fdy->Yd(B_Mandel, *fdeps);
    //printf("\nY0= %e\n", yieldfun);

    if( yieldfun > (fdy->getTolerance()) ) { // start of plastic part
      D_gamma = 0.0;
      d_gamma = 0.0;
      iter_counter = 0;

      do {   // beginning of do - while

    // Return Symmetric XC::BJtensor M and n_scalar and n_tensor
        Mtensor = fdf->dFods(B_Mandel, *fdeps);
        Mtensor = Ce("ik")*Mtensor("kj");
      Mtensor.null_indices();
        MCtensor = (Mtensor + Mtensor.transpose11()) * 0.5;

        // Return tangent variables
    Ltensor = fde3d->getTangentTensor();

    tensorTemp1 = tensorI2("ij") *B_PK2("mn");
      tensorTemp1.null_indices();
    tensorTemp2 = Ce("ik") *Ltensor("kjmn");
      tensorTemp2.null_indices();
        LM = tensorTemp2 + tensorTemp1.transpose0110() + tensorTemp1.transpose0111();

    if( fdEvolutionS) {
      nscalar = fdf->dFodq(B_Mandel, *fdeps);
      Kscalar = fdEvolutionS->HModulus(B_Mandel, *fdeps);
      dyodq = fdy->dYodq(B_Mandel, *fdeps);
      dnsodq = fdf->d2Fodqdq(B_Mandel, *fdeps);
    }

    if( fdEvolutionT) {
      ntensor = fdf->dFoda(B_Mandel, *fdeps);
      Ktensor = fdEvolutionT->HModulus(B_Mandel, *fdeps);
      dyoda = fdy->dYoda(B_Mandel, *fdeps);
      dntoda = fdf->d2Fodada(B_Mandel, *fdeps);
    }

        // Return XC::BJtensor Axx
        dyods = fdy->dYods(B_Mandel, *fdeps);
    dMods = fdf->d2Fodsds(B_Mandel, *fdeps);
        dMCods = Ce("ik")*dMods("kjmn");
      dMCods.null_indices();
        dMCods = (dMCods + dMCods.transpose1100()) * 0.5;

    A11 = dMCods("ijmn")*LM("mnkl");
      A11.null_indices();
        A11 = (A11 *D_gamma) + tensorI4;  //A11

    if( fdEvolutionS ) {
      a22 += (dnsodq *Kscalar*D_gamma);         //A22

      tensorTemp1 = fdf->d2Fodsdq(B_Mandel, *fdeps);
      A12 = Ce("ik") * tensorTemp1("kj");
        A12.null_indices();
          A12 = (A12 + A12.transpose11()) * (0.5*Kscalar*D_gamma);  //A12

      A21 = tensorTemp1("ij") * LM("ijkl");
        A21.null_indices();
      A21 = A21 *D_gamma;    //A21
    }

    if( fdEvolutionT ) {
      A33 = dntoda("ijmn")*Ktensor("mnkl");
        A33.null_indices();
              A33 += (A33 *D_gamma);  //A33

      tensorTemp2 = fdf->d2Fodsda(B_Mandel, *fdeps);
      A13 = Ce("ik") * tensorTemp2("kjmn");
        A13.null_indices();
          A13 = (A13 + A13.transpose1100()) * (0.5*D_gamma);
      A13 = A13("ijmn") * Ktensor("mnkl");
        A13.null_indices();            //A13
      A31 = tensorTemp2("ijmn") * LM("mnkl");
        A31.null_indices();
      A31 = A31 *D_gamma;            //A31
        }

    if( fdEvolutionS && fdEvolutionT ) {
      tensorTemp3 = fdf->d2Fodqda(B_Mandel, *fdeps);
      A23 = tensorTemp3("ij") * Ktensor("ijkl");
        A23.null_indices();
      A23 = A23 *D_gamma; //A23
      tensorTemp2 = fdf->d2Fodsda(B_Mandel, *fdeps);
      A32 = tensorTemp2 * (Kscalar*D_gamma); //A32
    }

    int i, j;
    // CCC: XC::Tensor -> XC::Matrix
    C99 = A11.BJtensor2BJmatrix_2();
    for(i=10; i<=19; i++)
      CCC.val(i,i) = 1.0;

               if( fdEvolutionS ) {
      CCC.val(1,10) = A12.cval(1,1);
      CCC.val(2,10) = A12.cval(1,2);
      CCC.val(3,10) = A12.cval(1,3);
      CCC.val(4,10) = A12.cval(2,1);
      CCC.val(5,10) = A12.cval(2,2);
      CCC.val(6,10) = A12.cval(2,3);
      CCC.val(7,10) = A12.cval(3,1);
      CCC.val(8,10) = A12.cval(3,2);
      CCC.val(9,10) = A12.cval(3,3);

      CCC.val(10,1) = A21.cval(1,1);
          CCC.val(10,2) = A21.cval(1,2);
      CCC.val(10,3) = A21.cval(1,3);
      CCC.val(10,4) = A21.cval(2,1);
      CCC.val(10,5) = A21.cval(2,2);
      CCC.val(10,6) = A21.cval(2,3);
      CCC.val(10,7) = A21.cval(3,1);
      CCC.val(10,8) = A21.cval(3,2);
      CCC.val(10,9) = A21.cval(3,3);

      CCC.val(10,10) = a22;
    }

    if( fdEvolutionT ) {
      C99 = A13.BJtensor2BJmatrix_2();
      for(i =1; i <=9; i++) {
        for(j =1; j <=9; j++) {
          CCC.val(10+i,j) = C99.cval(i,j);
          CCC.val(j,10+i) = C99.cval(i,j);
        }
      }
      C99 = A33.BJtensor2BJmatrix_2();
      for(i =1; i <=9; i++) {
        for(j =1; j <=9; j++) {
          CCC.val(10+i,10+j) = C99.cval(i,j);
        }
      }
    }

    if( fdEvolutionS && fdEvolutionT ) {
      CCC.val(10,11) = A23.cval(1,1);
      CCC.val(10,12) = A23.cval(1,2);
      CCC.val(10,13) = A23.cval(1,3);
      CCC.val(10,14) = A23.cval(2,1);
      CCC.val(10,15) = A23.cval(2,2);
      CCC.val(10,16) = A23.cval(2,3);
      CCC.val(10,17) = A23.cval(3,1);
      CCC.val(10,18) = A23.cval(3,2);
      CCC.val(10,19) = A23.cval(3,3);

      CCC.val(11,10) = A32.cval(1,1);
      CCC.val(12,10) = A32.cval(1,2);
      CCC.val(13,10) = A32.cval(1,3);
      CCC.val(14,10) = A32.cval(2,1);
      CCC.val(15,10) = A32.cval(2,2);
      CCC.val(16,10) = A32.cval(2,3);
      CCC.val(17,10) = A32.cval(3,1);
      CCC.val(18,10) = A32.cval(3,2);
      CCC.val(19,10) = A32.cval(3,3);
    }

    // Inverse of CCC
    //CCC.print("C","\n");
    CCC = CCC.inverse();
    //CCC.print();

    // CCC: XC::Matrix -> XC::Tensor
    for(i =1; i <=9; i++) {
      for(j =1; j <=9; j++) {
        C99.val(i,j) = CCC.cval(i,j);
      }
    }

    A11 = C99.BJmatrix2BJtensor_2();

    A12.val(1,1)=CCC.cval(1,10);
    A12.val(1,2)=CCC.cval(2,10);
    A12.val(1,3)=CCC.cval(3,10);
    A12.val(2,1)=CCC.cval(4,10);
    A12.val(2,2)=CCC.cval(5,10);
    A12.val(2,3)=CCC.cval(6,10);
    A12.val(3,1)=CCC.cval(7,10);
    A12.val(3,2)=CCC.cval(8,10);
    A12.val(3,3)=CCC.cval(9,10);

    A21.val(1,1)=CCC.cval(10,1);
    A21.val(1,2)=CCC.cval(10,2);
    A21.val(1,3)=CCC.cval(10,3);
    A21.val(2,1)=CCC.cval(10,4);
    A21.val(2,2)=CCC.cval(10,5);
    A21.val(2,3)=CCC.cval(10,6);
    A21.val(3,1)=CCC.cval(10,7);
    A21.val(3,2)=CCC.cval(10,8);
    A21.val(3,3)=CCC.cval(10,9);

    a22  = CCC.cval(10,10);

    for(i =1; i <=9; i++) {
      for(j =1; j <=9; j++) {
        C99.val(i,j) = CCC.cval(i,10+j);
      }
    }
    A13 = C99.BJmatrix2BJtensor_2();

    for(i =1; i <=9; i++) {
      for(j =1; j <=9; j++) {
        C99.val(j,i) = CCC.cval(10+i,j);
      }
    }
    A31 = C99.BJmatrix2BJtensor_2();

    A32.val(1,1)=CCC.cval(11,10);
    A32.val(1,2)=CCC.cval(12,10);
    A32.val(1,3)=CCC.cval(13,10);
    A32.val(2,1)=CCC.cval(14,10);
    A32.val(2,2)=CCC.cval(15,10);
    A32.val(2,3)=CCC.cval(16,10);
    A32.val(3,1)=CCC.cval(17,10);
    A32.val(3,2)=CCC.cval(18,10);
    A32.val(3,3)=CCC.cval(19,10);

    A23.val(1,1)=CCC.cval(10,11);
    A23.val(1,2)=CCC.cval(10,12);
    A23.val(1,3)=CCC.cval(10,13);
    A23.val(2,1)=CCC.cval(10,14);
    A23.val(2,2)=CCC.cval(10,15);
    A23.val(2,3)=CCC.cval(10,16);
    A23.val(3,1)=CCC.cval(10,17);
    A23.val(3,2)=CCC.cval(10,18);
    A23.val(3,3)=CCC.cval(10,19);

    for(i =1; i <=9; i++) {
      for(j =1; j <=9; j++) {
        C99.val(i,j) = CCC.cval(10+i,10+j);
      }
    }
    A33 = C99.BJmatrix2BJtensor_2();


        // Return d_gamma
    tensorTemp6 = dyods("ij")*LM("ijkl");
      tensorTemp6.null_indices();
    if( fdEvolutionT ) {
      tensorTemp7 = dyoda("ij")*Ktensor("ijkl");
        tensorTemp7.null_indices();
    }
    // !!!tensorTemp6&7 will be used later

    //Begin, evaluate f^{T}*C
        tensorTemp1 = tensorTemp6("kl")*A11("klmn");
      tensorTemp1.null_indices();

        tensorTemp2  = A21 *(dyodq *Kscalar);
    tensorTemp4 = tensorTemp1 + tensorTemp2;
    if( fdEvolutionT ) {
          tensorTemp3 = tensorTemp7("kl")*A31("klmn");
        tensorTemp3.null_indices();
      tensorTemp4 += tensorTemp3;
    }

        tensorTemp1 = tensorTemp6("kl")*A12("kl");
      tensorTemp1.null_indices();
    temp1 = tensorTemp1.trace();
        temp2 = a22 *(dyodq *Kscalar);
    temp2 += temp1;
    if( fdEvolutionT ) {
          tensorTemp3 = tensorTemp7("kl")*A32("kl");
        tensorTemp3.null_indices();
      temp3 = tensorTemp3.trace();
      temp2 += temp3;
    }

        tensorTemp1 = tensorTemp6("kl")*A13("klmn");
      tensorTemp1.null_indices();
        tensorTemp2  = A23 *(dyodq *Kscalar);
    tensorTemp5 = tensorTemp1 + tensorTemp2;
    if( fdEvolutionT ) {
          tensorTemp3 = tensorTemp7("kl")*A33("klmn");
        tensorTemp3.null_indices();
      tensorTemp5 += tensorTemp3;
    }
    //End, evaluate f^{T}*C: tensorTemp4-temp2-tensorTemp5


    //Begin, evaluate f^{T}*C*r
    tensorTemp1 = tensorTemp4("ij") *res_Ee("ij");
      tensorTemp1.null_indices();
    temp1 = tensorTemp1.trace();
    temp4 = temp1 + temp2*res_xi;
    if( fdEvolutionT ) {
      tensorTemp3 = tensorTemp5("ij") *res_eta("ij");
        tensorTemp3.null_indices();
      temp3 = tensorTemp3.trace();
      temp4 += temp3;
    }
    //End, evaluate f^{T}*C*r: temp4

    //Begin, evaluate f^{T}*C*m
    tensorTemp1 = tensorTemp4("ij") *MCtensor("ij");
      tensorTemp1.null_indices();
    temp1 = tensorTemp1.trace();
    temp5 = temp1 + temp2*nscalar;
    if( fdEvolutionT ) {
      tensorTemp3 = tensorTemp5("ij") *ntensor("ij");
        tensorTemp3.null_indices();
      temp3 = tensorTemp3.trace();
      temp5 += temp3;
    }
    //End, evaluate f^{T}*C*m: temp5
    // !!! temp5 will be used later

    d_gamma = (yieldfun - temp4) / temp5;  // Here is d_gamma!!!
    //if(d_gamma < 0.0) d_gamma = 0.0;

    //Begin,  Calculate incremental variables
    tensorTemp2 = MCtensor*(-d_gamma) - res_Ee;
    temp2 = nscalar*(-d_gamma) - res_xi;
    if( fdEvolutionT ) {
      tensorTemp8 = ntensor*(-d_gamma) - res_eta;
    }

    tensorTemp1 = A11("ijkl") *tensorTemp2("kl");
      tensorTemp1.null_indices();
    D_Ee = tensorTemp1 + A12*temp2;
    if( fdEvolutionT ) {
      tensorTemp3 = A13("ijkl") *tensorTemp8("kl");
        tensorTemp3.null_indices();
      D_Ee += tensorTemp3;
    }

    tensorTemp1 = A21("kl") *tensorTemp2("kl");
      tensorTemp1.null_indices();
    temp1 = tensorTemp1.trace();
    D_xi = temp1 + a22*temp2 ;
    if( fdEvolutionT ) {
      tensorTemp3 = A23("kl") *tensorTemp8("kl");
        tensorTemp3.null_indices();
      temp3 = tensorTemp3.trace();
      D_xi += temp3;
    }

    if( fdEvolutionT ) {
      tensorTemp1 = A31("ijkl") *tensorTemp2("kl");
        tensorTemp1.null_indices();
      tensorTemp3 = A33("ijkl") *tensorTemp8("kl");
        tensorTemp3.null_indices();
      D_eta = tensorTemp1 + tensorTemp3 + (A32*temp2);
    }
    //End,  Calculate incremental variables: D_Ee, D_xi, D_eta

        // Update Variables
        D_gamma += d_gamma;                  // updated D_gamma

    Ee += D_Ee;
    Ce = Ee*2.0 + tensorI2;
    fde3d->setTrialC(Ce);  // Note: It is C, not F!!!
    B_PK2 = fde3d->getStressTensor();    // Updated B_PK2
        B_Mandel = Ce("ik")*B_PK2("kj");     // Update Mandel Stress
          B_Mandel.null_indices();

    xi += D_xi;
    q += Kscalar*D_xi;
    fdeps->setStrainLikeInVar(xi);
    fdeps->setStressLikeInVar(q);

    if( fdEvolutionT ) {
      eta += D_eta;
      tensorTemp2 = Ktensor("ijkl")*D_eta("kl");
        tensorTemp2.null_indices();
      a += tensorTemp2;
      fdeps->setStrainLikeKiVar(eta);
      fdeps->setStressLikeKiVar(a);
    }

    //Begin, Calculate residuals
    res_Ee = (MCtensor*D_gamma) + Ee - Ee_n;
    res_xi = (nscalar*D_gamma) + xi - xi_n;

    tensorTemp1 = res_Ee("ij")*res_Ee("ij");
      tensorTemp1.null_indices();
    res_norm_Ee = tensorTemp1.trace();
        residual = sqrt( res_norm_Ee + res_xi*res_xi );

    if( fdEvolutionT ) {
      res_eta = (ntensor*D_gamma) + eta - eta_n;
      tensorTemp3 = res_eta("ij")*res_eta("ij");
        tensorTemp3.null_indices();
      res_norm_eta = tensorTemp3.trace();
      residual = sqrt( res_norm_Ee + res_xi*res_xi + res_norm_eta );
    }
    //End, Calculate residuals: residual

    yieldfun = fdy->Yd(B_Mandel, *fdeps);        // Updated yieldfun
    //printf("Y= %e\n ", yieldfun);

    iter_counter++;

    if( iter_counter > Max_Iter ) {
      std::cerr << "Stop: Iteration More than " << Max_Iter;
      std::cerr << " in return mapping algorithm of FD EP model" << "\n";
      exit(-1);
    }

      } while( yieldfun > fdy->getTolerance() || residual > tolerance ); // end of do - while

      // For Numerical stability
      D_gamma *= (1.0 - tolerance);
      if( D_gamma < 0.0 )
        D_gamma = 0.0;

      // Update Fp
      tensorTemp2 = Mtensor("ij")*Fp_n("jk");
        tensorTemp2.null_indices();
      Fp = Fp_n + tensorTemp2 *D_gamma;
      fdeps->setFpInVar(Fp);
      Fpinv = Fp.inverse();  //Using the iterative FP

      Fe = F("ij")*Fpinv("jk");   Fe.null_indices();
      Ce = Fe("ki")*Fe("kj");     Ce.null_indices();
      fde3d->setTrialC(Ce);       // Note: It is C, not F!!!
      B_PK2 = fde3d->getStressTensor();

      // iniPK2
      iniPK2 = Fpinv("ip")*Fpinv("jq")*B_PK2("pq");
        iniPK2.null_indices();

      //Begin, evaluate the first term of D^{1}
      tensorTemp8 = tensorTemp4 * (1.0/temp5);
      tensorTemp9 = Mtensor("kj")*tensorTemp8("mn");
        tensorTemp9.null_indices();
      //End, evaluate the first term of D^{1}: tensorTemp9

      //Begin, Evaluate \hat{C}^{11}
      tensorTemp6 = A11("klmn") *MCtensor("mn");
    tensorTemp6.null_indices();
      tensorTemp7  = A12 *nscalar;
      tensorTemp0 = tensorTemp6 + tensorTemp7;
      if( fdEvolutionT ) {
        tensorTemp8 = A13("klmn") *ntensor("mn");
      tensorTemp8.null_indices();
        tensorTemp0 += tensorTemp8;
      }
      tensorTemp0 = tensorTemp0("ij")*tensorTemp4("kl");
        tensorTemp0.null_indices();
      tensorTemp1 = A11 - tensorTemp0*(1.0/temp5);
      //End, Evaluate \hat{C}^{11}

      //Here using the results of tensorTemp1
      LATStensor = Ltensor("ijkl")*tensorTemp1("klmn");
        LATStensor.null_indices();

      //Begin, Evaluate \hat{C}^{21}
      tensorTemp6 = A21("mn") *MCtensor("mn");
    tensorTemp6.null_indices();
      temp1 = tensorTemp6.trace();
      temp4 = temp1 + a22 *nscalar;
      if( fdEvolutionT ) {
        tensorTemp8 = A23("mn") *ntensor("mn");
      tensorTemp8.null_indices();
    temp3 = tensorTemp8.trace();
        temp4 += temp3;
      }
      tensorTemp0 = tensorTemp4 *temp4;
      tensorTemp2 = A21 - tensorTemp0*(1.0/temp5);
      //End, Evaluate \hat{C}^{21}

      //Begin, Evaluate \hat{C}^{31}
      if( fdEvolutionT ) {
        tensorTemp6 = A31("klmn") *MCtensor("mn");
      tensorTemp6.null_indices();
        tensorTemp7 = A32 *nscalar;
        tensorTemp8 = A33("klmn") *ntensor("mn");
      tensorTemp8.null_indices();
        tensorTemp0 = tensorTemp6 + tensorTemp7 + tensorTemp8;
        tensorTemp0 = tensorTemp0("ij")*tensorTemp4("kl");
          tensorTemp0.null_indices();
        tensorTemp3 = A31 - tensorTemp0*(1.0/temp5);
      }
      //End, Evaluate \hat{C}^{31}

      // Begin...
      tensorTemp6 = (fdf->d2Fodsds(B_Mandel, *fdeps))("ijkl")  *LM("klmn");
        tensorTemp6.null_indices();
      tensorTemp7 = (fdf->d2Fodsdq(B_Mandel, *fdeps)) *Kscalar;
      if( fdEvolutionT ) {
        tensorTemp8 = (fdf->d2Fodada(B_Mandel, *fdeps))("ijkl")*Ktensor("klmn");
          tensorTemp8.null_indices();
      }

      tensorTemp5 = tensorTemp6("ijkl")*tensorTemp1("klmn");
        tensorTemp5.null_indices();
      tensorTemp1 = tensorTemp7("ij")*tensorTemp2("mn");
        tensorTemp1.null_indices();
      tensorTemp5 += tensorTemp1;
      if( fdEvolutionT ) {
        tensorTemp2 = tensorTemp8("ijkl")*tensorTemp3("klmn");
          tensorTemp2.null_indices();
        tensorTemp5 += tensorTemp2;
      }

      tensorTemp9 += (tensorTemp5 *D_gamma);

      tensorTemp9 = Fp_ninv("it") * tensorTemp9("tjmn");
        tensorTemp9.null_indices();
      // ...End

      tensorTemp5 = tensorI2("il")*Fpinv("nj")*B_PK2("nk")*tensorTemp9("klpq");
        tensorTemp5.null_indices();
      tensorTemp6 = Fpinv("ni")*tensorI2("jl")*B_PK2("nk")*tensorTemp9("klpq");
        tensorTemp6.null_indices();

      tensorTemp1 = Fpinv("im") * Fpinv("jn");
        tensorTemp1.null_indices();

      tensorTemp7 = tensorTemp1("imjn") * LATStensor("mnrs");
        tensorTemp8.null_indices();

      tensorTemp8 = tensorTemp7 - tensorTemp5 - tensorTemp6;

      tensorTemp2 = Fp_ninv("kr") * Fp_ninv("ls");
        tensorTemp2.null_indices();

      iniTangent = tensorTemp8("ijrs") * tensorTemp2("krls");
        iniTangent.null_indices();

    }      // end of plastic part
    else { // start of elastic part

      Fe = F("ij") * Fp_ninv("jk");
        Fe.null_indices();
      Ce = Fe("ki") * Fe("kj");
        Ce.null_indices();

      fde3d->setTrialC(Ce);  // Note: It is C, not F!!!
      B_PK2 = fde3d->getStressTensor();

      iniPK2 = Fp_ninv("ip")*Fp_ninv("jq")*B_PK2("pq");
        iniPK2.null_indices();

      LATStensor = fde3d->getTangentTensor();

      tensorTemp1 = Fp_ninv("im")*Fp_ninv("jn");
        tensorTemp1.null_indices();
      tensorTemp2 = Fp_ninv("kr")*Fp_ninv("ls");
        tensorTemp2.null_indices();
      tensorTemp3 = tensorTemp1("imjn")*LATStensor("mnrs");
        tensorTemp3.null_indices();
      iniTangent = tensorTemp3("ijrs")*tensorTemp2("krls");
        iniTangent.null_indices();
    }

    iniGreen = F("ki")*F("kj");
      iniGreen.null_indices();
    iniGreen = (iniGreen - tensorI2) * 0.5;

    cauchystress = Fe("ip")*B_PK2("pq");
      cauchystress.null_indices();
    cauchystress = cauchystress("iq")*Fe("jq");
      cauchystress.null_indices();
    cauchystress = cauchystress*(1.0/F.determinant());

    return 0;
}
//*/

//----------------------------------------------------------------------
//Mandel Version
int XC::FiniteDeformationEP3D::SemiImplicitAlgorithm()
{
    // *** This is the key function ***


    // Initializing
    double yieldfun = 0.0;              // trial value of yield function
    double D_gamma  = 0.0;              // consistency parameter /Delta_{gamma}
    double d_gamma  = 0.0;              // increment of consistency parameter /delta_{gamma}
    int         iter_counter = 0;

    XC::straintensor res_Ee = tensorZ2;     // residual of intermediate Ee
    XC::straintensor res_eta = tensorZ2;    // norm of residual of eta

    XC::straintensor Fp = tensorI2;;        // Plastic deformation gradient
    double xi = 0.0;                        // strain like internal isotropic variable
    double q = 0.0;                        // stress like internal isotropic variable
    XC::straintensor eta;                        // strain like internal kinematic variable
    XC::stresstensor a;                        // stress like internal kinematic variable

    XC::straintensor Fpinv = tensorI2;      // inverse of Fp
    XC::straintensor Ce = tensorI2;                // intermediate C
    XC::straintensor Ee = tensorZ2;                // intermediate Ee

    XC::straintensor Fp_n = tensorI2;       // Plastic deformation gradient at time n
    XC::straintensor Fp_ninv = tensorI2;    // Plastic deformation gradient at time n
    XC::straintensor Ee_n = tensorZ2;       // Ee at the incremental step n, calculated from Fp_n
    double xi_n;                        // xi at the incremental step n, known
    XC::straintensor eta_n = tensorZ2;        // eta at the incremental step n, known

    XC::stresstensor Mtensor = tensorZ2;    // --> dFl/dT
    XC::stresstensor MCtensor = tensorZ2;   // --> dFl/dS
    BJtensor Ltensor = tensorZ4 ;         // Tangent XC::BJtensor in the intermediate configuration
    BJtensor LATStensor = tensorZ4;        // Consistent tangent XC::BJtensor in the intermediate configuration

    double nscalar = 0.0;                // --> dFl/d(xi)
    double Kscalar = 0.0;                // Isotropic hardening modoulus
    XC::straintensor ntensor;                // --> dFl/d(eta)
    BJtensor Ktensor = tensorZ4;                // Kinematic hardening modoulus

    XC::stresstensor dyods = tensorZ2 ;     // --> dY/d(stress)
    double dyodq = 0.0;                        // --> dY/d(xi)
    XC::stresstensor dyoda = tensorZ2 ;     // --> dY/d(eta)

    XC::straintensor D_Ee = tensorZ2;
    double D_xi  = 0.0;
    XC::straintensor D_eta = tensorZ2;

    BJtensor tensorTemp0;
    BJtensor tensorTemp1;
    BJtensor tensorTemp2;
    BJtensor tensorTemp3;
    BJtensor tensorTemp4;
    BJtensor tensorTemp5;
    double temp0 = 0.0;
    double lowerPart = 0.0;

    BJtensor LM = tensorZ4;                 // For Mandel Tangent Stiffness
    XC::stresstensor  B_Mandel = tensorZ2;   // For Mandel stress

    tensorTemp1 = tensorI2("ij")*tensorI2("kl");
      tensorTemp1.null_indices();
    BJtensor tensorI4 = tensorTemp1.transpose0110();

    // Read the previous incremental step history variables
    Fp = fdeps->getCommittedFpInVar();
    Fp_n = Fp;
    Fp_ninv  =  Fp_n.inverse();
    Fpinv = Fp.inverse();
    Fe = F("ij")*Fpinv("jk");   Fe.null_indices();
    Ce = Fe("ki")*Fe("kj");     Ce.null_indices();
    Ee = (Ce - tensorI2) * 0.5;
    Ee_n = Ee;

    if( fdEvolutionS ) {
      xi = fdeps->getCommittedStrainLikeInVar();
      xi_n = xi;
      q = fdeps->getCommittedStressLikeInVar();
    }

    if( fdEvolutionT ) {
      eta = fdeps->getCommittedStrainLikeKiVar();
      eta_n = eta;
      a = fdeps->getCommittedStressLikeKiVar();
    }

    // Return stress from finite deformation elastic model
    fde3d->setTrialC(Ce);         // Note: It is C, not F!!!
    B_PK2 = fde3d->getStressTensor();

    B_Mandel = Ce("ik")*B_PK2("kj");        // Mandel Stress
      B_Mandel.null_indices();

    // Evaluate the value of yield function
    yieldfun = fdy->Yd(B_Mandel, *fdeps);
    //printf("\nY0= %e\n", yieldfun);

    if( yieldfun > (fdy->getTolerance()) ) { // start of plastic part
      D_gamma = 0.0;
      d_gamma = 0.0;
      iter_counter = 0;

      Mtensor = fdf->dFods(B_Mandel, *fdeps);
      //MCtensor = Ce("ik")*Mtensor("kj");
      //MCtensor.null_indices();
      //MCtensor = (MCtensor + MCtensor.transpose11()) * 0.5;
      if( fdEvolutionS)
        nscalar = fdf->dFodq(B_Mandel, *fdeps);
      if( fdEvolutionT)
        ntensor = fdf->dFoda(B_Mandel, *fdeps);

      do {   // beginning of do - while
        MCtensor = Ce("ik")*Mtensor("kj");
      MCtensor.null_indices();
        MCtensor = (MCtensor + MCtensor.transpose11()) * 0.5;

        // Return tangent variables
    Ltensor = fde3d->getTangentTensor();
    tensorTemp1 = tensorI2("ij") *B_PK2("mn");
      tensorTemp1.null_indices();
    tensorTemp2 = Ce("ik") *Ltensor("kjmn");
      tensorTemp2.null_indices();
        LM = tensorTemp2 + tensorTemp1.transpose0110() + tensorTemp1.transpose0111();
        dyods = fdy->dYods(B_Mandel, *fdeps);

    if( fdEvolutionS) {
      Kscalar = fdEvolutionS->HModulus(B_Mandel, *fdeps);
      dyodq = fdy->dYodq(B_Mandel, *fdeps);
    }

    if( fdEvolutionT) {
      Ktensor = fdEvolutionT->HModulus(B_Mandel, *fdeps);
      dyoda = fdy->dYoda(B_Mandel, *fdeps);
    }

        // Return d_gamma
    tensorTemp4 = dyods("ij")*LM("ijkl");
      tensorTemp4.null_indices();
    tensorTemp1 = tensorTemp4("kl")*MCtensor("kl");
      tensorTemp1.null_indices();
    lowerPart = tensorTemp1.trace();

    if( fdEvolutionS )
      lowerPart += dyodq * (Kscalar*nscalar);

    if( fdEvolutionT ) {
      tensorTemp5 = dyoda("ij")*Ktensor("ijkl");
        tensorTemp5.null_indices();
      tensorTemp2 = tensorTemp5("kl")*ntensor("kl");
        tensorTemp2.null_indices();
      temp0 = tensorTemp2.trace();
      lowerPart += temp0;
    }

    if(lowerPart != 0.0)
      d_gamma = yieldfun / lowerPart;
    //if(d_gamma < 0.0) d_gamma = 0.0;
    //printf("d_gamma= %e\n ", d_gamma);

    //Begin,  Calculate incremental variables
    D_Ee = MCtensor * (-d_gamma);
    if( fdEvolutionS )
      D_xi = nscalar * (-d_gamma);
    if( fdEvolutionT )
      D_eta = ntensor * (-d_gamma);

        // Update Variable
        D_gamma += d_gamma;   // updated D_gamma

    Ee += D_Ee;
    Ce = Ee*2.0 + tensorI2;
    fde3d->setTrialC(Ce);  // Note: It is C, not F!!!
    B_PK2 = fde3d->getStressTensor();    // Updated B_PK2
        B_Mandel = Ce("ik")*B_PK2("kj");     // Update Mandel Stress
          B_Mandel.null_indices();

    if( fdEvolutionS ) {
      xi += D_xi;
      q += (Kscalar * D_xi);
      fdeps->setStrainLikeInVar(xi);
      fdeps->setStressLikeInVar(q);
    }

    if( fdEvolutionT ) {
      eta += D_eta;
      tensorTemp1 = Ktensor("ijkl") *D_eta("kl");
        tensorTemp1.null_indices();
      a += tensorTemp1;
      fdeps->setStrainLikeKiVar(eta);
      fdeps->setStressLikeKiVar(a);
    }

    yieldfun = fdy->Yd(B_Mandel, *fdeps);        // Updated yieldfun
    //printf("Y= %e\n ", yieldfun);

    iter_counter++;

    if( iter_counter > Max_Iter ) {
      std::cerr << "Stop: Iteration More than " << Max_Iter;
      std::cerr << " in return mapping algorithm of FD EP model" << "\n";
      exit(-1);
    }

      } while( yieldfun > fdy->getTolerance() ); // end of do - while

      //// For Numerical stability
      //D_gamma *= (1.0 - tolerance);
      //if( D_gamma < 0.0 )
      //  D_gamma = 0.0;

      // Update Fp
      tensorTemp2 = Mtensor("ij")*Fp_n("jk");
        tensorTemp2.null_indices();
      Fp = Fp_n + (tensorTemp2 *D_gamma);
      fdeps->setFpInVar(Fp);

      // Return iniTangent and iniPK2
      Fpinv = Fp.inverse();       // Using the iterative FP
      Fe = F("ij")*Fpinv("jk");
        Fe.null_indices();
      Ce = Fe("ki")*Fe("kj");
        Ce.null_indices();

      fde3d->setTrialC(Ce);       // Note: It is C, not F!!!
      B_PK2 = fde3d->getStressTensor();

      //iniPK2 = B_PK2;
      iniPK2 = Fpinv("ip")*Fpinv("jq")*B_PK2("pq");
        iniPK2.null_indices();

      tensorTemp5 = Ltensor("ijkl") * MCtensor("kl");
        tensorTemp5.null_indices();

      tensorTemp3 = tensorTemp5("ij") * tensorTemp4("mn");
        tensorTemp3.null_indices();

      if(lowerPart != 0.0)
        LATStensor = Ltensor - ( tensorTemp3 * (1.0/lowerPart) );

      tensorTemp1 = Mtensor("ri")*Fpinv("lj")*Fp_ninv("kr")*B_PK2("kl");
        tensorTemp1.null_indices();
      tensorTemp2 = Fpinv("ki")*Mtensor("rj")*Fp_ninv("lr")*B_PK2("kl");
        tensorTemp2.null_indices();
      tensorTemp3 = tensorTemp1 + tensorTemp2;
      tensorTemp5 = tensorTemp3("ij") * tensorTemp4("mn");
        tensorTemp5.null_indices();

      tensorTemp1 = Fpinv("im") * Fpinv("jn");
        tensorTemp1.null_indices();
      tensorTemp3 = tensorTemp1("imjn") * LATStensor("mnrs");
        tensorTemp3.null_indices();

      tensorTemp3 = tensorTemp3 - ( tensorTemp5 * (1.0/lowerPart) );

      tensorTemp2 = Fpinv("kr") * Fpinv("ls");
        tensorTemp2.null_indices();
      iniTangent = tensorTemp3("ijrs") * tensorTemp2("krls");
        iniTangent.null_indices();

    }       // end of plastic part
    else {  // start of elastic part

    // Return iniTangent and iniPK2

      Fe = F("ij") * Fp_ninv("jk");
        Fe.null_indices();
      Ce = Fe("ki") * Fe("kj");
        Ce.null_indices();

      fde3d->setTrialC(Ce);  // Note: It is C, not F!!!
      B_PK2 = fde3d->getStressTensor();

      //iniPK2 = B_PK2;
      iniPK2 = Fp_ninv("ip")*Fp_ninv("jq")*B_PK2("pq");
        iniPK2.null_indices();

      LATStensor = fde3d->getTangentTensor();

      tensorTemp1 = Fp_ninv("im") * Fp_ninv("jn");
        tensorTemp1.null_indices();
      tensorTemp2 = Fp_ninv("kr") * Fp_ninv("ls");
        tensorTemp2.null_indices();
      tensorTemp3 = tensorTemp1("imjn") * LATStensor("mnrs");
        tensorTemp3.null_indices();
      iniTangent = tensorTemp3("ijrs") * tensorTemp2("krls");
        iniTangent.null_indices();

    }  // end of elastic part

    iniGreen = F("ki") * F("kj");
      iniGreen.null_indices();
    iniGreen = (iniGreen - tensorI2) * 0.5;

    cauchystress = Fe("ip") * B_PK2("pq");
      cauchystress.null_indices();
    cauchystress = cauchystress("iq") * Fe("jq");
      cauchystress.null_indices();
    cauchystress = cauchystress * (1.0/F.determinant());

    return 0;
}


#endif
