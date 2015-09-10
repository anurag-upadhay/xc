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
** file 'COPYRIGHT'  in main directory for information on usage and   **
** redistribution,  and for a DISCLAIMER OF ALL WARRANTIES.           **
**                                                                    **
** Developed by:                                                      **
**   Frank McKenna (fmckenna@ce.berkeley.edu)                         **
**   Gregory L. Fenves (fenves@ce.berkeley.edu)                       **
**   Filip C. Filippou (filippou@ce.berkeley.edu)                     **
**                                                                    **
** ****************************************************************** */
                                                                        
// $Revision: 1.8 $
// $Date: 2003/02/25 23:33:13 $
// $Source: /usr/local/cvs/OpenSees/SRC/element/zeroLength/ZeroLength.h,v $
                                                                        
                                                                        
#ifndef ZeroLength_h
#define ZeroLength_h

// File: ~/element/zeroLength/ZeroLength.h
// 
// Written: GLF
// Created: 12/99
// Revision: A
//
// Description: This file contains the class definition for ZeroLength.
// A ZeroLength element is defined by two nodes with the same coordinate.
// One or more material objects may be associated with the nodes to
// provide a force displacement relationship.
// ZeroLength element will work with 1d, 2d, or 3d material models.
//
// What: "@(#) ZeroLength.h, revA"

#include <domain/mesh/element/Element0D.h>
#include "material/uniaxial/ZeroLengthMaterials.h"
#include <utility/matrix/Matrix.h>
#include <material/uniaxial/DqUniaxialMaterial.h>


// Type of dimension of element NxDy has dimension x=1,2,3 and
// y=2,4,6,12 degrees-of-freedom for the element
enum Etype { D1N2, D2N4, D2N6, D3N6, D3N12 };


namespace XC {
class Node;
class Channel;
class UniaxialMaterial;
class Response;

//! \ingroup ElemZL
//
//! @brief Elemento de longitud cero.
class ZeroLength: public Element0D
  {
  private:
    Etype elemType;
    Matrix *theMatrix; //!< pointer to objects matrix (a class Matrix)
    Vector *theVector; //!< pointer to objects vector (a class Vector)
    // Storage for uniaxial material models
    ZeroLengthMaterials theMaterial1d; //!< array of pointers to 1d materials y direcciones asociadas.

    Matrix t1d;        // hold the transformation matrix

    // private methods
    void checkDirection(ID &dir) const;
    
    void setUp(int Nd1, int Nd2,const Vector &x,const Vector &y);
    void setTran1d ( Etype e, int n );
    double computeCurrentStrain1d(int mat, const Vector& diff ) const;    



    // static data - single copy for all objects of the class    
    static Matrix ZeroLengthM2;   // class wide matrix for 2*2
    static Matrix ZeroLengthM4;   // class wide matrix for 4*4
    static Matrix ZeroLengthM6;   // class wide matrix for 6*6
    static Matrix ZeroLengthM12;  // class wide matrix for 12*12
    static Vector ZeroLengthV2;   // class wide Vector for size 2
    static Vector ZeroLengthV4;   // class wide Vector for size 4
    static Vector ZeroLengthV6;   // class wide Vector for size 6
    static Vector ZeroLengthV12;  // class wide Vector for size 12

  protected:
    void setUpType(const size_t &);

    int sendData(CommParameters &);
    int recvData(const CommParameters &);
    bool procesa_comando(CmdStatus &status);
  public:
    // Constructor for a single 1d material model
    ZeroLength(int tag,int dimension,int Nd1, int Nd2,const Vector& x,const Vector& yprime,UniaxialMaterial &theMaterial,int direction );
    // Constructor for a multiple 1d material models
    ZeroLength(int tag,int dimension,int Nd1, int Nd2, const Vector& x,const Vector& yprime,const DqUniaxialMaterial &theMaterial,const ID &direction);
    ZeroLength(int tag,int dimension,const Material *ptr_mat,int direction);
    ZeroLength(void);
    Element *getCopy(void) const;
    ~ZeroLength(void);

    void setDomain(Domain *theDomain);

    inline void clearMaterials(void)
      { theMaterial1d.clear(); }
    void setMaterial(const int &,const std::string &);
    void setMaterials(const std::deque<int> &,const std::vector<std::string> &);
    ZeroLengthMaterials &getMaterials(void)
      { return theMaterial1d; }
    // public methods to set the state of the element    
    int commitState(void);
    int revertToLastCommit(void);        
    int revertToStart(void);        
    int update(void);

    // public methods to obtain stiffness, mass, damping and residual information    
    std::string getElementType(void) const;
    const Matrix &getTangentStiff(void) const;
    const Matrix &getInitialStiff(void) const;
    const Matrix &getDamp(void) const;
    const Matrix &getMass(void) const;

    int addLoad(ElementalLoad *theLoad, double loadFactor);
    int addInertiaLoadToUnbalance(const Vector &accel);    

    const Vector &getResistingForce(void) const;
    const Vector &getResistingForceIncInertia(void) const;            

    // public methods for element output
    int sendSelf(CommParameters &);
    int recvSelf(const CommParameters &);
    void Print(std::ostream &s, int flag =0);    

    Response *setResponse(const std::vector<std::string> &argv, Information &eleInformation);
    int getResponse(int responseID, Information &eleInformation);
    
    void updateDir(const Vector& x, const Vector& y);
    any_const_ptr GetProp(const std::string &cod) const;
  };
} // fin namespace XC

#endif



