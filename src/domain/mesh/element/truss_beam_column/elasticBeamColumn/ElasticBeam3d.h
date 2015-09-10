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
// $Date: 2003/03/11 20:42:40 $
// $Source: /usr/local/cvs/OpenSees/SRC/element/elasticBeamColumn/ElasticBeam3d.h,v $
                                                                        
                                                                        
// File: ~/model/ElasticBeam3d.h
//
// Written: fmk 11/95
// Revised:
//
// Purpose: This file contains the class definition for ElasticBeam3d.
// ElasticBeam3d is a plane frame member.

#ifndef ElasticBeam3d_h
#define ElasticBeam3d_h

#include <domain/mesh/element/truss_beam_column/ProtoBeam3d.h>
#include <utility/matrix/Matrix.h>
#include "domain/mesh/element/truss_beam_column/EsfBeamColumn3d.h"
#include "domain/mesh/element/fvectors/FVectorBeamColumn3d.h"
#include "domain/mesh/element/coordTransformation/CrdTransf3d.h"

namespace XC {
class Channel;
class Information;
class Response;
class SectionForceDeformation;

//! \ingroup ElemBarra
//
//! @brief Elemento barra para problemas tridimensionales.
class ElasticBeam3d : public ProtoBeam3d
  {
  private:
    Vector eInic; //!< section initial deformations
    double rho;  //!< Densidad.
    int sectionTag;
    mutable EsfBeamColumn3d q;
    FVectorBeamColumn3d q0;  //!< Fixed end forces in basic system (no torsion)
    FVectorBeamColumn3d p0;  //!< Reactions in basic system (no torsion)
 
    CrdTransf3d *theCoordTransf; //!< Transformación de coordenadas.

    static Matrix K;
    static Vector P;
    
    static Matrix kb;

    void set_transf(const CrdTransf *trf);
  protected:
    DbTagData &getDbTagData(void) const;
    int sendData(CommParameters &cp);
    int recvData(const CommParameters &cp);
    bool procesa_comando(CmdStatus &status);
  public:
    ElasticBeam3d(int tag= 0);
    ElasticBeam3d(int tag,const Material *m,const CrdTransf *trf);
    ElasticBeam3d(int tag, double A, double E, double G, double Jx, double Iy, double Iz, int Nd1, int Nd2,
		  CrdTransf3d &theTransf, double rho = 0.0, int sectionTag = 0);

    ElasticBeam3d(int tag, int Nd1, int Nd2, SectionForceDeformation *section, CrdTransf3d &theTransf, double rho = 0.0);
    ElasticBeam3d(const ElasticBeam3d &otro);
    ElasticBeam3d &operator=(const ElasticBeam3d &otro);
    Element *getCopy(void) const;
    ~ElasticBeam3d(void);

    int setInitialSectionDeformation(const Vector&);
    inline const Vector &getInitialSectionDeformation(void) const
      { return eInic; }
    const Vector &getSectionDeformation(void) const;

    void setDomain(Domain *theDomain);
    
    virtual CrdTransf *getCoordTransf(void);
    virtual const CrdTransf *getCoordTransf(void) const;

    int commitState(void);
    int revertToLastCommit(void);
    int revertToStart(void);

    double getRho(void) const
      { return rho; }
    void setRho(const double &r)
      { rho= r; }
    const Vector &getInitialStrain(void) const
      { return eInic; }
    void setInitialStrain(const Vector &e)
      { eInic= e; }
    
    int update(void);
    const Matrix &getTangentStiff(void) const;
    const Matrix &getInitialStiff(void) const;
    const Matrix &getMass(void) const;    

    void zeroLoad(void);	
    int addLoad(ElementalLoad *theLoad, double loadFactor);
    int addInertiaLoadToUnbalance(const Vector &accel);

    const Vector &getResistingForce(void) const;
    const Vector &getResistingForceIncInertia(void) const;

    Vector getVDirEjeFuerteLocales(void) const;
    Vector getVDirEjeDebilLocales(void) const;
    double getAnguloEjeFuerte(void) const;
    double getAnguloEjeDebil(void) const;
    const Vector &getVDirEjeFuerteGlobales(void) const;
    const Vector &getVDirEjeDebilGlobales(void) const;
    
    int sendSelf(CommParameters &);
    int recvSelf(const CommParameters &);
    
    void Print(std::ostream &s, int flag =0);    

