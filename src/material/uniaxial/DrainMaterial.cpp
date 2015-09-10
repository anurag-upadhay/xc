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
/* ****************************************************************** **
**    OpenSees - Open System for Earthquake Engineering Simulation    **
**          Pacific Earthquake Engineering Research Center            **
**                                                                    **
**                                                                    **
** (C) Copyright 1999, The Regents of the University of California    **
** All Rights Reserved.                                               **
**                                                                    **
** Commercial use of this program without express permission of the   **
** University of California, Berkeley, is strictly prohibited.  See   **
** file 'COPYRIGHT'  in XC::main directory for information on usage and   **
** redistribution,  and for a DISCLAIMER OF ALL WARRANTIES.           **
**                                                                    **
** Developed by:                                                      **
**   Frank McKenna (fmckenna@ce.berkeley.edu)                         **
**   Gregory L. Fenves (fenves@ce.berkeley.edu)                       **
**   Filip C. Filippou (filippou@ce.berkeley.edu)                     **
**                                                                    **
** ****************************************************************** */
                                                                        
// $Revision: 1.5 $
// $Date: 2003/04/02 22:02:42 $
// $Source: /usr/local/cvs/OpenSees/SRC/material/uniaxial/DrainMaterial.cpp,v $
                                                                        
// Written: MHS
// Created: June 2001
//
// Description: This file contains the class definition for 
// DrainMaterial. DrainMaterial wraps a Drain spring element subroutine
// and converts it to the XC::UniaxialMaterial interface for use in
// zero length elements, beam sections, or anywhere else
// UniaxialMaterials may be used.
//
// For more information see the Drain-2DX user guide:
//    Allahabadi, R.; Powell, G. H.
//    UCB/EERC-88/06, Berkeley: Earthquake Engineering Research Center,
//    University of California, Mar. 1988, 1 vol.

#include <material/uniaxial/DrainMaterial.h>
#include <utility/matrix/Vector.h>
#include <utility/matrix/ID.h>

#include <cstdlib>
#include <cfloat>

XC::DrainMaterial::DrainMaterial(int tag, int classTag, int nhv, int ndata, double b)
  :UniaxialMaterial(tag,classTag),epsilon(0.0), epsilonDot(0.0), sigma(0.0), tangent(0.0),
   data(ndata), hstv(2*nhv,0.0), numData(ndata), numHstv(nhv),
   epsilonP(0.0), sigmaP(0.0), tangentP(0.0), beto(b)
  {
    if(numHstv < 0)
      numHstv = 0;
    
    if(numData < 0)
      numData = 0;
  
    // determine initial tangent
    this->invokeSubroutine();
    initialTangent = tangent;
  }

int XC::DrainMaterial::setTrialStrain(double strain, double strainRate)
  {
        // Store the strain
        epsilon = strain;
        epsilonDot = strainRate;

        // Invoke Drain subroutine
        return this->invokeSubroutine();
  }

int XC::DrainMaterial::setTrial(double strain, double &stress, double &stiff, double strainRate)
  {
        // Store the strain
        epsilon = strain;
        epsilonDot = strainRate;

        // Invoke Drain subroutine
        int res = this->invokeSubroutine();

        stress = sigma;
        stiff = tangent;

        return res;
  }

double XC::DrainMaterial::getStrain(void) const
  { return epsilon; }

double XC::DrainMaterial::getStrainRate(void) const
  { return epsilonDot; }

double XC::DrainMaterial::getStress(void) const
  { return sigma; }

double XC::DrainMaterial::getTangent(void) const
  { return tangent; }

double XC::DrainMaterial::getInitialTangent(void) const
  { return initialTangent; }

double XC::DrainMaterial::getDampTangent(void) const
  {
    // Damping computed here using the last committed tangent
    // rather than the initial tangent!
    return beto*tangentP;
  }

int XC::DrainMaterial::commitState(void)
  {
        // Set committed values equal to corresponding trial values
        for (int i = 0; i < numHstv; i++)
                hstv[i] = hstv[i+numHstv];

        epsilonP = epsilon;
        sigmaP   = sigma;
        tangentP = tangent;

        return 0;
  }

int XC::DrainMaterial::revertToLastCommit(void)
  {
        // Set trial values equal to corresponding committed values
        for (int i = 0; i < numHstv; i++)
                hstv[i+numHstv] = hstv[i];

        epsilon = epsilonP;
        sigma   = sigmaP;
        tangent = tangentP;

        return 0;
  }

