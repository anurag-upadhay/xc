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
//ShellCrdTransf3dBase.cc

#include "ShellCrdTransf3dBase.h"
#include "utility/matrix/Matrix.h"
#include <domain/mesh/node/Node.h>

#include "utility/actor/objectBroker/FEM_ObjectBroker.h"
#include "utility/actor/actor/MovableVector.h"
#include "utility/matrix/ID.h"
#include "domain/mesh/element/plane/shell/R3vectors.h"
#include "domain/mesh/element/NodePtrs.h"
#include "xc_utils/src/base/CmdStatus.h"
#include "xc_utils/src/base/any_const_ptr.h"
#include "xc_utils/src/base/utils_any.h"
#include "xc_utils/src/geom/pos_vec/Pos2d.h"
#include "xc_utils/src/geom/pos_vec/Pos3d.h"
#include "xc_utils/src/geom/pos_vec/Vector3d.h"
#include "xc_utils/src/geom/pos_vec/Vector2d.h"
#include "xc_utils/src/geom/d2/Plano3d.h"
#include "xc_utils/src/geom/sis_ref/Ref2d3d.h"

//! @brief Constructor por defecto
XC::ShellCrdTransf3dBase::ShellCrdTransf3dBase(void)
  : EntCmd(), MovableObject(0), theNodes(nullptr),
    vpos_centroide(3,0.0), g1(3,0.0), g2(3,0.0), g3(3,0.0) {}

//! @brief Constructor por defecto
XC::ShellCrdTransf3dBase::ShellCrdTransf3dBase(const Vector &v1,const Vector &v2,const Vector &v3)
  : EntCmd(), MovableObject(0), theNodes(nullptr),
    vpos_centroide(3,0.0), g1(v1), g2(v2), g3(v3) {}

//! @brief Lee comandos de un objeto ShellCrdTransf3dBase
bool XC::ShellCrdTransf3dBase::procesa_comando(CmdStatus &status)
  {
    const std::string cmd= deref_cmd(status.Cmd());
    if(verborrea>2)
      std::clog << "(ShellCrdTransf3dBase) Procesando comando: " << cmd << std::endl;

    if(cmd == "update")
      {
        std::clog << "ShellCrdTransf3dBase; el comando: " << cmd
                  << " está pensado para pruebas." << std::endl; 
        status.GetBloque(); //Ignoramos argumentos.
        update();
        return true;
      }
    else
      return EntCmd::procesa_comando(status);
  }

int XC::ShellCrdTransf3dBase::initialize(const NodePtrs &ptrs)
  {
    theNodes= &ptrs;
    vpos_centroide.Zero();
    vpos_centroide+= ptrs[0]->getCrds();
    vpos_centroide+= ptrs[1]->getCrds();
    vpos_centroide+= ptrs[2]->getCrds();
    vpos_centroide+= ptrs[3]->getCrds();
    vpos_centroide/=4.0;
    return 0;
  }

//! @brief actualiza las coordenadas locales de los nodos.
int XC::ShellCrdTransf3dBase::setup_nodal_local_coordinates(double xl[2][4]) const
  {
    //local nodal coordinates in plane of shell
    for(int i= 0;i<4;i++)
      {
        const Vector &coorI= (*theNodes)[i]->getCrds();
        xl[0][i]= coorI^g1;
        xl[1][i]= coorI^g2;
      }
    return 0;
  }

//! @brief Returns element's plane.
Plano3d XC::ShellCrdTransf3dBase::getPlane(void) const
  {
    const Vector &vO= getVPosCentroide();
    const Pos3d O(vO[0],vO[1],vO[2]);
    const Vector3d v= Vector3d(g3[0],g3[1],g3[2]);
    return Plano3d(O,v);
  }

//! @brief Returns point's projection over element's plane.
Pos3d XC::ShellCrdTransf3dBase::getProj(const Pos3d &p)
  { return getPlane().Proyeccion(p); }

