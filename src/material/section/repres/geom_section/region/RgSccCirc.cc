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
//RgSccCirc.cpp
// Written by Remo M. de Souza
// December 1998

#include <cmath>
#include <utility/matrix/Matrix.h>
#include <material/section/repres/geom_section/region/RgSccCirc.h>
#include <material/section/repres/cell/QuadCell.h>
#include "xc_utils/src/base/CmdStatus.h"
#include "xc_utils/src/base/any_const_ptr.h"
#include "xc_basic/src/texto/cadena_carac.h"
#include "xc_utils/src/geom/d2/poligonos2d/Poligono2d.h"
#include "xc_utils/src/geom/d2/SectorAnilloCircular2d.h"
#include "xc_utils/src/geom/d2/Rejilla2d.h"

const int I= 0, J=1, K= 2, L= 3; //Índices de los vértices.
const int Y= 0, Z=1; //Índices de las coordenadas.

//! @brief Constructor por defecto.
XC::RgSccCirc::RgSccCirc(Material *mat)
  : RgQuadCell(mat), centerPosit(2),intRad(0.0), extRad(0.0), initAng(0.0), finalAng(360.0)
  {}


XC::RgSccCirc::RgSccCirc(Material *mat, int numSubdivCircunf, int numSubdivRadial,
                     const XC::Vector &centerPosition, double internRadius, 
                     double externRadius, double initialAngle, double finalAngle)
  : RgQuadCell(mat,numSubdivCircunf,numSubdivRadial),
    centerPosit(centerPosition), intRad(internRadius), extRad(externRadius), 
    initAng(initialAngle), finalAng(finalAngle)
  {}

//! @brief Lee un objeto RgSccCirc desde archivo
bool XC::RgSccCirc::procesa_comando(CmdStatus &status)
  {
    const std::string cmd= deref_cmd(status.Cmd());
    if(verborrea>2)
      std::clog << "(RgSccCirc) Procesando comando: " << cmd << std::endl;

    if(cmd == "yCent") //Coordenada y del centro.
      {
        centerPosit(0)= interpretaDouble(status.GetString());
        return true;
      }
    else if(cmd == "zCent") //Coordenada z del centro.
      {
        centerPosit(1)= interpretaDouble(status.GetString());
        return true;
      }
    else if(cmd == "centro") //Coordenadas del centro.
      {
	std::deque<std::string> str_coord= separa_cadena(status.GetString(),",");
        if(str_coord.size()<2)
	  std::cerr << "Error; " << cmd 
		    << " se requieren dos coordenadas." << std::endl;
        else
          {
            centerPosit(0)= interpretaDouble(str_coord[0]);
            centerPosit(1)= interpretaDouble(str_coord[1]);
	  }
        return true;
      }
    else if(cmd == "intRad")
      {
        intRad= interpretaDouble(status.GetString());
        return true;
      }
    else if((cmd == "extRad") || (cmd == "Rad")) //Radio exterior.
      {
        extRad= interpretaDouble(status.GetString());
        return true;
      }
    else if(cmd == "initAng")
      {
        initAng= interpretaDouble(status.GetString());
        return true;
      }
    else if(cmd == "finalAng")
      {
        finalAng= interpretaDouble(status.GetString());
        return true;
      }
    else
      return RgQuadCell::procesa_comando(status);
  }

void XC::RgSccCirc::setCenterPosition(const Vector &centerPosition)
  { centerPosit = centerPosition; }

void XC::RgSccCirc::setCenterPos(const Pos2d &p)
  {
    centerPosit(0)= p.x();
    centerPosit(1)= p.y();
  }

Pos2d XC::RgSccCirc::getCenterPos(void) const
  { return Pos2d(centerPosit(0),centerPosit(1)); }

void XC::RgSccCirc::setRadii(double internRadius, double externRadius)
  {
    intRad = internRadius;
    extRad = externRadius;
  }

void XC::RgSccCirc::setAngles(double initialAngle, double finalAngle)
  {
    initAng  = initialAngle;
    finalAng = finalAngle;
  }

void XC::RgSccCirc::getRadii(double &internRadius, double &externRadius) const
  {
    internRadius = intRad;
    externRadius = extRad;
  }

void XC::RgSccCirc::getAngles(double &initialAngle, double &finalAngle) const
  {
    initialAngle = initAng;
    finalAngle   = finalAng;
  }