int XC::DrainMaterial::revertToStart(void)
  {
        // Set all trial and committed values to zero
        for (int i = 0; i < 2*numHstv; i++)
                hstv[i] = 0.0;

        epsilonP = 0.0;
        sigmaP   = 0.0;
        tangentP = 0.0;

        return 0;
  }

// WARNING -- if you wish to override any method in this base class, you must
// also override the getCopy method to return a pointer to the derived class!!!
XC::UniaxialMaterial *XC::DrainMaterial::getCopy(void) const
  { return new DrainMaterial(*this); }


//! @brief Envía sus miembros a través del canal que se pasa como parámetro.
int XC::DrainMaterial::sendData(CommParameters &cp)
  {
    int res= UniaxialMaterial::sendData(cp);
    res+= cp.sendDoubles(epsilon,epsilonDot,sigma,tangent,getDbTagData(),CommMetaData(3));
    res+= cp.sendVector(data,getDbTagData(),CommMetaData(4));
    res+= cp.sendVector(hstv,getDbTagData(),CommMetaData(5));
    res+= cp.sendInts(numData,numHstv,getDbTagData(),CommMetaData(6));
    res+= cp.sendDoubles(epsilonP,sigmaP,tangentP,beto,initialTangent,getDbTagData(),CommMetaData(7));
    return res;
  }

//! @brief Recibe sus miembros a través del canal que se pasa como parámetro.
int XC::DrainMaterial::recvData(const CommParameters &cp)
  {
    int res= UniaxialMaterial::recvData(cp);
    res+= cp.receiveDoubles(epsilon,epsilonDot,sigma,tangent,getDbTagData(),CommMetaData(3));
    res+= cp.receiveVector(data,getDbTagData(),CommMetaData(4));
    res+= cp.receiveVector(hstv,getDbTagData(),CommMetaData(5));
    res+= cp.receiveInts(numData,numHstv,getDbTagData(),CommMetaData(6));
    res+= cp.receiveDoubles(epsilonP,sigmaP,tangentP,beto,initialTangent,getDbTagData(),CommMetaData(7));
    return res;
  }

//! @brief Envía el objeto a través del canal que se pasa como parámetro.
int XC::DrainMaterial::sendSelf(CommParameters &cp)
  {
    setDbTag(cp);
    const int dataTag= getDbTag();
    inicComm(8);
    int res= sendData(cp);

    res+= cp.sendIdData(getDbTagData(),dataTag);
    if(res < 0)
      std::cerr << nombre_clase() << "sendSelf() - failed to send data\n";
    return res;
  }

//! @brief Recibe el objeto a través del canal que se pasa como parámetro.
int XC::DrainMaterial::recvSelf(const CommParameters &cp)
  {
    inicComm(8);
    const int dataTag= getDbTag();
    int res= cp.receiveIdData(getDbTagData(),dataTag);

    if(res<0)
      std::cerr << nombre_clase() << "::recvSelf - failed to receive ids.\n";
    else
      {
        //setTag(getDbTagDataPos(0));
        res+= recvData(cp);
        if(res<0)
          std::cerr << nombre_clase() << "::recvSelf - failed to receive data.\n";
      }
    return res;
  }
    
void XC::DrainMaterial::Print(std::ostream &s, int flag)
  {
        s << "DrainMaterial, type: ";
        
        switch (this->getClassTag()) {
        case MAT_TAG_DrainHardening:
                s << "Hardening" << std::endl;
                break;
        case MAT_TAG_DrainBilinear:
                s << "Bilinear" << std::endl;
                break;
        case MAT_TAG_DrainClough1:
                s << "Clough1" << std::endl;
                break;
        case MAT_TAG_DrainClough2:
                s << "Clough2" << std::endl;
                break;
        case MAT_TAG_DrainPinch1:
                s << "Pinch1" << std::endl;
                break;
        // Add more cases as needed

        default:
                s << "Material identifier = " << this->getClassTag() << std::endl;
                break;
        }
  }

// Declarations for the Hardening subroutines
extern "C" int fill00_(double *data, double *hstv, double *stateP);
extern "C" int resp00_(int *kresis, int *ksave, int *kgem, int *kstep,
                                                                int *ndof, int *kst, int *kenr,
                                                                double *ener, double *ened, double *enso, double *beto,
                                                                double *relas, double *rdamp, double *rinit,
                                                                double *ddise, double *dise, double *vele);