//! @brief Returns element's reference.
Ref2d3d XC::ShellCrdTransf3dBase::getLocalReference(void) const
  {
    const Vector &vO= getVPosCentroide();
    const Pos3d O(vO[0],vO[1],vO[2]);
    const Vector3d v1= Vector3d(g1[0],g1[1],g1[2]);
    const Vector3d v2= Vector3d(g2[0],g2[1],g2[2]);
    return Ref2d3d(O,v1,v2);
  }

//! @brief Returns point's local coordinates.
Pos2d XC::ShellCrdTransf3dBase::getLocalCoordinates(const Pos3d &p) const
  {
    //local coordinates of point in plane of shell
    const Vector coorI(p.x(),p.y(),p.z());
    return Pos2d(coorI^g1,coorI^g2);
  }

//! @brief Return natural coordinates for point xyz (in cartesian coord.)
//! based on \$ 23.6 from Felippa book:"Introduction to Finite Element Methods"
//! see IFEM.Ch23.pdf
//! @param p position in element's local coordinates.
XC::ParticlePos3d XC::ShellCrdTransf3dBase::getNaturalCoordinates(const Pos3d &pg,double xl[2][4]) const
  {
    ParticlePos3d retval;
    const double &x1= xl[0][0]; const double &x2= xl[0][1]; const double &x3= xl[0][2]; const double &x4= xl[0][3];
    const double &y1= xl[1][0]; const double &y2= xl[1][1]; const double &y3= xl[1][2]; const double &y4= xl[1][3];
    const double xb= x1-x2+x3-x4;
    const double yb= y1-y2+y3-y4;
    const double xcChi= x1+x2-x3-x4;
    const double ycChi= y1+y2-y3-y4;
    const double xcEta= x1-x2-x3+x4;
    const double ycEta= y1-y2-y3+y4;
    const double x0= (x1+x2+x3+x4)/4.0;
    const double y0= (y1+y2+y3+y4)/4.0;
    const double J0= (x3-x1)*(y4-y2)-(x4-x2)*(y3-y1); 
    const double A=J0/2.0;
    const double J1= (x3-x4)*(y1-y2)-(x1-x2)*(y3-y4);
    const double J2= (x2-x3)*(y1-y4)-(x1-x4)*(y2-y3);
    const Pos2d p= getLocalCoordinates(pg);
    const double dx= p.x()-x0;
    const double dy= p.y()-y0;
    const double bChi= A-dx*yb+dy*xb;
    const double bEta= -A-dx*yb+dy*xb;
    const double cChi= dx*ycChi-dy*xcChi;
    const double cEta= dx*ycEta-dy*xcEta;
    retval.set_r_coordinate(2*cChi/(-sqrt(bChi*bChi-2*J1*cChi)-bChi ));
    retval.set_s_coordinate(2*cEta/( sqrt(bEta*bEta+2*J2*cEta)-bEta));
    return retval;
  }   


//! @brief Devuelve la matriz de la transformación.
const XC::Matrix &XC::ShellCrdTransf3dBase::getTrfMatrix(void) const
  {
    static Matrix R(3,3);
    // Fill in transformation matrix
    R(0,0)= g1(0); R(0,1)= g1(1); R(0,2)= g1(2);
    R(1,0)= g2(0); R(1,1)= g2(1); R(1,2)= g2(2);
    R(2,0)= g3(0); R(2,1)= g3(1); R(2,2)= g3(2);
    
    return R;
  }

