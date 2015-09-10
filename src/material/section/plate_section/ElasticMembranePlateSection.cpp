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
                                                                        
// $Revision: 1.9 $
// $Date: 2003/02/14 23:01:33 $
// $Source: /usr/local/cvs/OpenSees/SRC/material/section/ElasticMembranePlateSection.cpp,v $

// Ed "C++" Love
//
//  Elastic Plate Section with membrane
//


#include <material/section/plate_section/ElasticMembranePlateSection.h>
#include "xc_utils/src/base/CmdStatus.h"
#include "xc_utils/src/base/any_const_ptr.h"
#include <cstdio> 
#include <cstdlib>
#include <cmath> 
#include "material/section/ResponseId.h"

//Deformaciones.

// De membrana.
// strain(0): epsilon_1 (Longitudinal eje 1)
// strain(1): epsilon_2 (Lontitudinal eje 2)
// strain(2): epsilon_12 (Cortante contenido en plano 12)

// De placa.
// strain(3): m_xx bending first axis.
// strain(4): m_yy bending second axis.
// strain(4): m_xy
// strain(6): shear first axis.
// strain(7): shear 2nd axis. 


//null constructor
XC::ElasticMembranePlateSection::ElasticMembranePlateSection(int tag)
  : ElasticPlateProto<8>(tag, SEC_TAG_ElasticMembranePlateSection) {}

//null constructor
XC::ElasticMembranePlateSection::ElasticMembranePlateSection(void)
  : ElasticPlateProto<8>(0,SEC_TAG_ElasticMembranePlateSection) {}


//full constructor
XC::ElasticMembranePlateSection::ElasticMembranePlateSection(int tag,double young,double poisson,double thickness,double r )
  : ElasticPlateProto<8>(tag, SEC_TAG_ElasticMembranePlateSection,young,poisson,thickness), rhoH(r*thickness)
  {}

//! @brief Lee un objeto XC::ElasticMembranePlateSection desde archivo
bool XC::ElasticMembranePlateSection::procesa_comando(CmdStatus &status)
  {
    const std::string cmd= deref_cmd(status.Cmd());
    if(verborrea>2)
      std::clog << "(ElasticMembranePlateSection) Procesando comando: " << cmd << std::endl;
    if(cmd == "rho")
      {
        rhoH= interpretaDouble(status.GetString());
        return true;
      }
    else
      return ElasticPlateProto<8>::procesa_comando(status);
  }

//! @brief make a clone of this material
XC::SectionForceDeformation*  XC::ElasticMembranePlateSection::getCopy(void) const
  { return new ElasticMembranePlateSection(*this); }

//! @brief density per unit area
double XC::ElasticMembranePlateSection::getRho(void) const
  { return rhoH; }

//! @brief assigns density per unit area
void XC::ElasticMembranePlateSection::setRho(const double &r)
  { rhoH= r; }


//! @brief Devuelve las etiquetas de los grados de libertad a los que el
//! elemento aporta rigidez.
const XC::ResponseId &XC::ElasticMembranePlateSection::getType(void) const 
  { return RespShellMat; }

//! @brief Devuelve la resultante de las tensiones en la sección.
const XC::Vector &XC::ElasticMembranePlateSection::getStressResultant(void) const
  {
    const double M= membraneModulus(); //membrane modulus
    const Vector &strain= getSectionDeformation();
    double G= shearModulus(); //shear modulus
    //membrane resultants
    stress(0)=  M*strain(0) + (nu*M)*strain(1);
    stress(1)=  (nu*M)*strain(0) +  M*strain(1);
    stress(2)=  G*strain(2);
    G*= five6;  //multiply by shear correction factor
    const double D=  bendingModulus();  //bending modulus

    //bending resultants
    stress(3)= -( D*strain(3) + nu*D*strain(4) );
    stress(4)= -( nu*D*strain(3) + D*strain(4) );
    stress(5)= -0.5*D*( 1.0 - nu )*strain(5);
    stress(6)= G*strain(6);
    stress(7)= G*strain(7);
    return stress;
  }