extern "C" int stif00_(int *kstt, int *ktype, int *ndof, double *fk);
extern "C" int get00_(double *hstv);


// I don't know which subroutines to call, so fill in the XX for XC::Bilinear later -- MHS
// Declarations for the XC::Bilinear subroutines
//extern "C" int fillXX_(double *data, double *hstv, double *stateP);
//extern "C" int respXX_(int *kresis, int *ksave, int *kgem, int *kstep,
//                                                                int *ndof, int *kst, int *kenr,
//                                                                double *ener, double *ened, double *enso, double *beto,
//                                                                double *relas, double *rdamp, double *rinit,
//                                                                double *ddise, double *dise, double *vele);
//extern "C" int stifXX_(int *kstt, int *ktype, int *ndof, double *fk);
//extern "C" int getXX_(double *hstv);


// I don't know which subroutines to call, so fill in the XX for Clough1 later -- MHS
// Declarations for the Clough1 subroutines
//extern "C" int fillXX_(double *data, double *hstv, double *stateP);
//extern "C" int respXX_(int *kresis, int *ksave, int *kgem, int *kstep,
//                                                                int *ndof, int *kst, int *kenr,
//                                                                double *ener, double *ened, double *enso, double *beto,
//                                                                double *relas, double *rdamp, double *rinit,
//                                                                double *ddise, double *dise, double *vele);
//extern "C" int stifXX_(int *kstt, int *ktype, int *ndof, double *fk);
//extern "C" int getXX_(double *hstv);


// I don't know which subroutines to call, so fill in the XX for Clough2 later -- MHS
// Declarations for the Clough2 subroutines
//extern "C" int fillXX_(double *data, double *hstv, double *stateP);
//extern "C" int respXX_(int *kresis, int *ksave, int *kgem, int *kstep,
//                                                                int *ndof, int *kst, int *kenr,
//                                                                double *ener, double *ened, double *enso, double *beto,
//                                                                double *relas, double *rdamp, double *rinit,
//                                                                double *ddise, double *dise, double *vele);
//extern "C" int stifXX_(int *kstt, int *ktype, int *ndof, double *fk);
//extern "C" int getXX_(double *hstv);


// I don't know which subroutines to call, so fill in the XX for Pinch1 later -- MHS
// Declarations for the Pinch1 subroutines
//extern "C" int fillXX_(double *data, double *hstv, double *stateP);
//extern "C" int respXX_(int *kresis, int *ksave, int *kgem, int *kstep,
//                                                                int *ndof, int *kst, int *kenr,
//                                                                double *ener, double *ened, double *enso, double *beto,
//                                                                double *relas, double *rdamp, double *rinit,
//                                                                double *ddise, double *dise, double *vele);
//extern "C" int stifXX_(int *kstt, int *ktype, int *ndof, double *fk);
//extern "C" int getXX_(double *hstv);


// Add more declarations as needed