//! @brief Transformación a globales de un vector.
XC::Vector XC::ShellCrdTransf3dBase::local_to_global(const Matrix &R,const Vector &pl) const
  {
    Vector retval(24);
    //Nodo 1.
    retval(0)= R(0,0)*pl[0] + R(1,0)*pl[1] + R(2,0)*pl[2];
    retval(1)= R(0,1)*pl[0] + R(1,1)*pl[1] + R(2,1)*pl[2];
    retval(2)= R(0,2)*pl[0] + R(1,2)*pl[1] + R(2,2)*pl[2];

    retval(3)= R(0,0)*pl[3] + R(1,0)*pl[4] + R(2,0)*pl[5];
    retval(4)= R(0,1)*pl[3] + R(1,1)*pl[4] + R(2,1)*pl[5];
    retval(5)= R(0,2)*pl[3] + R(1,2)*pl[4] + R(2,2)*pl[5];

    //Nodo 2.
    retval(6)= R(0,0)*pl[6] + R(1,0)*pl[7] + R(2,0)*pl[8];
    retval(7)= R(0,1)*pl[6] + R(1,1)*pl[7] + R(2,1)*pl[8];
    retval(8)= R(0,2)*pl[6] + R(1,2)*pl[7] + R(2,2)*pl[8];

    retval(9)= R(0,0)*pl[9] + R(1,0)*pl[10] + R(2,0)*pl[11];
    retval(10)= R(0,1)*pl[9] + R(1,1)*pl[10] + R(2,1)*pl[11];
    retval(11)= R(0,2)*pl[9] + R(1,2)*pl[10] + R(2,2)*pl[11];

    //Nodo 3.
    retval(12)= R(0,0)*pl[12] + R(1,0)*pl[13] + R(2,0)*pl[14];
    retval(13)= R(0,1)*pl[12] + R(1,1)*pl[13] + R(2,1)*pl[14];
    retval(14)= R(0,2)*pl[12] + R(1,2)*pl[13] + R(2,2)*pl[14];

    retval(15)= R(0,0)*pl[15] + R(1,0)*pl[16] + R(2,0)*pl[17];
    retval(16)= R(0,1)*pl[15] + R(1,1)*pl[16] + R(2,1)*pl[17];
    retval(17)= R(0,2)*pl[15] + R(1,2)*pl[16] + R(2,2)*pl[17];

    //Nodo 4.
    retval(18)= R(0,0)*pl[18] + R(1,0)*pl[19] + R(2,0)*pl[20];
    retval(19)= R(0,1)*pl[18] + R(1,1)*pl[19] + R(2,1)*pl[20];
    retval(20)= R(0,2)*pl[18] + R(1,2)*pl[19] + R(2,2)*pl[20];

    retval(21)= R(0,0)*pl[21] + R(1,0)*pl[22] + R(2,0)*pl[23];
    retval(22)= R(0,1)*pl[21] + R(1,1)*pl[22] + R(2,1)*pl[23];
    retval(23)= R(0,2)*pl[21] + R(1,2)*pl[22] + R(2,2)*pl[23];

    return retval;
  }

