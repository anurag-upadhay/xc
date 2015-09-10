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
** ****************************************************************** */
                                                                        
 
/*----+----+----+----+----+----+----+----+----+----+----+----+----+----+----*
 |                                                                          | 
 |              MultiaxialCyclicPlasticity  NDMaterial                      |
 +                                                                          +
 |--------------------------------------------------------------------------|
 |                                                                          |
 +             Authors: Gang Wang  AND  Professor Nicholas Sitar            +
 |                                                                          |
 |			   Department of Civil and Environmental Engineering            |
 +			   Univeristy of California, Berkeley, CA 94720, USA            +
 |                                                                          |
 |             Email: wang@ce.berkeley.edu (G.W.)                           |
 +                                                                          + 
 |  Disclaimers:                                                            |
 |  (1) This is implemenation of MultiaxialCyclicPlasticity for clays       |
 +      Model References:                                                   +
 |      Borja R.I, Amies, A.P. Multiaxial Cyclic Plasticity Model for       |
 |            Clays, ASCE J. Geotech. Eng. Vol 120, No 6, 1051-1070         |
 +      Montans F.J, Borja R.I. Implicit J2-bounding Surface Plasticity     +    
 |            using Prager's translation rule. Int. J. Numer. Meth. Engng.  |
 |            55:1129-1166, 2002                                            |
 +      Code References:                                                    +
 |      Ignacio Romero and Adrian Rodriguez Marek, Brick element model with |
 |            a Multiaxial Cyclic Plasticity Model, in GEOFEAP, UC Berkeley |
 +  (2) Questions regarding this code should be directed to Gang Wang       +
 |  (3) Documentation could be found at                                     |
 |        www.ce.berkeley.edu/~wang/papers/MultiaxialCyclicPlasticity.pdf   |
 +                                                                          +
 |  Development History:                                                    |
 |  First Draft   -- April 2004                                             |
 +  Rewrite       -- Nov   2004                                             + 
 |  Final Release --                                                        |
 |                                                                          | 
 +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----*/

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  
                              User Command

   ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    nDMaterial MultiaxialCyclicPlasticity $tag, $rho, $K, $G,
	                                      $Su , $Ho , $h, $m, $beta, $KCoeff
   ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   where: 
      tag   : tag for this material
	  rho   : density
	  K     : buck modulus
	  G     : maximum (small strain) shear modulus
	  Su    : undrained shear strength, size of bounding surface R=sqrt(8/3)*Su
	  Ho    : linear kinematic hardening modulus of bounding surface
	  h     : hardening parameter
	  m     : hardening parameter
	  beta  : integration parameter, usually beta=0.5
	  KCoeff: coefficient of earth pressure, K0

  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


#ifndef MultiaxialCyclicPlasticity_h
#define MultiaxialCyclicPlasticity_h

#include <cstdio> 
#include <cstdlib> 
#include <cmath> 

#include <utility/matrix/Vector.h>
#include <utility/matrix/Matrix.h>
#include <material/nD/NDMaterial.h>


namespace XC{
//! @ingroup SoilNDMat
//
//! @brief ??.
class MultiaxialCyclicPlasticity: public NDMaterial
  {
  protected :
    //this is mike's problem
    static Tensor rank2;
    static Tensor rank4;

    // Material parameter used for K0 condition
    double K0;           //lateral earth pressure coefficient           
    double bulk_K0;
    double shear_K0;

    // Parameters used for Bounding Surface
    double bulk ;        //bulk modulus
    double shear ;       //shear modulus
    double density;      //material density (mass/volume)
    double R;			   //Radius of Bounding Surface, R=sqrt(8/3)*Su
    double Ho;		   //Limit value of hardening modulus Ho=Hard
    double h;			   //Exponential degradation parameter H=h*kappa^m
    double m;			   //Exponential degradation parameter H=h*kappa^m
    double beta;         //integration parameter
    double eta;         //viscosity   // not used now
 
  // some flags
    int flagjustunload;       // not used
    int flagfirstload;        // very first loading, initialize so_n
    int icounter;             // iteration counter, local newton
    int iternum;              // iteration counter, global newton

    int plasticflag;          // flags indicate stage of plasticity at current step
    int plasticflag_n;        // flags indicate stage of plasticity at t=n
  
    // state variables
    double kappa; 	   //kappa at t=n
    double Psi;          //Psi   at t=n
    double X[3];         //X[1]:Psi X[2]:kappa at t=n+1
    double alp;          //alp for strain split across B.S.
    double load;         //loading/unloading indicator

  //material input
  Matrix strain ;		 // strain @ t=n+1, input from element
  //material response 
  Matrix stress ;		 // stress @ t=n+1, computed this step
  Matrix backs;                  // back stress for BS @ t=n
  Matrix so;                     // unload deviatoric back stress
  //memory variables
  Matrix strain_n;               // strain @ t=n;   stored before
  Matrix stress_n;		 // stress @ t=n;   stored before
  Matrix backs_n;                // back stress for BS @ t=n+1
  Matrix so_n;                   // unload point for t=n

  double tangent[3][3][3][3] ;   // material tangent
  static double initialTangent[3][3][3][3] ;   //material tangent
  static double IIdev[3][3][3][3] ; //rank 4 deviatoric 
  static double IbunI[3][3][3][3] ; //rank 4 I bun I 


  // element tag associated with this material; added method by Gang Wang
  int EleTag;
  static int MaterialStageID; // classwide tag
  static int IncrFormulationFlag; 


  //parameters
  static const double one3 ;
  static const double two3 ;
  static const double four3 ;
  static const double root23 ;
  static const double infinity ;

  static Vector MCPparameter; // debug tool

  
  void initialize(void); //initialize internal variables
  void plastic_integrator(void); //plasticity integration routine, used in MaterialStageID==2
  void elastic_integrator(void);//elasticity integration routine, used in MaterialStageID==1 (K0)

  void doInitialTangent(void) const;
  virtual void index_map( int matrix_index, int &i, int &j ) const;//matrix index to tensor index mapping


  protected:
    int sendData(CommParameters &);
    int recvData(const CommParameters &);
  public : 
    //null constructor
    MultiaxialCyclicPlasticity();
    MultiaxialCyclicPlasticity(int tag,int classtag= ND_TAG_MultiaxialCyclicPlasticity);

				   
//full constructor
  MultiaxialCyclicPlasticity(int    tag,
			     int    classTag,
			     double rho,
			     double K,
			     double G,
			     double Su,
			     double Ho_kin,
			     double Parameter_h,
			     double Parameter_m,
			     double Parameter_beta,
			     double Kcoeff,
			     double viscosity = 0
			     );


  //elastic constructor
  MultiaxialCyclicPlasticity( int tag, int classTag, double rho, double K, double G );

  virtual NDMaterial* getCopy(const std::string &) const;

  //swap history variables
  virtual int commitState(void); 
  //revert to last saved state
  virtual int revertToLastCommit(void);
  //revert to start
  virtual int revertToStart(void);

  //sending and receiving
  virtual int sendSelf(CommParameters &);  
  virtual int recvSelf(const CommParameters &);

  //print out material data
  void Print(std::ostream &s, int flag = 0);

  virtual NDMaterial *getCopy(void) const;
  virtual const std::string &getType(void) const ;
  virtual int getOrder (void) const ;
    
  double getRho(void) const;
  int updateParameter(int responseID, Information &eleInformation);	
  Vector& getMCPparameter(void);   // used for debug only
  }; //end of MultiaxialCyclicPlasticity declarations

} //fin namespace XC

#endif