int XC::DrainMaterial::invokeSubroutine(void)
  {
    // Number of degrees of freedom
    static const int NDOF = 2;

    // Flags sent into RESPXX subroutine
    int kresis = 2;                // Compute static and damping forces
    int ksave  = 0;                // Do not save results
    int kgem   = 0;                // Geometric effects (not used)
    int kstep  = 1;                // Step number (set by subroutine)
    int ndof   = NDOF;        // Number of degrees of freedom
    int kst    = 1;                // Stiffness formation code
    int kenr   = 2;                // Calculate static and dynamic energy

    // Energy terms computed in RESPXX subroutine
    double ener = 0.0;        // Change in elasto-plastic energy
    double ened = 0.0;        // Change in damping energy
    double enso = 0.0;        // Change in second-order energy (not used)

    // Force terms computed in RESPXX subroutine
    static double relas[NDOF];        // Resisting force vector
    static double rdamp[NDOF];        // Damping force vector
    static double rinit[NDOF];        // Initial force vector (not used)

    // Total displacement vector
    static double dise[NDOF];
    dise[0] = 0.0;
    dise[1] = epsilon;

    // Incremental displacement vector
    static double ddise[NDOF];
    ddise[0] = 0.0;
    ddise[1] = epsilon-epsilonP;

    // Velocity vector
    static double vele[NDOF];
    vele[0] = 0.0;
    vele[1] = epsilonDot;

    // Fill in committed state array
    static double stateP[3];
    stateP[0] = epsilonP;
    stateP[1] = sigmaP;
    stateP[2] = tangentP;

    // Flags sent into STIFXX subroutine
    int kstt  = 1;                        // Total stiffness
    int ktype = 1;                        // Elastic stiffness only

    // Stiffness computed in STIFXX subroutine
    static double fk[NDOF*NDOF];

    double *dataPtr= new double[numData];
    assert(dataPtr);
    for(int i= 0;i<numData;i++)
      dataPtr[i]= data[i];
    double *hstvPtr= new double[2*numHstv];
    assert(hstvPtr);
    for(int i= 0;i<2*numHstv;i++)
      hstvPtr[i]= hstv[i];
    
    switch (this->getClassTag())
      {
      case MAT_TAG_DrainHardening:
        // Fill the common block with parameters and history variables
        fill00_(dataPtr, hstvPtr, stateP);

                // Call the response subroutine
                resp00_(&kresis, &ksave, &kgem, &kstep, &ndof, &kst, &kenr,
                        &ener, &ened, &enso, &beto, relas, rdamp, rinit, ddise, dise, vele);
                
                // Call the stiffness subroutine
                stif00_(&kstt, &ktype, &ndof, fk);
                
                // Get the trial history variables
                get00_(&hstvPtr[numHstv]);
                break;

        case MAT_TAG_DrainBilinear:
                // I don't know which subroutines to call, so fill in the XX for Bilinear later -- MHS
                std::cerr << "DrainMaterial::invokeSubroutine -- Bilinear subroutine not yet linked\n"; exit(-1);


                //fillXX_(dataPtr, hstvPtr, stateP);
                //respXX_(&kresis, &ksave, &kgem, &kstep, &ndof, &kst, &kenr,
                //        &ener, &ened, &enso, &beto, relas, rdamp, rinit, ddise, dise, vele);
                //stifXX_(&kstt, &ktype, &ndof, fk);
                //getXX_(&hstvPtr[numHstv]);
                break;

        case MAT_TAG_DrainClough1:
                // I don't know which subroutines to call, so fill in the XX for Clough1 later -- MHS
                std::cerr << "XC::DrainMaterial::invokeSubroutine -- Clough1 subroutine not yet linked\n"; exit(-1);

                //fillXX_(dataPtr, hstvPtr, stateP);
                //respXX_(&kresis, &ksave, &kgem, &kstep, &ndof, &kst, &kenr,
                //        &ener, &ened, &enso, &beto, relas, rdamp, rinit, ddise, dise, vele);
                //stifXX_(&kstt, &ktype, &ndof, fk);
                //getXX_(&hstvPtr[numHstv]);
                break;

        case MAT_TAG_DrainClough2:
                // I don't know which subroutines to call, so fill in the XX for Clough2 later -- MHS
                std::cerr << "XC::DrainMaterial::invokeSubroutine -- Clough2 subroutine not yet linked\n"; exit(-1);
                          
                //fillXX_(dataPtr, hstvPtr, stateP);
                //respXX_(&kresis, &ksave, &kgem, &kstep, &ndof, &kst, &kenr,
                //        &ener, &ened, &enso, &beto, relas, rdamp, rinit, ddise, dise, vele);
                //stifXX_(&kstt, &ktype, &ndof, fk);
                //getXX_(&hstvPtr[numHstv]);
                break;

        case MAT_TAG_DrainPinch1:
                // I don't know which subroutines to call, so fill in the XX for Pinch1 later -- MHS
                std::cerr << "XC::DrainMaterial::invokeSubroutine -- Pinch1 subroutine not yet linked\n"; exit(-1);
                
                //fillXX_(dataPtr, hstvPtr, stateP);
                //respXX_(&kresis, &ksave, &kgem, &kstep, &ndof, &kst, &kenr,
                //        &ener, &ened, &enso, &beto, relas, rdamp, rinit, ddise, dise, vele);
                //stifXX_(&kstt, &ktype, &ndof, fk);
                //getXX_(&hstvPtr[numHstv]);
                break;

        // Add more cases as needed

        default:
                std::cerr << "XC::DrainMaterial::invokeSubroutine -- unknown material type\n"; exit(-1);
                return -1;
        }
    for(int i= 0;i<numData;i++)
      data[i]= dataPtr[i];
    for(int i= 0;i<2*numHstv;i++)
      hstv[i]= hstvPtr[i];

    // Total stress is elastic plus damping force
    sigma = relas[1] + rdamp[1];

    // Get tangent stiffness
    tangent = fk[0];
    return 0;
  }