//! @brief Transformación a globales de una matriz.
XC::Matrix XC::ShellCrdTransf3dBase::local_to_global(const Matrix &R,const Matrix &kl) const
  {
    static Matrix tmp(24,24);

    // Transform local matrix to global system
    // First compute kl*T_{lg}
    for(int m = 0; m < 24; m++)
      {
        tmp(m,0)= kl(m,0)*R(0,0) + kl(m,1)*R(1,0)  + kl(m,2)*R(2,0);
        tmp(m,1)= kl(m,0)*R(0,1) + kl(m,1)*R(1,1)  + kl(m,2)*R(2,1);
        tmp(m,2)= kl(m,0)*R(0,2) + kl(m,1)*R(1,2)  + kl(m,2)*R(2,2);

        tmp(m,3)= kl(m,3)*R(0,0) + kl(m,4)*R(1,0)  + kl(m,5)*R(2,0);
        tmp(m,4)= kl(m,3)*R(0,1) + kl(m,4)*R(1,1)  + kl(m,5)*R(2,1);
        tmp(m,5)= kl(m,3)*R(0,2) + kl(m,4)*R(1,2)  + kl(m,5)*R(2,2);

        tmp(m,6)= kl(m,6)*R(0,0) + kl(m,7)*R(1,0)  + kl(m,8)*R(2,0);
        tmp(m,7)= kl(m,6)*R(0,1) + kl(m,7)*R(1,1)  + kl(m,8)*R(2,1);
        tmp(m,8)= kl(m,6)*R(0,2) + kl(m,7)*R(1,2)  + kl(m,8)*R(2,2);

        tmp(m,9)= kl(m,9)*R(0,0) + kl(m,10)*R(1,0) + kl(m,11)*R(2,0);
        tmp(m,10)= kl(m,9)*R(0,1) + kl(m,10)*R(1,1) + kl(m,11)*R(2,1);
        tmp(m,11)= kl(m,9)*R(0,2) + kl(m,10)*R(1,2) + kl(m,11)*R(2,2);

        tmp(m,12)= kl(m,12)*R(0,0) + kl(m,13)*R(1,0) + kl(m,14)*R(2,0);
        tmp(m,13)= kl(m,12)*R(0,1) + kl(m,13)*R(1,1) + kl(m,14)*R(2,1);
        tmp(m,14)= kl(m,12)*R(0,2) + kl(m,13)*R(1,2) + kl(m,14)*R(2,2);

        tmp(m,15)= kl(m,15)*R(0,0) + kl(m,16)*R(1,0) + kl(m,17)*R(2,0);
        tmp(m,16)= kl(m,15)*R(0,1) + kl(m,16)*R(1,1) + kl(m,17)*R(2,1);
        tmp(m,17)= kl(m,15)*R(0,2) + kl(m,16)*R(1,2) + kl(m,17)*R(2,2);

        tmp(m,18)= kl(m,18)*R(0,0) + kl(m,19)*R(1,0) + kl(m,20)*R(2,0);
        tmp(m,19)= kl(m,18)*R(0,1) + kl(m,19)*R(1,1) + kl(m,20)*R(2,1);
        tmp(m,20)= kl(m,18)*R(0,2) + kl(m,19)*R(1,2) + kl(m,20)*R(2,2);

        tmp(m,21)= kl(m,21)*R(0,0) + kl(m,22)*R(1,0) + kl(m,23)*R(2,0);
        tmp(m,22)= kl(m,21)*R(0,1) + kl(m,22)*R(1,1) + kl(m,23)*R(2,1);
        tmp(m,23)= kl(m,21)*R(0,2) + kl(m,22)*R(1,2) + kl(m,23)*R(2,2);
      }

    Matrix retval(24,24); // Global matrix for return
    // Now compute T'_{lg}*(kl*T_{lg})
    for(int m = 0;m<24;m++)
      {
        retval(0,m) = R(0,0)*tmp(0,m) + R(1,0)*tmp(1,m)  + R(2,0)*tmp(2,m);
        retval(1,m) = R(0,1)*tmp(0,m) + R(1,1)*tmp(1,m)  + R(2,1)*tmp(2,m);
        retval(2,m) = R(0,2)*tmp(0,m) + R(1,2)*tmp(1,m)  + R(2,2)*tmp(2,m);

        retval(3,m) = R(0,0)*tmp(3,m) + R(1,0)*tmp(4,m)  + R(2,0)*tmp(5,m);
        retval(4,m) = R(0,1)*tmp(3,m) + R(1,1)*tmp(4,m)  + R(2,1)*tmp(5,m);
        retval(5,m) = R(0,2)*tmp(3,m) + R(1,2)*tmp(4,m)  + R(2,2)*tmp(5,m);

        retval(6,m) = R(0,0)*tmp(6,m) + R(1,0)*tmp(7,m)  + R(2,0)*tmp(8,m);
        retval(7,m) = R(0,1)*tmp(6,m) + R(1,1)*tmp(7,m)  + R(2,1)*tmp(8,m);
        retval(8,m) = R(0,2)*tmp(6,m) + R(1,2)*tmp(7,m)  + R(2,2)*tmp(8,m);

        retval(9,m)  = R(0,0)*tmp(9,m) + R(1,0)*tmp(10,m) + R(2,0)*tmp(11,m);
        retval(10,m) = R(0,1)*tmp(9,m) + R(1,1)*tmp(10,m) + R(2,1)*tmp(11,m);
        retval(11,m) = R(0,2)*tmp(9,m) + R(1,2)*tmp(10,m) + R(2,2)*tmp(11,m);

        retval(12,m) = R(0,0)*tmp(12,m) + R(1,0)*tmp(13,m) + R(2,0)*tmp(14,m);
        retval(13,m) = R(0,1)*tmp(12,m) + R(1,1)*tmp(13,m) + R(2,1)*tmp(14,m);
        retval(14,m) = R(0,2)*tmp(12,m) + R(1,2)*tmp(13,m) + R(2,2)*tmp(14,m);

        retval(15,m) = R(0,0)*tmp(15,m) + R(1,0)*tmp(16,m) + R(2,0)*tmp(17,m);
        retval(16,m) = R(0,1)*tmp(15,m) + R(1,1)*tmp(16,m) + R(2,1)*tmp(17,m);
        retval(17,m) = R(0,2)*tmp(15,m) + R(1,2)*tmp(16,m) + R(2,2)*tmp(17,m);

        retval(18,m) = R(0,0)*tmp(18,m) + R(1,0)*tmp(19,m) + R(2,0)*tmp(20,m);
        retval(19,m) = R(0,1)*tmp(18,m) + R(1,1)*tmp(19,m) + R(2,1)*tmp(20,m);
        retval(20,m) = R(0,2)*tmp(18,m) + R(1,2)*tmp(19,m) + R(2,2)*tmp(20,m);

        retval(21,m) = R(0,0)*tmp(21,m) + R(1,0)*tmp(22,m) + R(2,0)*tmp(23,m);
        retval(22,m) = R(0,1)*tmp(21,m) + R(1,1)*tmp(22,m) + R(2,1)*tmp(23,m);
        retval(23,m) = R(0,2)*tmp(21,m) + R(1,2)*tmp(22,m) + R(2,2)*tmp(23,m);
      }
    return retval;
  }

