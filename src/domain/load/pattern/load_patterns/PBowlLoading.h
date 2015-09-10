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
//# PROJECT:           Object Oriented Finite Element Program
//# PURPOSE:           Plastic Bowl (aka Domain Reduction) implementation:
//#                    This file contains the class definition for PBowlLoading.
//#                    PBowlLoading is a subclass of loadPattern,
//#                    which implements the plastic bowl loading
//#                    (aka Domain Reduction Method) as described
//#                    by Jacobo Bielak et al.
//# CLASS:             PBowlLoading
//#
//# VERSION:           0.61803398874989 (golden section)
//# LANGUAGE:          C++
//# TARGET OS:         all...
//# DESIGN:            Zhaohui Yang, Boris Jeremic
//# PROGRAMMER(S):     Jinxiu Liao, Zhaohui Yang, Boris Jeremic
//#
//#
//# DATE:              21Oct2002
//# UPDATE HISTORY:    31Oct2002 fixed some memory leaks
//#                    04Nov2002 changed the way plastic bowl elements are
//#                     input.
//#                    10Nov2002 Zeroing diagonal and out of diagaonal blocks
//#                     for b<->e nodes
//#                    13Nov2002 changes to split "b" and "e" nodes within
//#                     the plastic bowl elements. Also, simple definition of
//#                     of cubic bowl is now facilitated ...
//#
//#
//#
//===============================================================================

#ifndef PBowlLoading_h
#define PBowlLoading_h

// Purpose:
#include <domain/load/pattern/LoadPattern.h>

namespace XC {
class Vector;
class Matrix;
 class ID;

//! @ingroup LPatterns
//
//! @brief ??.
class PBowlLoading : public LoadPattern
  {
  private:
    ID *PBowlElements;   // vector containing the plastic bowling elements
    ID *ExteriorNodes;   // vector containing the nodes on plastic bowl except boundary nodes
    ID *BoundaryNodes;   // vector containing the nodes on the boundary of the plastic bowl
    Matrix *PBowlLoads;  // matrix containing the plastic bowling loads

    Matrix *U;           // vector to store input displ. for all nodes and all time steps
    int UnumDataPoints;   // number of data points
    Matrix *Udd;         // vector to store input accel. for all nodes and all time steps
    int UddnumDataPoints;// number of data points

    int thetimeSteps;


    //Coordinates for the plastic box
    double PBTimeIncr;   // specifies the time increment used in load path vector
    double cFactor;      // additional factor on the returned load factor
    double xPlus;	 // x-coor for the right surface
    double xMinus;       // x-coor for the left  surface
    double yPlus;	 // y-coor for the right surface
    double yMinus;	 // y-coor for the left surface
    double zPlus;	 // z-coor for the up surface
    double zMinus;	 // z-coor for the bottom surface

    bool LoadComputed;   // flag to indicate whether the equivalent force has been computed
  protected:
    //void addPBElements(const ID &PBEle);    //Adding plastic bowl elements
    //void addPBNodes(const ID &PBNodes);     //Adding plastic bowl nodes
    //void addPBLoads(const Matrix &PBLoads); //Adding plastic bowl loades
    void CompPBLoads();        //Finding all plastic bowl nodes and compute the equivalent forces from plastic bowl loading
    const Vector & getNodalLoad(int node, double time); //Getting the nodal load computed from plastic bowl loading corresponding to time
  public:
    PBowlLoading(void);
    PBowlLoading(int tag);
    PBowlLoading(int tag, const char *PBEfName, const char *DispfName,
                 const char *AccefName, double theTimeIncr=1.0,double theFactor=1.0,
// coordinates of "b" nodes for cubic plastic bowl
                 double xplus  = 0.0,
                 double xminus = 0.0,
                 double yplus  = 0.0,
                 double yminus = 0.0,
                 double zplus  = 0.0,
                 double zminus = 0.0);
    ~PBowlLoading(void);

    void setDomain(Domain *theDomain);
    void applyLoad(double time);
    void Print(std::ostream &s, int flag =0);

    // methods for o/p
    int sendSelf(CommParameters &);
    int recvSelf(const CommParameters &);

  };
} // fin namespace XC

#endif