const XC::Matrix &XC::RgSccCirc::getVertCoords(void) const
  {
    static Matrix vertCoord(4,2);
    const double cosI= cos(initAng);
    const double cosF= cos(finalAng);
    const double sinI= sin(initAng);
    const double sinF= sin(finalAng);
    vertCoord(I,Y)= centerPosit(0) + intRad*cosI;
    vertCoord(I,Z)= centerPosit(1) + intRad*sinI;
    vertCoord(J,Y)= centerPosit(0) + extRad*cosI;
    vertCoord(J,Z)= centerPosit(1) + extRad*sinI;
    vertCoord(K,Y)= centerPosit(0) + extRad*cosF;
    vertCoord(K,Z)= centerPosit(1) + extRad*sinF;
    vertCoord(L,Y)= centerPosit(0) + intRad*cosF;
    vertCoord(L,Z)= centerPosit(1) + intRad*sinF;
    return vertCoord;
  }

double XC::RgSccCirc::getMaxY(void) const
  {
    std::cerr << "RgSccCirc::getMaxY no implementada." << std::endl;
    return 0.0;
  }
double XC::RgSccCirc::getMaxZ(void) const
  {
    std::cerr << "RgSccCirc::getMaxZ no implementada." << std::endl;
    return 0.0;
  }
double XC::RgSccCirc::getMinY(void) const
  {
    std::cerr << "RgSccCirc::getMinY no implementada." << std::endl;
    return 0.0;
  }
double XC::RgSccCirc::getMinZ(void) const
  {
    std::cerr << "RgSccCirc::getMinZ no implementada." << std::endl;
    return 0.0;
  }

const XC::Vector &XC::RgSccCirc::getCenterPosition(void) const
  { return centerPosit; }

//! @brief Devuelve un poligono inscrito en el sector del anillo circular.
Poligono2d XC::RgSccCirc::getPoligono(void) const
  { return getSector().getPoligono2d(nDivCirc()); }

SectorAnilloCircular2d &XC::RgSccCirc::getSector(void) const
  {
    static SectorAnilloCircular2d retval;
    Pos2d O(centerPosit(0),centerPosit(1));
    Circulo2d c(O,extRad);
    SectorCircular2d sc(c,initAng,finalAng);
    retval= SectorAnilloCircular2d(sc,intRad);
    return retval;
  }

//! @brief Devuelve la rejilla que corresponde a la discretización.
const Rejilla2d &XC::RgSccCirc::Malla(void) const
  { return alloc(Rejilla2d(getSector().Malla(nDivRad(),nDivCirc()))); }

const XC::VectorCells &XC::RgSccCirc::getCells(void) const
  {
    if(nDivRad() > 0  && nDivCirc() > 0)
      {
        Malla();
        int numCells  = this->getNumCells();

        cells.resize(numCells);

        int k= 0;
        XC::Matrix cellVertCoord(4,2);
        for(int j= 1;j<nDivRad()+1;j++)
          {
            for(int i= 1;i<nDivCirc()+1;i++)
              {
                cellVertCoord= getCellVertCoords(i,j);   //centerPosit(0) + rad_j  * cosTh1;
                cells.put(k,QuadCell(cellVertCoord)); 
                //std::cerr << "\ncreating cells XC::Cell " << k << " :" << cells[k];
                k++; 
              }
           }
       }
     return cells;
  }

//! @brief Constructor virtual.
XC::RegionSecc *XC::RgSccCirc::getCopy(void) const
  { return new XC::RgSccCirc(*this); }
 
//! \brief Devuelve la propiedad del objeto cuyo código (de la propiedad) se pasa
//! como parámetro.
any_const_ptr XC::RgSccCirc::GetProp(const std::string &cod) const
  {
    if(cod=="yCent") //Devuelve la coordenada y del centro.
      return any_const_ptr(centerPosit(0));
    if(cod=="zCent") //Devuelve la coordenada z del centro.
      return any_const_ptr(centerPosit(1));
    if(cod=="intRad") //Devuelve el radio interior.
      return any_const_ptr(intRad);
    if((cod=="extRad") ||(cod=="Rad") ) //Devuelve el radio exterior.
      return any_const_ptr(extRad);
    if(cod=="initAng") //Devuelve el ángulo inicial.
      return any_const_ptr(initAng);
    if(cod=="finalAng") //Devuelve el ángulo final.
      return any_const_ptr(finalAng);
    else
      return RgQuadCell::GetProp(cod);
  }

void XC::RgSccCirc::Print(std::ostream &s, int flag) const
  {
    s << "\nRgQuadCell Type: RgSccCirc";
    //s << "\nMaterial Id: " << getMaterialID();
    s << "\nNumber of subdivisions in the radial direction: " << nDivRad();
    s << "\nNumber of subdivisions in the circunferential direction: " << nDivCirc();
    s << "\nCenter Position: " << centerPosit;
    s << "\nInternal Radius: " << intRad << "\tExternal Radius: " << extRad;
    s << "\nInitial Angle: " << initAng << "\tFinal Angle: " << finalAng;
  }

std::ostream &XC::operator<<(std::ostream &s, XC::RgSccCirc &RgSccCirc)
  {
    RgSccCirc.Print(s);
    return s;
  }