//! @brief Devuelve la matriz de rigidez tangente expresada en globales.
void XC::ShellCrdTransf3dBase::getGlobalTangent(Matrix &stiff) const
{ //Por defecto ya está en globales.
  }

//! @brief Devuelve el vector residuo y la matriz de rigidez tangente expresados en globales.
void XC::ShellCrdTransf3dBase::getGlobalResidAndTangent(Vector &resid,Matrix &stiff) const
  { //Por defecto ya está en globales.
  }

//! @brief Devuelve los vectores expresados en coordenadas globales.
const XC::Matrix &XC::ShellCrdTransf3dBase::getVectorGlobalCoordFromLocal(const Matrix &localCoords) const
  {
    const Matrix &R= getTrfMatrix();
    static Matrix retval;
    const size_t numPts= localCoords.noRows(); //Número de vectores a transformar.
    retval.resize(numPts,3);
    for(size_t i= 0;i<numPts;i++)
      {
        // retval = Rlj'*localCoords (Multiplica el vector por R traspuesta).
        retval(i,0)= R(0,0)*localCoords(i,0) + R(1,0)*localCoords(i,1) + R(2,0)*localCoords(i,2);
        retval(i,1)= R(0,1)*localCoords(i,0) + R(1,1)*localCoords(i,1) + R(2,1)*localCoords(i,2);
        retval(i,2)= R(0,2)*localCoords(i,0) + R(1,2)*localCoords(i,1) + R(2,2)*localCoords(i,2);
      }
    return retval;
  }