//! @brief Devuelve la matriz de rigidez tangente.
const XC::Matrix &XC::ElasticMembranePlateSection::getSectionTangent(void) const
  {
    const double M= membraneModulus(); //membrane modulus
    double G= shearModulus(); //shear modulus
    tangent.Zero();

    //membrane tangent terms

    tangent(0,0)= M;
    tangent(1,1)= M;

    tangent(0,1)= nu*M;
    tangent(1,0)= tangent(0,1);

    tangent(2,2)= G;

    G*= five6;  //multiply by shear correction factor

    const double D= bendingModulus();  //bending modulus

    //bending tangent terms

    tangent(3,3)= -D;
    tangent(4,4)= -D;

    tangent(3,4)= -nu*D;
    tangent(4,3)= tangent(3,4);

    tangent(5,5)= -0.5 * D * ( 1.0 - nu );
    tangent(6,6)= G;

    tangent(7,7)= G;

    return tangent;
  }


//! @brief Devuelve la matriz de rigidez noval.
const XC::Matrix &XC::ElasticMembranePlateSection::getInitialTangent(void) const
  {
    const double M= membraneModulus(); //membrane modulus
    double G= shearModulus(); //shear modulus
    tangent.Zero();

    //membrane tangent terms
    tangent(0,0)= M;
    tangent(1,1)= M;

    tangent(0,1)= nu*M;
    tangent(1,0)= tangent(0,1);

    tangent(2,2)= G;

    G*= five6;  //multiply by shear correction factor

    const double D=bendingModulus(); //bending modulus

    //bending tangent terms
    tangent(3,3)= -D;
    tangent(4,4)= -D;

    tangent(3,4)= -nu*D;
    tangent(4,3)= tangent(3,4);
    tangent(5,5)= -0.5*D*(1.0-nu);
    tangent(6,6)= G;
    tangent(7,7) = G;
    return this->tangent;
  }


//print out data
void  XC::ElasticMembranePlateSection::Print( std::ostream &s, int flag )
  {
    s << "ElasticMembranePlateSection: \n ";
    s <<  "  Young's Modulus E = "  <<  E  <<  std::endl;;
    s <<  "  Poisson's Ratio nu = " <<  nu <<  std::endl;;
    s <<  "  Thickness h = "        <<  h  <<  std::endl;;
    s <<  "  Density rho = "        <<  (rhoH/h)  <<  std::endl;;
    return;
  }

//! @brief Devuelve un vector para almacenar los dbTags
//! de los miembros de la clase.
XC::DbTagData &XC::ElasticMembranePlateSection::getDbTagData(void) const
  {
    static DbTagData retval(9);
    return retval;
  }

//! @brief Envía los datos a través del canal que se pasa como parámetro.
int XC::ElasticMembranePlateSection::sendData(CommParameters &cp)
  {
    int res= ElasticPlateProto<8>::sendData(cp);
    res+= cp.sendDouble(rhoH,getDbTagData(),CommMetaData(8));
    return res;
  }

//! @brief Recibe los datos a través del canal que se pasa como parámetro.
int XC::ElasticMembranePlateSection::recvData(const CommParameters &cp)
  {
    int res= ElasticPlateProto<8>::recvData(cp);
    res+= cp.receiveDouble(rhoH,getDbTagData(),CommMetaData(8));
    return res;
  }

int XC::ElasticMembranePlateSection::sendSelf(CommParameters &cp) 
  {
    setDbTag(cp);
    const int dataTag= getDbTag();
    inicComm(9);
    int res= sendData(cp);

    res+= cp.sendIdData(getDbTagData(),dataTag);
    if(res < 0)
      std::cerr << nombre_clase() << "sendSelf() - failed to send data\n";
    return res;
  }


int XC::ElasticMembranePlateSection::recvSelf(const CommParameters &cp)
  {
    inicComm(9);
    const int dataTag= getDbTag();
    int res= cp.receiveIdData(getDbTagData(),dataTag);

    if(res<0)
      std::cerr << nombre_clase() << "::recvSelf - failed to receive ids.\n";
    else
      {
        res+= recvData(cp);
        if(res<0)
          std::cerr << nombre_clase() << "::recvSelf - failed to receive data.\n";
      }
    return res;
  }
 
//! \brief Devuelve la propiedad del objeto cuyo código (de la propiedad) se pasa
//! como parámetro.
any_const_ptr XC::ElasticMembranePlateSection::GetProp(const std::string &cod) const
  {
    if(cod=="rho")
      return any_const_ptr(rhoH);
    else
      return ElasticPlateProto<8>::GetProp(cod);
  }