    inline double getAN1(void) //fuerza axil que se ejerce sobre la barra en su extremo dorsal.
      {                 //¡Ojo! antes hay que llamara a "calc_resisting_force".
        return q.AN1()+p0[0];
      }
    inline double getAN2(void) //fuerza axil que se ejerce sobre la barra en su extremo frontal.
      {                 //¡Ojo! antes hay que llamara a "calc_resisting_force".
        return q.AN2();
      }
    inline double getN1(void) //axil en el extremo dorsal.
      {                 //¡Ojo! antes hay que llamara a "calc_resisting_force".
        return -q.AN1()-p0[0];
      }
    inline double getN2(void) //axil en el extremo frontal.
      {                 //¡Ojo! antes hay que llamara a "calc_resisting_force".
        return q.AN2();
      }
    inline double getN(void) //axil medio.
      {                 //¡Ojo! antes hay que llamara a "calc_resisting_force".
        return (-q.AN1()-p0[0]+q.AN2())/2.0;
      }
    inline double getAMz1(void)
      {                 //¡Ojo! antes hay que llamara a "calc_resisting_force".
        return q.Mz1(); //Momento z que se ejerce sobre la barra en su extremo dorsal.
      }
    inline double getAMz2(void)
      {                 //¡Ojo! antes hay que llamara a "calc_resisting_force".
        return q.Mz2(); //Momento z que se ejerce sobre la barra en su extremo frontal.
      }
    inline double getMz1(void)
      {                 //¡Ojo! antes hay que llamara a "calc_resisting_force".
        return -q.Mz1(); //Momento z en su extremo dorsal.
      }
    inline double getMz2(void)
      {                 //¡Ojo! antes hay que llamara a "calc_resisting_force".
        return -q.Mz2(); //Momento z en su extremo frontal.
      }
    inline double getVy(void)
      {                 //¡Ojo! antes hay que llamara a "calc_resisting_force".
        return q.Vy(theCoordTransf->getInitialLength()); //Cortante y.
      }
    inline double getAVy1(void)
      {                 //¡Ojo! antes hay que llamara a "calc_resisting_force".
        return q.Vy(theCoordTransf->getInitialLength())+p0[1]; //Cortante y que se ejerce sobre la barra en su extremo dorsal.
      }
    inline double getAVy2(void)
      {                 //¡Ojo! antes hay que llamara a "calc_resisting_force".
        return -q.Vy(theCoordTransf->getInitialLength())+p0[2]; //Cortante y que se ejerce sobre la barra en su extremo frontal.
      }
    inline double getVy1(void)
      {                 //¡Ojo! antes hay que llamara a "calc_resisting_force".
        return -q.Vy(theCoordTransf->getInitialLength())-p0[1]; //Cortante y en su extremo dorsal.
      }
    inline double getVy2(void)
      {                 //¡Ojo! antes hay que llamara a "calc_resisting_force".
        return q.Vy(theCoordTransf->getInitialLength())-p0[2]; //Cortante y en su extremo frontal.
      }
    inline double getVz(void)
      {                 //¡Ojo! antes hay que llamara a "calc_resisting_force".
        return q.Vz(theCoordTransf->getInitialLength()); //Cortante z.
      }
    inline double getAVz1(void)
      {                 //¡Ojo! antes hay que llamara a "calc_resisting_force".
        return q.Vz(theCoordTransf->getInitialLength())+p0[3]; //Cortante z que se ejerce sobre la barra en su extremo dorsal.
      }
    inline double getAVz2(void)
      {                 //¡Ojo! antes hay que llamara a "calc_resisting_force".
        return -q.Vz(theCoordTransf->getInitialLength())+p0[4]; //Cortante z que se ejerce sobre la barra en su extremo frontal.
      }
    inline double getVz1(void)
      {                 //¡Ojo! antes hay que llamara a "calc_resisting_force".
        return -q.Vz(theCoordTransf->getInitialLength())-p0[3]; //Cortante z en su extremo dorsal.
      }
    inline double getVz2(void)
      {                 //¡Ojo! antes hay que llamara a "calc_resisting_force".
        return q.Vz(theCoordTransf->getInitialLength())-p0[4]; //Cortante z en su extremo frontal.
      }
    inline double getMy1(void)
      {                 //¡Ojo! antes hay que llamara a "calc_resisting_force".
        return q.My1(); //Momento y en el extremo dorsal.
      }
    inline double getMy2(void)
      {                 //¡Ojo! antes hay que llamara a "calc_resisting_force".
        return q.My2(); //Momento y en el extremo frontal.
      }
    inline double getT(void) //Torsor en la barra.
      {                 //¡Ojo! antes hay que llamara a "calc_resisting_force".
        return q.T();
      }
    inline double getT1(void)
      {                 //¡Ojo! antes hay que llamara a "calc_resisting_force".
        return q.T1(); //+p0[0]; //Torsor en el extremo dorsal.
      }
    inline double getT2(void)
      {                 //¡Ojo! antes hay que llamara a "calc_resisting_force".
        return q.T2(); //Torsor en el extremo frontal.
      }
    any_const_ptr GetProp(const std::string &) const;

    Response *setResponse(const std::vector<std::string> &argv, Information &info);
    int getResponse(int responseID, Information &info);
  };
} // fin namespace XC

#endif