//! @brief Devuelve el vector expresado en coordenadas locales.
const XC::Vector &XC::ShellCrdTransf3dBase::getVectorLocalCoordFromGlobal(const Vector &globalCoords) const
  {
    static Vector vectorCoo(3);
    const Matrix &R= getTrfMatrix();
    vectorCoo[0]= R(0,0)*globalCoords[0] + R(0,1)*globalCoords[1] + R(0,2)*globalCoords[2];
    vectorCoo[1]= R(1,0)*globalCoords[0] + R(1,1)*globalCoords[1] + R(1,2)*globalCoords[2];
    vectorCoo[2]= R(2,0)*globalCoords[0] + R(2,1)*globalCoords[1] + R(2,2)*globalCoords[2];
    return vectorCoo;
  }

//! @brief Devuelve un vector para almacenar los dbTags
//! de los miembros de la clase.
XC::DbTagData &XC::ShellCrdTransf3dBase::getDbTagData(void) const
  {
    static DbTagData retval(3);
    return retval;
  }

//! @brief Envia los miembros del objeto a través del canal que se pasa como parámetro.
int XC::ShellCrdTransf3dBase::sendData(CommParameters &cp)
  {
    int res=cp.sendVector(g1,getDbTagData(),CommMetaData(0));
    res+=cp.sendVector(g2,getDbTagData(),CommMetaData(1));
    res+=cp.sendVector(g3,getDbTagData(),CommMetaData(2));
    return res;
  }

//! @brief Recibe los miembros del objeto a través del canal que se pasa como parámetro.
int XC::ShellCrdTransf3dBase::recvData(const CommParameters &cp)
  {
    int res= cp.receiveVector(g1,getDbTagData(),CommMetaData(0));
    res+= cp.receiveVector(g2,getDbTagData(),CommMetaData(1));
    res+= cp.receiveVector(g3,getDbTagData(),CommMetaData(2));
    return res;    
  }

//! @brief Devuelve la propiedad del objeto cuyo código se pasa
//! como parámetro.
any_const_ptr XC::ShellCrdTransf3dBase::GetProp(const std::string &cod) const
  {
    if(cod == "getVPosCentroide")
      {
        tmp_gp_mdbl= vector_to_m_double(vpos_centroide);
        return any_const_ptr(tmp_gp_mdbl);
      }
    else if(cod == "getTrialDisp")
      {
        const int iNod= popInt(cod);
        tmp_gp_mdbl= vector_to_m_double((*theNodes)[iNod]->getTrialDisp());
        return any_const_ptr(tmp_gp_mdbl);
      }
    else if(cod == "getBasicTrialDisp")
      {
        const int iNod= popInt(cod);
        tmp_gp_mdbl= vector_to_m_double(getBasicTrialDisp(iNod));
        return any_const_ptr(tmp_gp_mdbl);
      }
    else if(cod == "getTrialVel")
      {
        const int iNod= popInt(cod);
        tmp_gp_mdbl= vector_to_m_double((*theNodes)[iNod]->getTrialVel());
        return any_const_ptr(tmp_gp_mdbl);
      }
    else if(cod == "getBasicTrialVel")
      {
        const int iNod= popInt(cod);
        tmp_gp_mdbl= vector_to_m_double(getBasicTrialVel(iNod));
        return any_const_ptr(tmp_gp_mdbl);
      }
    else if(cod == "getTrialAccel")
      {
        const int iNod= popInt(cod);
        tmp_gp_mdbl= vector_to_m_double((*theNodes)[iNod]->getTrialAccel());
        return any_const_ptr(tmp_gp_mdbl);
      }
    else if(cod == "getBasicTrialAccel")
      {
        const int iNod= popInt(cod);
        tmp_gp_mdbl= vector_to_m_double(getBasicTrialAccel(iNod));
        return any_const_ptr(tmp_gp_mdbl);
      }
    else
      return EntCmd::GetProp(cod);
  }