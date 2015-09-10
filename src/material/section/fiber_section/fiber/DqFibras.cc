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
//DqFibras.cc

#include "DqFibras.h"
#include "utility/matrix/Vector.h"
#include "material/section/KRSeccion.h"
#include "material/section/fiber_section/fiber/Fiber.h"
#include "material/section/fiber_section/FiberSection2d.h"
#include "material/section/fiber_section/FiberSection3dBase.h"
#include "material/section/fiber_section/FiberSection3d.h"
#include "material/section/fiber_section/FiberSectionGJ.h"
#include "material/section/repres/geom_section/GeomSection.h"
#include "xc_utils/src/base/CmdStatus.h"
#include "xc_utils/src/nucleo/CmdParser.h"
#include <material/uniaxial/UniaxialMaterial.h>
#include "xc_utils/src/base/any_const_ptr.h"
#include "xc_utils/src/base/utils_any_const_ptr.h"
#include "xc_utils/src/base/utils_any.h"
#include <domain/mesh/element/Information.h>
#include <utility/matrix/Vector.h>
#include "xc_utils/src/geom/sis_ref/Ref3d3d.h"
#include "xc_utils/src/geom/pos_vec/Vector3d.h"
#include "xc_utils/src/geom/pos_vec/Pos2d.h"
#include "xc_utils/src/geom/d2/BND2d.h"
#include "xc_utils/src/geom/d1/Recta2d.h"
#include "xc_utils/src/geom/d1/Segmento2d.h"
#include "xc_utils/src/geom/d2/Circulo2d.h"
#include "xc_utils/src/geom/d2/poligonos2d/Poligono2d.h"
#include "xc_utils/src/geom/d2/poligonos2d/bool_op_poligono2d.h"
#include "xc_utils/src/geom/d2/Semiplano2d.h"
#include "xc_utils/src/geom/listas/utils_list_pos2d.h"
#include "material/section/diag_interaccion/PlanoDeformacion.h"
#include "xc_utils/src/nucleo/InterpreteRPN.h"

//! @brief Constructor.
XC::DqFibras::DqFibras(const size_t &num)
  : EntCmd(), dq_ptr_fibras(num,static_cast<Fiber *>(nullptr)), yCDG(0.0), zCDG(0.0)
  {}

//! @brief Constructor de copia.
XC::DqFibras::DqFibras(const DqFibras &otro)
  : EntCmd(otro), dq_ptr_fibras(otro), yCDG(otro.yCDG), zCDG(otro.zCDG)
  {}

//! @brief Operador asignación.
XC::DqFibras &XC::DqFibras::operator=(const DqFibras &otro)
  {
    EntCmd::operator=(otro);
    dq_ptr_fibras::operator=(otro);
    yCDG= otro.yCDG;
    zCDG= otro.zCDG;
    return *this;
  }

//! @brief Agrega al contenedor la fibra que se pasa como parámetro.
XC::Fiber *XC::DqFibras::inserta(const Fiber &f)
  {
    Fiber *retval= f.getCopy();
    push_back(retval);
    return retval;
  }

//! @brief Agrega al contenedor el puntero a fibra que se pasa como parámetro.
void XC::DqFibras::push_back(Fiber *f)
   { dq_ptr_fibras::push_back(f); }


//! @brief Lee un objeto XC::DqFibras desde archivo
bool XC::DqFibras::procesa_comando(CmdStatus &status)
  {
    const std::string cmd= deref_cmd(status.Cmd());
    if(verborrea>2)
      std::clog << "(DqFibras) Procesando comando: " << cmd << std::endl;
    const CmdParser &parser= status.Parser();
    if(cmd == "for_each_fiber" || cmd == "for_each")
      { 
        ejecuta_bloque_for_each(status,status.GetBloque());
        return true;
      }
    else if(cmd == "fibra")
      {
        std::deque<boost::any> indices;
        if(parser.TieneIndices())
          {
            indices= parser.SeparaIndices(this);
            if(!indices.empty())
              {
                const int idx= convert_to_int(indices[0]);
                Fiber *f= (*this)[idx];
                if(f)
                  f->LeeCmd(status);
                else
                  std::cerr << "DqFibras::procesa_comando: '" << cmd
                            << "'; no se encontró la fibra de índice: " << idx
                            << std::endl;
              }
          }
        else
          std::cerr << "DqFibras::procesa_comando: '" << cmd
                    << "'; se necesita el índice de la fibra." << std::endl;
        return true;
      }
    else
      return EntCmd::procesa_comando(status);
  }

//! @brief Hace que cada una de las fibras ejecute el bloque que se pasa como parámetro.
void XC::DqFibras::ejecuta_bloque_for_each(CmdStatus &status,const std::string &bloque)
  {
    const std::string nmbBlq= nombre_clase()+":for_each";
    std::deque<Fiber *>::iterator i= begin();
    for(;i!= end();i++)
      if(*i)
        (*i)->EjecutaBloque(status,bloque,nmbBlq);
      else
        std::cerr << "DqFibras::ejecuta_bloque_for_each; Puntero a fibra nulo." << std::endl;
  }

//! @brief Busca la fibra cuyo tag se pasa como parámetro.
const XC::Fiber *XC::DqFibras::buscaFibra(const int &tag) const
  {
    const Fiber *retval= nullptr;
    std::deque<Fiber *>::const_iterator i= begin();
    for(;i!= end();i++)
      if((*i)->getTag() == tag)
        {
          retval= *i;
          break;
        }
    return retval;
  }

//! @brief Busca la fibra cuyo tag se pasa como parámetro.
XC::Fiber *XC::DqFibras::buscaFibra(const int &tag)
  {
    Fiber *retval= nullptr;
    std::deque<Fiber *>::iterator i= begin();
    for(;i!= end();i++)
      if((*i)->getTag() == tag)
        {
          retval= *i;
          break;
        }
    return retval;
  }

//! @brief Devuelve el valor mínimo de la coordenada y de las teselas.
double XC::DqFibras::GetYMin(void) const
  {
    double retval= 0.0;
    if(!empty())
      {
        register std::deque<Fiber *>::const_iterator i= begin();
        retval= (*i)->getLocY(); i++;
        for(;i!= end();i++)
          if(*i)
            retval= std::min(retval,(*i)->getLocY());
          else
            std::cerr << "DqFibras::GetYMin; Puntero a fibra nulo." << std::endl;
      }
    else
      std::cerr << "DqFibras::GetYMin; el conjunto de fibras está vacío." << std::endl;
    return retval;
  }

//! @brief Devuelve el valor mínimo de la coordenada z de las teselas.
double XC::DqFibras::GetZMin(void) const
  {
    double retval= 0.0;
    if(!empty())
      {
        register std::deque<Fiber *>::const_iterator i= begin();
        retval= (*i)->getLocZ(); i++;
        for(;i!= end();i++)
          if(*i)
            retval= std::min(retval,(*i)->getLocZ());
          else
            std::cerr << "DqFibras::GetZMin; Puntero a fibra nulo." << std::endl;
      }
    else
      std::cerr << "DqFibras::GetZMin; el conjunto de fibras está vacío." << std::endl;
    return retval;
  }

//! @brief Devuelve el valor máximo de la coordenada y de las teselas.
double XC::DqFibras::GetYMax(void) const
  {
    double retval= 0.0;
    if(!empty())
      {
        register std::deque<Fiber *>::const_iterator i= begin();
        retval= (*i)->getLocY(); i++;
        for(;i!= end();i++)
          if(*i)
            retval= std::max(retval,(*i)->getLocY());
          else
            std::cerr << "DqFibras::GetYMax; Puntero a fibra nulo." << std::endl;
      }
    else
      std::cerr << "DqFibras::GetYMax; el conjunto de fibras está vacío." << std::endl;
    return retval;
  }

//! @brief Devuelve el valor máximo de la coordenada z de las teselas.
double XC::DqFibras::GetZMax(void) const
  {
    double retval= 0.0;
    if(!empty())
      {
        register std::deque<Fiber *>::const_iterator i= begin();
        retval= (*i)->getLocZ(); i++;
        for(;i!= end();i++)
          if(*i)
            retval= std::max(retval,(*i)->getLocZ());
          else
            std::cerr << "DqFibras::GetZMax; Puntero a fibra nulo." << std::endl;
      }
    else
      std::cerr << "DqFibras::GetZMax; el conjunto de fibras está vacío." << std::endl;
    return retval;
  }

//! @brief Devuelve las posiciones de las teselas.
GeomObj::list_Pos2d XC::DqFibras::getPosiciones(void) const
  {
    GeomObj::list_Pos2d retval;
    if(!empty())
      {
        for(std::deque<Fiber *>::const_iterator i= begin();i!= end();i++)
          if(*i)
            retval.push_back(Pos2d((*i)->getLocY(),(*i)->getLocZ()));
          else
            std::cerr << "DqFibras::GetYMax; Puntero a fibra nulo." << std::endl;
      }
    return retval;
  }

//! @brief Devuelve la esquina superior derecha del rectángulo envolvente. 
Pos2d XC::DqFibras::GetPMax(void) const
  { return Pos2d(GetYMax(),GetZMax()); }

//! @brief Devuelve la esquina inferior izquierda del rectángulo envolvente. 
Pos2d XC::DqFibras::GetPMin(void) const
  { return Pos2d(GetYMin(),GetZMin()); }

//! @brief Devuelve el rectángulo envolvente. 
BND2d XC::DqFibras::Bnd(void) const
  { return BND2d(GetPMin(),GetPMax()); }

//! @brief Devuelve las suma de las áreas de las teselas.
double XC::DqFibras::getSumaAreas(const double &factor) const
  {
    double retval= 0.0;
    const size_t numFibers= getNumFibers();
    for(register size_t i=0;i<numFibers;i++)
      retval+= (*this)[i]->getArea();
    return factor*retval;
  }

//! @brief Devuelve el momento de inercia respecto al paralelo al z a una
//! distancia y0 del origen.
double XC::DqFibras::getIz(const double &factor,const double &y0) const
  {
    double retval= 0.0;
    register std::deque<Fiber *>::const_iterator i= begin();
    for(;i!= end();i++)
      if(*i)
        retval+= (*i)->getArea()*sqr((*i)->getLocY()-y0);
      else
        std::cerr << "DqFibras::getIz; Puntero a fibra nulo." << std::endl;
    retval*= factor;
    return retval;
  }

//! @brief Devuelve el momento de inercia respecto al eje paralelo al y a una
//! distancia z0 del origen.
double XC::DqFibras::getIy(const double &factor,const double &z0) const
  {
    double retval= 0.0;
    register std::deque<Fiber *>::const_iterator i= begin();
    for(;i!= end();i++)
      if(*i)
        retval+= (*i)->getArea()*sqr((*i)->getLocZ()-z0);
      else
        std::cerr << "DqFibras::getIy; Puntero a fibra nulo." << std::endl;
    retval*= factor;
    return retval;
  }

//! @brief Devuelve el producto de inercia respecto a los ejes paralelos aa una
//! distancias (y0,z0) del origen.
double XC::DqFibras::getPyz(const double &factor,const double &y0,const double &z0) const
  {
    double retval= 0.0;
    register std::deque<Fiber *>::const_iterator i= begin();
    for(;i!= end();i++)
      if(*i)
        retval+= (*i)->getArea()*((*i)->getLocZ()-z0)*((*i)->getLocY()-y0);
      else
        std::cerr << "DqFibras::getPyz; Puntero a fibra nulo." << std::endl;
    retval*= factor;
    return retval;
  }

//! @brief Devuelve el área de la sección homogeneizada.
double XC::DqFibras::getAreaSeccHomogeneizada(const double &E0) const
  {
    double retval= 0.0;

    for(std::deque<Fiber *>::const_iterator i= begin();i!= end();i++)
      {
        const UniaxialMaterial *mat= (*i)->getMaterial();
        if(mat)
          retval+= (*i)->getArea()*(mat->getTangent()/E0);
        else
          std::cerr << "DqFibras::getIySeccHomogeneizada; puntero a material nulo." << std::endl;
      }
    return retval;
  }

//! @brief Devuelve las coordenadas del CDG de la sección homogeneizada.
const XC::Vector &XC::DqFibras::getCdgSeccHomogeneizada(const double &E0) const
  {
    double Qy= 0.0,Qz= 0.0;
    double Atot= 0.0;
    double areaFibraPond= 0.0;
    double yLoc= 0.0, zLoc= 0.0;

    for(std::deque<Fiber *>::const_iterator i= begin();i!= end();i++)
      {
        yLoc= (*i)->getLocY();
        zLoc= (*i)->getLocZ();
        const UniaxialMaterial *mat= (*i)->getMaterial();
        if(mat)
          {
            areaFibraPond= (*i)->getArea()*(mat->getTangent()/E0);
            Atot+= areaFibraPond;
            Qz+= -yLoc*areaFibraPond; //Coordenada y cambiada de signo.
            Qy+= zLoc*areaFibraPond;
          }
        else
          std::cerr << "DqFibras::getIySeccHomogeneizada; puntero a material nulo." << std::endl;
      }
    static Vector retval(2);
    retval[0]= -Qz/Atot; //Coordenada y del CDG  XXX ¿Signo menos?
    retval[1]= Qy/Atot; //Coordenada z del CDG 
    return retval;
  }

//! @brief Devuelve el momento de inercia de la sección homogeneizada respecto al eje paralelo al y por el CDG.
//! @param E0: Módulo elástico de referencia.
double XC::DqFibras::getIySeccHomogeneizada(const double &E0) const
  {
    double retval= 0.0;
    const Vector &cdg= getCdgSeccHomogeneizada(E0);
    register std::deque<Fiber *>::const_iterator i= begin();
    for(;i!= end();i++)
      if(*i)
        {
          const UniaxialMaterial *mat= (*i)->getMaterial();
          if(mat)
            retval+= (*i)->getArea()*sqr((*i)->getLocZ()-cdg[1])*(mat->getTangent()/E0);
          else
	    std::cerr << "DqFibras::getIySeccHomogeneizada; puntero a material nulo." << std::endl;
        }
      else
        std::cerr << "DqFibras::getIySeccHomogeneizada; puntero a fibra nulo." << std::endl;
    return retval;
  }

//! @brief Devuelve el momento de inercia de la sección homogeneizada respecto al eje paralelo al z por el CDG.
//! @param E0: Módulo elástico de referencia.
double XC::DqFibras::getIzSeccHomogeneizada(const double &E0) const
  {
    double retval= 0.0;
    const Vector &cdg= getCdgSeccHomogeneizada(E0);
    register std::deque<Fiber *>::const_iterator i= begin();
    for(;i!= end();i++)
      if(*i)
        {
          const UniaxialMaterial *mat= (*i)->getMaterial();
          if(mat)
            retval+= (*i)->getArea()*sqr((*i)->getLocY()-cdg[0])*(mat->getTangent()/E0);
          else
	    std::cerr << "DqFibras::getIzSeccHomogeneizada; puntero a material nulo." << std::endl;
        }
      else
        std::cerr << "DqFibras::getIzSeccHomogeneizada; puntero a fibra nulo." << std::endl;
    return retval;
  }

//! @brief Devuelve el producto de inercia de la sección homogeneizada respecto a los ejes paralelos al y y al z por el CDG.
//! @param E0: Módulo elástico de referencia.
double XC::DqFibras::getPyzSeccHomogeneizada(const double &E0) const
  {
    double retval= 0.0;
    const Vector &cdg= getCdgSeccHomogeneizada(E0);
    register std::deque<Fiber *>::const_iterator i= begin();
    for(;i!= end();i++)
      if(*i)
        {
          const UniaxialMaterial *mat= (*i)->getMaterial();
          if(mat)
            retval+= (*i)->getArea()*((*i)->getLocZ()-cdg[1])*((*i)->getLocY()-cdg[0])*(mat->getTangent()/E0);
          else
	    std::cerr << "DqFibras::getPyzSeccHomogeneizada; puntero a material nulo." << std::endl;
        }
      else
        std::cerr << "DqFibras::getPyzSeccHomogeneizada; puntero a fibra nulo." << std::endl;
    return retval;
  }

//! @brief Devuelve la componente i,j del tensor de inercia calculado respecto al CDG.
double XC::DqFibras::getISeccHomogeneizada(const double &E0,const unsigned short int &i,const unsigned short int &j) const
  {
    unsigned short int k= i + (j-1)*2;
    double retval= 0.0;
    switch(k)
      {
        case 1: //(1,1)
          retval= getIySeccHomogeneizada(E0);
          break;
        case 2: //(2,1)
        case 3: //(1,2)
          retval= -getPyzSeccHomogeneizada(E0);
          break;
	case 4:
          retval= getIzSeccHomogeneizada(E0);
          break;
      }
    return retval;
  }

//! @brief Devuelve el tensor de inercia calculado desde el centro de gravedad del objeto.
XC::Matrix &XC::DqFibras::getISeccHomogeneizada(const double &E0) const
  {
    static Matrix i(2,2);
    i(0,0)= getIySeccHomogeneizada(E0); i(0,1)= -getPyzSeccHomogeneizada(E0);
    i(1,0)= i(0,1);   i(1,1)= getIzSeccHomogeneizada(E0);
    return i;
  }

//! @brief Devuelve el tensor de inercia respector al punto o.
XC::Matrix &XC::DqFibras::getISeccHomogeneizada(const double &E0,const Pos2d &o) const
  {
    static Matrix retval(2,2);
    const Matrix Ig= getISeccHomogeneizada(E0);
    Vector O(2); O[0]= o.x(); O[1]= o.y();
    const Vector og= getCdgSeccHomogeneizada(E0) - O;
    const double m= getAreaSeccHomogeneizada(E0);
    retval= Ig+m*(og.Norm2()*identity(Ig)-(og & og));
    return retval;
  }

//! @brief Devuelve el momento de inercia respecto al eje que pasa por O con dirección la de e.
double XC::DqFibras::getISeccHomogeneizada(const double &E0,const Pos2d &O,const Vector &e) const
  {
    const Matrix Io(getISeccHomogeneizada(E0,O));
    return dot(e,Io*e)/e.Norm2();
  }

//! @brief Devuelve el momento de inercia respecto a la recta que se pasa
//! como parámetro.
double XC::DqFibras::getISeccHomogeneizada(const double &E0,const Recta2d &r) const
  { return getISeccHomogeneizada(E0,r.Punto(),Vector(r.VDir()));  }

//! @brief Devuelve el momento estático de las áreas de las fibras situadas por encima de yf (y_fibra-yf > 0)
//! respecto al eje paralelo al z cuya y se pasa como parámetro (brazo= y_fibra-y0).
double XC::DqFibras::getSzPos(const double &yf,const double &y0,const double &factor) const
  {
    double retval= 0.0;
    register std::deque<Fiber *>::const_iterator i= begin();
    for(;i!= end();i++)
      if(*i)
        {
          const double y_fibra= (*i)->getLocY();
          if(y_fibra>yf)
            {
              const double brazo= y_fibra-y0;
              retval+= (*i)->getArea()*brazo;
            }
        }
      else
        std::cerr << "DqFibras::getSzPos; Puntero a fibra nulo." << std::endl;
    retval*= factor;
    return retval;
  }

//! @brief Devuelve el momento estático de las áreas de las fibras situadas por debajo de yf (y_fibra-yf < 0)
//! respecto al eje paralelo al z cuya y se pasa como parámetro (brazo= y_fibra-y0).
double XC::DqFibras::getSzNeg(const double &yf,const double &y0,const double &factor) const
  {
    double retval= 0.0;
    register std::deque<Fiber *>::const_iterator i= begin();
    for(;i!= end();i++)
      if(*i)
        {
          const double y_fibra= (*i)->getLocY();
          if(y_fibra<yf)
            {
              const double brazo= y_fibra-y0;
              retval+= (*i)->getArea()*brazo;
            }
        }
      else
        std::cerr << "DqFibras::getSzNeg; Puntero a fibra nulo." << std::endl;
    retval*= factor;
    return retval;
  }

//! @brief Devuelve el momento estático de las áreas de las fibras situada por encima de zf (z_fibra-zf > 0)
//! respecto al eje paralelo al y cuya z se pasa como parámetro (brazo= z_fibra-z0).
double XC::DqFibras::getSyPos(const double &zf,const double &z0,const double &factor) const
  {
    double retval= 0.0;
    register std::deque<Fiber *>::const_iterator i= begin();
    for(;i!= end();i++)
      if(*i)
        {
          const double z_fibra= (*i)->getLocZ();
          if(z_fibra>zf)
            retval+= (*i)->getArea()*(z_fibra-z0);
        }
      else
        std::cerr << "DqFibras::getSyPos; Puntero a fibra nulo." << std::endl;
    retval*= factor;
    return retval;
  }

//! @brief Devuelve el momento estático de las áreas de las fibras situadas por debajo  de zf (z_fibra-zf < 0)
//! respecto al eje paralelo al y cuya z se pasa como parámetro (brazo= z_fibra-z0).
double XC::DqFibras::getSyNeg(const double &zf,const double &z0,const double &factor) const
  {
    double retval= 0.0;
    register std::deque<Fiber *>::const_iterator i= begin();
    for(;i!= end();i++)
      if(*i)
        {
          const double z_fibra= (*i)->getLocZ();
          if(z_fibra<zf)
            retval+= (*i)->getArea()*(z_fibra-z0);
        }
      else
        std::cerr << "DqFibras::getSyNeg; Puntero a fibra nulo." << std::endl;
    retval*= factor;
    return retval;
  }

//! @brief Devuelve el momento estático de las áreas de las fibras situadas dentro
//! del semiplano que se pasa como parámetro.
double XC::DqFibras::getSPosSeccHomogeneizada(const double &E0,const Semiplano2d &sp) const
  {
    double retval= 0.0;
    double d= 0.0;
    for(std::deque<Fiber *>::const_iterator i= begin();i!= end();i++)
      {
        const UniaxialMaterial *mat= (*i)->getMaterial();
        if(mat)
          {
            d= -sp.DistSigno((*i)->getPos());
            if(d>0)
              retval+= (*i)->getArea()*(mat->getTangent()/E0)*d;
          }
        else
          std::cerr << "DqFibras::getSPosSeccHomogeneizada; puntero a material nulo." << std::endl;
      }
    return retval;
  }


//! @brief Devuelve el momento estático de las áreas de las fibras situadas fuera
//! del semiplano que se pasa como parámetro.
double XC::DqFibras::getSNegSeccHomogeneizada(const double &E0,const Semiplano2d &sp) const
  {
    double retval= 0.0;
    double d= 0.0;
    for(std::deque<Fiber *>::const_iterator i= begin();i!= end();i++)
      {
        const UniaxialMaterial *mat= (*i)->getMaterial();
        if(mat)
          {
            d= sp.DistSigno((*i)->getPos());
            if(d>0)
              retval+= (*i)->getArea()*(mat->getTangent()/E0)*d;
          }
        else
          std::cerr << "DqFibras::getSNegSeccHomogeneizada; puntero a material nulo." << std::endl;
      }
    return retval;
  }

//! @brief Funcion auxiliar para calcular los momentos principales de inercia.
inline double R(const double &Iy,const double &Iz,const double Pyz)
  { return sqrt(sqr((Iy-Iz)/2)+sqr(Pyz)); }

//! @brief Devuelve el momento de inercia principal mayor.
double XC::DqFibras::getI1(const double &factor,const double &y0,const double &z0) const
  {
    const double iy= getIy(factor,z0);
    const double iz= getIz(factor,y0);
    const double pyz= getPyz(factor,y0,z0);
    const double r= R(iy,iz,pyz);
    return (iy+iz)/2+r;
  }


//! @brief Devuelve el momento de inercia principal menor.
double XC::DqFibras::getI2(const double &factor,const double &y0,const double &z0) const
  {
    const double iy= getIy(factor,z0);
    const double iz= getIz(factor,y0);
    const double suma= (iy+iz)/2;
    const double pyz= getPyz();
    return suma-R(iy,iz,pyz);
  }

//! @brief Devuelve el ángulo que define un eje principal de inercia.
inline double theta_p(const double &Iy,const double &Iz,const double &Pyz)
  { return (atan(-2*Pyz/(Iy-Iz)))/2.0; }

//! @brief Devuelve el ángulo del eje del momento de inercia principal mayor con el eje y.
double XC::DqFibras::getTh1(const double &y0,const double &z0) const
  {
    const double Iy= getIy(1.0,z0);
    const double Iz= getIz(1.0,y0);
    const double Pyz= getPyz(1.0,y0,z0);
    double th1= theta_p(Iy,Iz,Pyz);
    const double media= (Iy+Iz)/2;
    const double dif2= (Iy-Iz)/2;
    const double th1_por2= 2*th1;
    const double seno= sin(th1_por2);
    const double coseno= cos(th1_por2);
    double i1= media+dif2*coseno-Pyz*seno;
    double i2= Iy+Iz-i1;
    if(i1<i2)
      {
        std::swap(i1,i2);
        th1+= (M_PI/2.0);
      }
    return th1;
  }

//! @brief Devuelve la dirección del eje del momento de inercia principal mayor.
XC::Vector XC::DqFibras::getEje1(const double &y0,const double &z0) const
  {
    Vector retval(2);
    const double th1= getTh1(y0,z0);
    retval[0]= cos(th1);
    retval[1]= sin(th1);
    return retval;
  }

//! @brief Devuelve la dirección del eje del momento de inercia principal menor.
XC::Vector XC::DqFibras::getEje2(const double &y0,const double &z0) const
  {
    Vector retval(2);
    const double th1= getTh1(y0,z0);
    retval[0]= sin(th1);
    retval[1]= -cos(th1);
    return retval;
  }

//! @brief Devuelve la resultante de tensiones.
double XC::DqFibras::Resultante(void) const
  {
    register double retval= 0;
    register std::deque<Fiber *>::const_iterator i= begin();
    for(;i!= end();i++)
      retval+= (*i)->getFuerza();
    return retval;
  }

//! @brief Devuelve el momento de la fuerza ejercida por las fibras
//! respecto al eje paralelo al «z» que pasa por y0
double XC::DqFibras::getMz(const double &y0) const
  {
    double retval= 0.0;
    register std::deque<Fiber *>::const_iterator i= begin();
    for(;i!= end();i++)
      retval+= (*i)->getMz(y0); 
    return retval;
  }

//! @brief Devuelve el momento de la fuerza ejercida por las fibras
//! respecto al eje  al eje paralelo al «y» que pasa por z0.
double XC::DqFibras::getMy(const double &z0) const
  {
    double retval= 0.0;
    register std::deque<Fiber *>::const_iterator i= begin();
    for(;i!= end();i++)
      retval+= (*i)->getMy(z0); 
    return retval;
  }

//! @brief Devuelve la excentricidad correspondiente al momento
//! de la fuerza ejercida por las fibras
//! respecto al eje paralelo al «z» que pasa por y0
double XC::DqFibras::getExcentricidadMz(const double &y0) const
  { return getMz(y0)/Resultante(); }

//! @brief Devuelve la excentricidad correspondiente al momento
//! de la fuerza ejercida por las fibras
//! respecto al eje  al eje paralelo al «y» que pasa por z0.
double XC::DqFibras::getExcentricidadMy(const double &z0) const
  { return getMy(z0)/Resultante(); }

//! @brief Devuelve el vector momento de la fuerza ejercida por las fibras
//! respecto a los ejes que pasan por (y0,z0).
Vector2d XC::DqFibras::getVectorMomento(const double &y0,const double &z0) const
  { return Vector2d(getMy(y0),getMz(y0)); }

//! @brief Devuelve el vector momento de las excentricidades fuerza ejercida por las fibras
//! respecto a los ejes que pasan por (y0,z0).
Pos2d XC::DqFibras::getPosResultante(const double &y0,const double &z0) const
  {
    const double R= Resultante();
    return Pos2d(getMz(y0)/R,getMy(z0)/R);
  }

//! @brief Devuelve la fibra neutra.
Recta2d XC::DqFibras::getFibraNeutra(void) const
  {
    const double R= Resultante();
    const double My= getMy();
    const double Mz= getMz();
    const Pos2d org(Mz/R,My/R);//Posición de la resultante.
    const Vector2d v(My,Mz); //Dirección de la fibra neutra.
    return Recta2d(org,v);
  }

//! @brief Devuelve verdadero si la sección está sometida
//! a momento flector.
bool XC::DqFibras::hayMomento(const double &tol) const
  {
    if(std::abs(getMz())>=tol)
      return true;
    else if(std::abs(getMy())>=tol)
      return true;
    else
      return false;
  }

//! @brief Devuelve la resultante de las compresiones en las fibras.
double XC::DqFibras::ResultanteComp(void) const
  {
    register double retval= 0;
    register double f= 0;
    register std::deque<Fiber *>::const_iterator i= begin();
    for(;i!= end();i++)
      if(*i)
        {
          f= (*i)->getFuerza();
          if(f<0.0) retval+= f;
        }
      else
        std::cerr << "DqFibras::Resultante; Puntero a fibra nulo." << std::endl;
    return retval;
  }

//! @brief Devuelve el momento de la compresión ejercida por las fibras
//! respecto al eje z.
double XC::DqFibras::getMzComp(const double &y0) const
  {
    double retval= 0.0;
    register std::deque<Fiber *>::const_iterator i= begin();
    for(;i!= end();i++)
      if((*i)->getFuerza()<0.0) retval+= (*i)->getMz(y0);
    return retval;
  }

//! @brief Devuelve el punto de paso de la resultante de tensiones menores
//! que «ref» (por defecto cero) compresiones, si no hay devuelve (0,0).
const XC::Vector &XC::DqFibras::baricentroCompresiones(void) const
  {
    static Vector retval(2);
    static double f,r;
    retval[0]= 0.0; retval[1]= 0.0; f= 0.0; r= 0.0;
    register std::deque<Fiber *>::const_iterator i= begin();
    for(;i!= end();i++)
      {
        f= (*i)->getFuerza();
        if(f<0.0)
          {
            r+= f;
            retval[0]+= (*i)->getMz();
            retval[1]+= (*i)->getMy();
          }
      }
    if(r!= 0.0)
      retval/= r;
    else
      { retval[0]= NAN; retval[1]= NAN; }
    return retval;
  }

//! @brief Devuelve el baricentro de las fibras cuya deformacion es
//! menor que la que se pasa como parámetro.
const XC::Vector &XC::DqFibras::baricentroDefMenores(const double &defRef) const
  {
    static Vector retval(2);
    static double def,r;
    retval[0]= 0.0; retval[1]= 0.0; def= 0.0; r= 0.0;
    register std::deque<Fiber *>::const_iterator i= begin();
    for(;i!= end();i++)
      {
        def=  (*i)->getDeformacion();
        if(def<defRef)
          {
            r+= (*i)->getFuerza();
            retval[0]+= (*i)->getMz();
            retval[1]+= (*i)->getMy();
          }
      }
    if(r!= 0.0)
      retval/= r;
    else
      { retval[0]= NAN; retval[1]= NAN; }
    return retval;
  }

//! @brief Devuelve el momento de la compresión ejercida por las fibras
//! respecto al eje y.
double XC::DqFibras::getMyComp(const double &z0) const
  {
    double retval= 0.0;
    register std::deque<Fiber *>::const_iterator i= begin();
    for(;i!= end();i++)
      if((*i)->getFuerza()<0.0) retval+= (*i)->getMy(z0);
    return retval;
  }

//! @brief Devuelve la resultante de las tracciones en las fibras.
double XC::DqFibras::ResultanteTracc(void) const
  {
    register double retval= 0;
    register double f= 0;
    register std::deque<Fiber *>::const_iterator i= begin();
    for(;i!= end();i++)
      if(*i)
        {
          f= (*i)->getFuerza();
          if(f>0.0) retval+= f;
        }
      else
        std::cerr << "DqFibras::Resultante; Puntero a fibra nulo." << std::endl;
    return retval;
  }

//! @brief Devuelve el momento de la tracción ejercida por las fibras
//! respecto al eje z.
double XC::DqFibras::getMzTracc(const double &y0) const
  {
    double retval= 0.0;
    register std::deque<Fiber *>::const_iterator i= begin();
    for(;i!= end();i++)
      if((*i)->getFuerza()>0.0) retval+= (*i)->getMz(y0);
    return retval;
  }

//! @brief Devuelve el momento de la tracción ejercida por las fibras
//! respecto al eje y.
double XC::DqFibras::getMyTracc(const double &z0) const
  {
    double retval= 0.0;
    register std::deque<Fiber *>::const_iterator i= begin();
    for(;i!= end();i++)
      if((*i)->getFuerza()>0.0) retval+= (*i)->getMy(z0);
    return retval;
  }

//! @brief Devuelve el punto de paso de la resultante de tracciones, si
//! no hay tracciones devuelve (0,0).
const XC::Vector &XC::DqFibras::baricentroTracciones(void) const
  {
    static Vector retval(2);
    static double f,r;
    retval[0]= 0.0; retval[1]= 0.0; f= 0.0; r= 0.0;
    register std::deque<Fiber *>::const_iterator i= begin();
    for(;i!= end();i++)
      {
        f= (*i)->getFuerza();
        if(f>0)
          {
            r+= f;
            retval[0]+= (*i)->getMz();
            retval[1]+= (*i)->getMy();
          }
      }
    if(r!= 0.0)
      retval/= r;
    else
      { retval[0]= NAN; retval[1]= NAN; }
    return retval;
  }

//! @brief Devuelve el baricentro de las fibras cuya deformacion es
//! mayor que la que se pasa como parámetro.
const XC::Vector &XC::DqFibras::baricentroDefMayores(const double &defRef) const
  {
    static Vector retval(2);
    static double def,r;
    retval[0]= 0.0; retval[1]= 0.0; def= 0.0; r= 0.0;
    register std::deque<Fiber *>::const_iterator i= begin();
    for(;i!= end();i++)
      {
        def=  (*i)->getDeformacion();
        if(def>defRef)
          {
            r+= (*i)->getFuerza();
            retval[0]+= (*i)->getMz();
            retval[1]+= (*i)->getMy();
          }
      }
    if(r!= 0.0)
      retval/= r;
    else
      { retval[0]= NAN; retval[1]= NAN; }
    return retval;
  }

//! @brief Devuelve el índice de la fibra más proxima a la posición que se pasa como parámetro.
size_t XC::DqFibras::nearest_fiber(const double &y,const double &z) const
  {
    size_t retval= 0;
    const size_t nf= getNumFibers();
    if(nf<1)
      {
        std::cerr << "DqFibras::nearest_fiber; No hay fibras definidas." << std::endl;
        return retval;
      }
    register size_t i= 0;
    Fiber *f= (*this)[i];
    double yf= 0.0;
    double zf= 0.0;
    if(f)
      f->getFiberLocation(yf,zf);
    else
      std::cerr << "DqFibras::nearest_fiber; Puntero a fibra nulo." << std::endl;
    double d2= sqr(yf-y)+sqr(zf-z);
    i++;
    for(;i<nf;i++)
      if((*this)[i])
        {
          (*this)[i]->getFiberLocation(yf,zf);
          double d2i= sqr(yf-y)+sqr(zf-z);
          if(d2i<d2)
            {
              d2= d2i;
              retval=i;
            }
        }
      else
        std::cerr << "DqFibras::nearest_fiber; Puntero a fibra nulo." << std::endl;
    return retval;
  }

//! Devuelve el subconjunto de fibras de éste que cumplen la condición que se
//! pasa como parámetro.
void XC::DqFibras::Cumplen(const std::string &cond,DqFibras &retval,bool clear)
  {
    if(clear) retval.clear();
    std::deque<Fiber *>::iterator i= begin();
    for(;i!= end();i++)
      if(*i)
        {
          if((*i)->verdadero(cond))
            retval.push_back(*i);
        }
      else
        std::cerr << "DqFibras::Cumplen; Puntero a fibra nulo." << std::endl;
    retval.updateCDG();
  }

//! Devuelve el subconjunto de fibras de éste cuyo material tiene por tag el que se
//! pasa como parámetro.
void XC::DqFibras::SelMatTag(const int &matTag,DqFibras &retval,bool clear)
  {
    if(clear) retval.clear();
    std::deque<Fiber *>::iterator i= begin();
    for(;i!= end();i++)
      if(*i)
        {
          Material *mat= (*i)->getMaterial();
          if(mat)
            {
              const int tag= mat->getTag();
              if(tag == matTag)
                retval.push_back(*i);
            }
        }
      else
        std::cerr << "DqFibras::SelMatTag; Puntero a fibra nulo." << std::endl;
  }

//! @brief Devuelve la deformación mínima.
double XC::DqFibras::getStrainMin(void) const
  {
    double retval= 0.0;
    if(!empty())
      {
        std::deque<Fiber *>::const_iterator i= begin();
        retval= (*i)->getMaterial()->getStrain();
        i++;
        for(;i!= end();i++)
          retval= std::min(retval,(*i)->getMaterial()->getStrain());
      }
    return retval;
  }

//! @brief Devuelve la deformación mínima.
double XC::DqFibras::getStrainMax(void) const
  {
    double retval= 0.0;
    if(!empty())
      {
        std::deque<Fiber *>::const_iterator i= begin();
        retval= (*i)->getMaterial()->getStrain();
        i++;
        for(;i!= end();i++)
          retval= std::max(retval,(*i)->getMaterial()->getStrain());
      }
    return retval;
  }

//! @brief Devuelve la deformación media.
double XC::DqFibras::getStrainMed(void) const
  {
    double retval= 0.0;
    double a= 0.0;
    if(!empty())
      {
        for(std::deque<Fiber *>::const_iterator i= begin();i!= end();i++)
          {
            retval+= (*i)->getArea()*(*i)->getMaterial()->getStrain();
            a+= (*i)->getArea();
          }
        retval/=a;
      }
    return retval;
  }

//! @brief Devuelve el plano de deformaciones ajustado por mínimos cuadrados.
XC::PlanoDeformacion XC::DqFibras::getPlanoDeformacion(void) const
  {
    GeomObj::list_Pos3d puntos;
    if(!empty())
      {
        for(std::deque<Fiber *>::const_iterator i= begin();i!= end();i++)
          if(*i)
            puntos.push_back(Pos3d((*i)->getMaterial()->getStrain(),(*i)->getLocY(),(*i)->getLocZ()));
          else
            std::cerr << "DqFibras::getDeformationPlane; Puntero a fibra nulo." << std::endl;
      }
    PlanoDeformacion retval;
    retval.AjusteMinimosCuadrados(puntos);
    return retval;
  }

//! @brief Devuelve el vector de deformaciones.
const XC::Vector &XC::DqFibras::getDeformation(void) const
  {
    const PlanoDeformacion pDef= getPlanoDeformacion();
    return pDef.getDeformation();
  }

//! @brief Devuelve la tensión mínima.
double XC::DqFibras::getStressMin(void) const
  {
    double retval= 0.0;
    if(!empty())
      {
        std::deque<Fiber *>::const_iterator i= begin();
        retval= (*i)->getMaterial()->getStress();
        i++;
        for(;i!= end();i++)
          retval= std::min(retval,(*i)->getMaterial()->getStress());
      }
    return retval;
  }

//! @brief Devuelve la tensión mínima.
double XC::DqFibras::getStressMax(void) const
  {
    double retval= 0.0;
    if(!empty())
      {
        std::deque<Fiber *>::const_iterator i= begin();
        retval= (*i)->getMaterial()->getStress();
        i++;
        for(;i!= end();i++)
          retval= std::max(retval,(*i)->getMaterial()->getStress());
      }
    return retval;
  }

//! @brief Devuelve la tensión media.
double XC::DqFibras::getStressMed(void) const
  {
    double retval= 0.0;
    double a= 0.0;
    if(!empty())
      {
        for(std::deque<Fiber *>::const_iterator i= begin();i!= end();i++)
          {
            retval+= (*i)->getArea()*(*i)->getMaterial()->getStress();
            a+= (*i)->getArea();
          }
        retval/=a;
      }
    return retval;
  }

XC::ClaseEsfuerzo XC::DqFibras::getClaseEsfuerzo(const double &tol) const
  {
    ClaseEsfuerzo retval= ERROR;
    const double epsMin= getStrainMin();
    const double epsMax= getStrainMax();
    if(epsMin>0) //Tracción.
      {
        const double r= (epsMax-epsMin)/epsMax;
        if(r<tol)
          retval= TRACCION_SIMPLE;
        else
          retval= TRACCION_COMPUESTA;
      }
    else if(epsMax>0) //Flexión.
      {
        const double R= Resultante()/epsMax;
        if(fabs(R)>tol)
          retval= FLEXION_COMPUESTA;
        else
          retval= FLEXION_SIMPLE;
      }
    else
      {
        const double r= (epsMin-epsMax)/epsMin;
        if(r<tol)
          retval= COMPRESION_SIMPLE;
        else
          retval= COMPRESION_COMPUESTA;
      }
    return retval;
  }

//! @brief Devuelve verdadero si la sección está en tracción simple o compuesta.
bool XC::DqFibras::enTraccion(void) const
  { return ((getStrainMin()>=0) && (getStrainMax()>0)); }

//! @brief Devuelve verdadero si la sección está en flexión simple o compuesta.
bool XC::DqFibras::enFlexion(void) const
  { return ((getStrainMin()<0) && (getStrainMax()>0)); }

//! @brief Devuelve verdadero si la sección está en compresión simple o compuesta.
bool XC::DqFibras::enCompresion(void) const
  { return ((getStrainMin()<0) && (getStrainMax()<=0)); }

std::string XC::DqFibras::getStrClaseEsfuerzo(const double &tol) const
  {
    std::string retval= "error";
    const ClaseEsfuerzo clase= getClaseEsfuerzo(tol);
    switch(clase)
      {
      case TRACCION_SIMPLE:
	retval= "traccion_simple";
        break;
      case TRACCION_COMPUESTA:
	retval= "traccion_compuesta";
        break;
      case FLEXION_SIMPLE:
	retval= "flexion_simple";
        break;
      case FLEXION_COMPUESTA:
	retval= "flexion_compuesta";
        break;
      case COMPRESION_SIMPLE:
	retval= "compresion_simple";
        break;
      case COMPRESION_COMPUESTA:
	retval= "compresion_compuesta";
        break;
      case ERROR:
	retval= "error";
        break;
      }
    return retval;
  }

//! @brief Devuelve la posición del centro de gravedad.
Pos2d XC::DqFibras::getCdg(void) const
  { return Pos2d(getYCdg(),getZCdg()); }

//! @brief Devuelve la profundidad de la fibra neutra, es decir la distancia desde
//! la fibra mas comprimida a la fibra neutra.
//! El algoritmo empleado se basa en que get_dist_to_neutral_axis devuelve distancias negativas
//! para fibras comprimidas (deformación negativa).
double XC::DqFibras::getNeutralAxisDepth(const FiberSectionBase &Section) const
  {
    double retval= 0.0;
    if(empty())
      return retval;

    std::deque<Fiber *>::const_iterator i= begin();
    double yLoc= (*i)->getLocY();
    double zLoc= (*i)->getLocZ();
    retval= Section.get_dist_to_neutral_axis(yLoc,zLoc);
    if(!isnan(retval))
      {
        i++;
        double tmp;
        for(;i!= end();i++)
          {
            yLoc= (*i)->getLocY(); 
            zLoc= (*i)->getLocZ();
            tmp= Section.get_dist_to_neutral_axis(yLoc,zLoc);
            retval= std::min(retval,tmp);
          }
      }
    return retval;
  }

//! @brief Devuelve un vector orientado desde el centro de tracciones al de compresiones.
XC::Vector XC::DqFibras::getVectorBrazoMecanico(void) const
  {
    const Segmento2d &bm= getSegmentoBrazoMecanico();
    return Vector(bm.GetVector());
  }

//! @brief Devuelve un segmento orientado desde el centro de tracciones al de compresiones.
Segmento2d XC::DqFibras::getSegmentoBrazoMecanico(void) const
  {
    Segmento2d retval;
    const double epsMin= getStrainMin();
    const double epsMax= getStrainMax();
    if((epsMin<0) && (epsMax>0))
      {
        const Vector &C= baricentroCompresiones();
        const Vector &T= baricentroTracciones();
        retval= Segmento2d(Pos2d(T[0],T[1]),Pos2d(C[0],C[1]));
      }
    else if(std::abs(epsMax-epsMin)>1e-6)
      {
        const double defRef= (epsMin+epsMax)/2.0;
        const Vector &C= baricentroDefMenores(defRef);
        const Vector &T= baricentroDefMayores(defRef);
        retval= Segmento2d(Pos2d(T[0],T[1]),Pos2d(C[0],C[1]));
      }
    else
      retval.setExists(false);
    return retval;
  }

//! @brief Devuelve la traza del plano de flexión en el plano de la sección.
Recta2d XC::DqFibras::getTrazaPlanoFlexion(void) const
  { return getSegmentoBrazoMecanico().RectaSoporte(); }

//! @brief Devuelve la traza de un plano perpendicular al de flexión
//! que pasa por el baricentro de tracciones.
Recta2d XC::DqFibras::getTrazaPlanoTraccion(void) const
  { 
    const Recta2d trazaFlexion= getTrazaPlanoFlexion();
    Pos2d pt(getYCdg(),getZCdg());
    const double epsMax= getStrainMax();
    if(epsMax>0) //Thera are tractions.
      {
        const Vector &T= baricentroTracciones();
        pt= Pos2d(T[0],T[1]);
      }
    return trazaFlexion.Perpendicular(pt);
  }

//! @brief Devuelve la traza de un plano perpendicular al de flexión
//! que pasa por el baricentro de compresiones.
Recta2d XC::DqFibras::getTrazaPlanoCompresion(void) const
  { 
    const Recta2d trazaFlexion= getTrazaPlanoFlexion();
    Pos2d pt(getYCdg(),getZCdg());
    const double epsMin= getStrainMin();
    if(epsMin<0) //There are compresions.
      {
        const Vector &C= baricentroCompresiones();
        pt= Pos2d(C[0],C[1]);
      }
    return trazaFlexion.Perpendicular(pt);
  }

//! @brief Devuelve el brazo mecánico con el que está trabajando la sección.
double XC::DqFibras::getBrazoMecanico(void) const
  { return getVectorBrazoMecanico().Norm(); }

//! @brief Devuelve la distancia media entre fibras.
double XC::DqFibras::getDistMediaFibras(void) const
  {
    double retval= 0;
    const size_t sz= seps.size();
    if(sz>0)
      {
        for(size_t i= 0;i<sz;i++)
          retval+= seps[i];
        retval/=sz;
      }
    return retval;
  }

//! @brief Calcula las áreas eficaces a fisuración en torno a las fibras que representan la armadura
//! (ver artículo 49.2.4 de la EHE-08 (área rallada figura 49.2.4b).
//! Ver también figuras 47.5 y 47.6 del tomo II del libro "Proyecto y cálculo de estructuras
//! de hormigón" de Calavera.
//En lugar de un cuadrado se considera un dodecágono del mismo área
//en torno al eje de la fibra. Creo que esto es menos anisótropo.
//! @param contornoAcEficazBruta: Contorno del área eficaz bruta obtenido de la sección.
double XC::DqFibras::calcAcEficazFibras(const std::list<Poligono2d> &contornoAcEficazBruta,const double &factor) const
  {
    double retval= 0.0;
    const size_t n= 12;
    double dm,L,R;
    const size_t sz= size();
    dq_ac_eficaz.clear();
    dq_ac_eficaz.resize(sz);
    Poligono2d tmp;
    for(size_t i= 0;i<sz;i++)
      {
        dm= getDiamEqFibra(i);
        L= factor*dm; //Lado del cuadrado prescrito por la norma.
        R= L*sqrt(2/(n*sin(2*M_PI/n)));
        const Pos2d pos= (*this)[i]->getPos();
        tmp= Circulo2d(pos,R).getPoligonoInscrito(n);
        if(tmp.Overlap(contornoAcEficazBruta))
          for(std::list<Poligono2d>::const_iterator j= contornoAcEficazBruta.begin();j!=contornoAcEficazBruta.end();j++)
            {
              if((*j).In(tmp))
                dq_ac_eficaz[i].push_back(tmp);
              else if((*j).Overlap(tmp))
                {
                  std::list<Poligono2d> tmpLst= interseccion(tmp,*j);
                  assert(!tmpLst.empty());
                  dq_ac_eficaz[i].insert(dq_ac_eficaz[i].end(),tmpLst.begin(),tmpLst.end());
                }
            }
      }

    //Recortar intersecciones.
    for(size_t i= 0;i<sz;i++)
      {
	std::list<Poligono2d> &p1= dq_ac_eficaz[i];
        for(size_t j= i+1;j<sz;j++)
          {
	    std::list<Poligono2d> &p2= dq_ac_eficaz[j];
            if(overlap(p1,p2))
              {
                const Pos2d c1= (*this)[i]->getPos();
                const Pos2d c2= (*this)[j]->getPos();
                particiona(c1,p1,c2,p2);
              }
          }
      }
    for(size_t i= 0;i<sz;i++)
      {
        if(!dq_ac_eficaz[i].empty())
          retval+= area(dq_ac_eficaz[i].begin(),dq_ac_eficaz[i].end());
      }
    const double area_contorno= area(contornoAcEficazBruta.begin(),contornoAcEficazBruta.end());
    if(retval>1.01*area_contorno)
      std::cerr << "El área eficaz: " << retval
                << " es mayor que la máxima: " << area_contorno << std::endl;
    return retval;
  }

//! @brief Devuelve los contornos del área eficaz de la fibra cuyo índice
//! se pasa como parámetro.
const std::list<Poligono2d> &XC::DqFibras::getContornoAcEficazFibra(const size_t &i) const
  { return dq_ac_eficaz[i]; }

//! @brief Devuelve el valor del área eficaz de la fibra cuyo índice
//! se pasa como parámetro.
double XC::DqFibras::getAcEficazFibra(const size_t &i) const
  {
    if(dq_ac_eficaz.size()!=size())
      std::cerr << "DqFibras::getAcEficazFibra; no se han calculado las áreas eficaces."
                << std::endl;
    return area(dq_ac_eficaz[i].begin(),dq_ac_eficaz[i].end());
  }

//! @brief Devuelve la suma de las áreas eficaces de las fibras.
double XC::DqFibras::getAcEficazFibras(void) const
  {
    const size_t sz= dq_ac_eficaz.size();
    if(sz!=size())
      std::cerr << "DqFibras::getAcEficazFibras; no se han calculado las áreas eficaces."
                << std::endl;
    double retval= 0;
    for(size_t i= 0;i<sz;i++)
      retval+= area(dq_ac_eficaz[i].begin(),dq_ac_eficaz[i].end());
    return retval;
  }

//! @brief Calcula los recubrimientos de las fibras.
void XC::DqFibras::calcRecubrimientos(const GeomSection &g) const
  {
    const GeomObj::list_Pos2d posiciones= getPosiciones();
    const Poligono2d contorno= g.getContornoRegiones();
    recubs= getRecubrimientos(posiciones,contorno);
    const size_t sz= recubs.size();
    for(size_t i= 0;i<sz;i++)
      if(recubs[i]<0)
        std::clog << "¡Ojo! la posición: " << posiciones[i]
                  << " está fuera de la sección." << std::endl;
  }

//! @brief Calcula la distancia de cada fibra a la más proxima.
void XC::DqFibras::calcSeparaciones(void) const
  { seps= getPosiciones().GetSeparaciones(); }

//! @brief Devuelve el valor del recubrimiento de la fibra cuyo índice
//! se pasa como parámetro.
const double &XC::DqFibras::getRecubrimientoFibra(const size_t &i) const
  {
    if(recubs.size()!=size())
      std::cerr << "DqFibras::getRecubrimientoFibra; no se han calculado los recubrimientos."
                << std::endl;
    return recubs[i];
  }

//! @brief Devuelve el valor de la separación recubrimiento de la fibra cuyo índice
//! se pasa como parámetro.
const double &XC::DqFibras::getSeparacionFibra(const size_t &i) const
  {
    if(seps.size()!=size())
      std::cerr << "DqFibras::getSeparacionFibra; no se han calculado las separaciones."
                << std::endl;
    return seps[i];
  }

//! @brief Devuelve el valor del diámetro que tendría la fibra si
//! su área fuera circular.
double XC::DqFibras::getDiamEqFibra(const size_t &i) const
  { return 2.0*sqrt((*this)[i]->getArea()/M_PI); }



//! @brief Devuelve la tensión en la fibra i en el instante en el que
//! se produce la fisuración de su área eficaz.
double XC::DqFibras::getSigmaSRFibra(const size_t &i,const double &Ec,const double &Es,const double &fctm) const
  {
    double retval= 0.0;
    const double Ag= getAcEficazFibra(i);
    if(Ag>0.0)
      {
        const double As= (*this)[i]->getArea();
        const double ro= As/Ag;
        const double n= Es/Ec;
        retval= (1-ro+n*ro)*Ag*fctm/As;
      }
    return retval;
  }

//! @brief Actualiza el CDG.
int XC::DqFibras::updateCDG(void)
  {
    double Qy= 0.0,Qz= 0.0;
    double Atot= 0.0;
    double areaFibra= 0.0;
    double yLoc= 0.0, zLoc= 0.0;

    // Recompute centroid
    for(std::deque<Fiber *>::iterator i= begin();i!= end();i++)
      {
        yLoc= (*i)->getLocY();
        zLoc= (*i)->getLocZ();
        areaFibra= (*i)->getArea();
        Atot+= areaFibra;
        Qz+= -yLoc*areaFibra; //Coordenada y cambiada de signo.
        Qy+= zLoc*areaFibra;
      }
    yCDG= -Qz/Atot; //Coordenada y del CDG  XXX ¿Signo menos?
    zCDG= Qy/Atot; //Coordenada z del CDG 
    return 0;
  }

//! @brief Actualiza los parámetros CDG, rigidez y resultante.
int XC::DqFibras::updateKRCDG(FiberSection2d &Section2d,KRSeccion &kr2)
  {
    kr2.zero();
    double Qz= 0.0;
    double Atot= 0.0;//!< Área total de las fibras.
    double areaFibra= 0.0;//!< Área de una fibra.
    double yLoc= 0.0, zLoc= 0.0;

    double tangent,stress,fs0;
    // Recompute centroid
    std::deque<Fiber *>::iterator i= begin();
    for(;i!= end();i++)
      {
        (*i)->getFiberLocation(yLoc, zLoc);
        areaFibra= (*i)->getArea();
        if(areaFibra!= 0.0)
          {
            Atot+= areaFibra;
            Qz+= -yLoc*areaFibra; //Coordenada y cambiada de signo.

            //Actualizamos matriz de rigidez.
            tangent= (*i)->getMaterial()->getTangent();
            kr2.updateK2d(areaFibra,yLoc,tangent);

            //Actualizamos resultante de tensiones.
            stress= (*i)->getMaterial()->getStress();
            fs0= stress * areaFibra;
            kr2.updateNMz(fs0,yLoc);
          }
      }
    yCDG= -Qz/Atot; //Coordenada y del CDG 
    kr2.kData[2]= kr2.kData[1]; //Simetría.
    return 0;
  }

//! @brief Añade una fibra XXX Mejorar actualización de parámetros.
XC::Fiber *XC::DqFibras::addFiber(FiberSection2d &Section2d,Fiber &newFiber,KRSeccion &kr2)
  {
    Fiber *retval= inserta(newFiber);
    updateKRCDG(Section2d,kr2);
    return retval;
  }

int XC::DqFibras::commitState(void)
  {
    int err= 0;
    std::deque<Fiber *>::iterator i= begin();
    for(;i!= end();i++)
      err+= (*i)->commitState();
    return err;
  }

//! @brief Establece los valores de las deformaciones iniciales.
int XC::DqFibras::setInitialSectionDeformation(const FiberSection2d &Section2d)
  {
    int retval= 0;
    UniaxialMaterial *theMat;
    double y; 
    std::deque<Fiber *>::iterator i= begin();
    for(;i!= end();i++)
      {
        theMat= (*i)->getMaterial();
        y= (*i)->getLocY();
        // determine material strain and set it
        retval+= theMat->setInitialStrain(Section2d.get_strain(y));
      }
    return retval;
  }

//! @brief Establece los valores de las deformaciones de prueba.
int XC::DqFibras::setTrialSectionDeformation(const FiberSection2d &Section2d,KRSeccion &kr2)
  {
    int retval= 0;
    kr2.zero();
    UniaxialMaterial *theMat;
    double y,areaFibra,strain,tangent,stress, fs0; 
    std::deque<Fiber *>::iterator i= begin();
    for(;i!= end();i++)
      {
        theMat= (*i)->getMaterial();
        y= (*i)->getLocY();
        areaFibra= (*i)->getArea();
        if(areaFibra!=0.0)
          {
            // determine material strain and set it
            strain= Section2d.get_strain(y);
            retval+= theMat->setTrial(strain, stress, tangent);

            //Actualizamos matriz de rigidez.
            kr2.updateK2d(areaFibra,y,tangent);

            //Actualizamos resultante de tensiones.
            fs0= stress * areaFibra;
            kr2.updateNMz(fs0,y);
          }
      }
    kr2.kData[2]= kr2.kData[1]; //Simetría.
    return retval;
  }

//! @brief Devuelve el estado de las fibras al del último commit.
int XC::DqFibras::revertToLastCommit(FiberSection2d &Section2d,KRSeccion &kr2)
  {
    int err= 0;
    kr2.zero();
    std::deque<Fiber *>::iterator i= begin();
    for(;i!= end();i++)
      err+= (*i)->getMaterial()->revertToLastCommit();
    err+= updateKRCDG(Section2d,kr2);
    return err;
  }

//! @brief Devuelve las fibras a su estado inicial.
int XC::DqFibras::revertToStart(FiberSection2d &Section2d,KRSeccion &kr2)
  {
    int err= 0;
    kr2.zero();
    std::deque<Fiber *>::iterator i= begin();
    for(;i!= end();i++)
      err+= (*i)->getMaterial()->revertToStart();
    err+= updateKRCDG(Section2d,kr2);
    return err;
  }

//! @brief Devuelve la matriz de rigidez noval.
const XC::Matrix &XC::DqFibras::getInitialTangent(const FiberSection2d &Section2d) const
  {
    static double kInitial[4];
    kInitial[0]= 0.0; kInitial[1]= 0.0;
    kInitial[2]= 0.0; kInitial[3]= 0.0;
    static Matrix kInitialMatrix(kInitial, 2, 2);

    std::deque<Fiber *>::const_iterator i= begin();
    UniaxialMaterial *theMat= nullptr;
    double y,areaFibra,tangent; 
    for(;i!= end();i++)
      {
        theMat= (*i)->getMaterial();
        y= (*i)->getLocY();
        areaFibra= (*i)->getArea();

        if(areaFibra!=0.0)
          {
            tangent= theMat->getInitialTangent();
            KRSeccion::updateK2d(kInitial,areaFibra,y,tangent);
          }
      }

    kInitial[2]= kInitial[1]; //Simetría.
    return kInitialMatrix;
  }

const XC::Vector &XC::DqFibras::getStressResultantSensitivity(int gradNumber, bool conditional)
  {
    static XC::Vector ds(2);
    ds.Zero();
    double y, areaFibra, stressGradient;
    std::deque<Fiber *>::const_iterator i= begin();
    for(;i!= end();i++)
      {
        y= (*i)->getLocY();
        areaFibra= (*i)->getArea();
        stressGradient= (*i)->getMaterial()->getStressSensitivity(gradNumber,true);
        stressGradient= stressGradient * areaFibra;
        ds(0)+= stressGradient;
        ds(1)+= stressGradient * y;
      }
    return ds;
  }

int XC::DqFibras::commitSensitivity(const XC::Vector& defSens, int gradNumber, int numGrads)
  {
    double d0= defSens(0);
    double d1= defSens(1);
    std::deque<Fiber *>::const_iterator i= begin();
    for(;i!= end();i++)
      {
        UniaxialMaterial *theMat= (*i)->getMaterial();
        double y= (*i)->getLocY();

        // determine material strain and set it
        const double strainSens= d0 + y*d1;
        theMat->commitSensitivity(strainSens,gradNumber,numGrads);
      }
    return 0;
  }

//! @brief Actualiza los parámetros CDG, matriz de rigidez rigidez y resultante.
int XC::DqFibras::updateKRCDG(FiberSection3d &Section3d,KRSeccion &kr3)
  {
    kr3.zero();
    double Qy= 0.0,Qz= 0.0;
    double Atot= 0.0;
    double areaFibra= 0.0;
    double yLoc= 0.0, zLoc= 0.0;

    // Recompute centroid
    double stress,tangent,fs0;
    std::deque<Fiber *>::iterator i= begin();
    for(;i!= end();i++)
      {
        yLoc= (*i)->getLocY();
        zLoc= (*i)->getLocZ();
        areaFibra= (*i)->getArea();
        if(areaFibra!=0.0)
          {
            Atot+= areaFibra;
            Qz+= -yLoc*areaFibra; //Coordenada y cambiada de signo.
            Qy+= zLoc*areaFibra;

            //Actualizamos matriz de rigidez.
            tangent= (*i)->getMaterial()->getTangent();

            kr3.updateK3d(areaFibra,yLoc,zLoc,tangent);

            //Actualizamos la resultante de tensiones.
            stress= (*i)->getMaterial()->getStress();
            fs0= stress * areaFibra;
            kr3.updateNMzMy(fs0,yLoc,zLoc);
	  }
      }
    yCDG= -Qz/Atot; //Coordenada y del CDG  XXX ¿Signo menos?
    zCDG= Qy/Atot; //Coordenada z del CDG 
    kr3.kData[3]= kr3.kData[1]; //Simetría de la matriz de rigidez.
    kr3.kData[6]= kr3.kData[2];
    kr3.kData[7]= kr3.kData[5];
    return 0;
  }

//! @brief Añade una fibra a la sección XXX Mejorar actualización de parámetros.
XC::Fiber *XC::DqFibras::addFiber(FiberSection3d &Section3d,Fiber &newFiber,KRSeccion &kr3)
  {
    Fiber *retval= inserta(newFiber);
    updateKRCDG(Section3d,kr3);
    return retval;
  }

//! @brief Establece los valores de las deformaciones iniciales.
int XC::DqFibras::setInitialSectionDeformation(const FiberSection3d &Section3d)
  {
    int retval= 0;
    UniaxialMaterial *theMat;
    double y,z; 
    std::deque<Fiber *>::iterator i= begin();
    for(;i!= end();i++)
      {
        theMat= (*i)->getMaterial();
        y= (*i)->getLocY();
        z= (*i)->getLocZ();
        // determine material strain and set it
        retval+= theMat->setInitialStrain(Section3d.get_strain(y,z));
      }
    return retval;
  }

//! @brief Establece los valores de las deformaciones de prueba.
int XC::DqFibras::setTrialSectionDeformation(FiberSection3d &Section3d,KRSeccion &kr3)
  {
    int retval= 0;
    kr3.zero();
    std::deque<Fiber *>::iterator i= begin();
    UniaxialMaterial *theMat;
    double y,z,areaFibra,tangent,stress, fs0; 
    for(;i!= end();i++)
      {
        theMat= (*i)->getMaterial();
        y= (*i)->getLocY();
        z= (*i)->getLocZ();

        // determine material strain and set it
        retval+= theMat->setTrial(Section3d.get_strain(y,z), stress, tangent);

        //Actualizamos matriz de rigidez.
        areaFibra= (*i)->getArea();
        if(areaFibra!=0.0)
          {
            kr3.updateK3d(areaFibra,y,z,tangent);

            //Actualizamos la resultante de tensiones.
            fs0= stress * areaFibra;
            kr3.updateNMzMy(fs0,y,z);
          }
      }
    kr3.kData[3]= kr3.kData[1]; //Simetría de la matriz de rigidez.
    kr3.kData[6]= kr3.kData[2];
    kr3.kData[7]= kr3.kData[5];
    return retval;
  }

//! @brief Devuelve el estado de la sección al del último commit.
int XC::DqFibras::revertToLastCommit(FiberSection3d &Section3d,KRSeccion &kr3)
  {
    int err= 0;
    kr3.zero();
    std::deque<Fiber *>::iterator i= begin();
    for(;i!= end();i++)
      err+= (*i)->getMaterial()->revertToLastCommit(); // invoke revertToLastCommit on the material
    err+= updateKRCDG(Section3d,kr3);
    return err;
  }

//! @brief Devuelve las fibras a su estado inicial.
int XC::DqFibras::revertToStart(FiberSection3d &Section3d,KRSeccion &kr3)
  {
    // revert the fibers to start
    int err= 0;
    kr3.zero();
    std::deque<Fiber *>::iterator i= begin();
    for(;i!= end();i++)
      err+= (*i)->getMaterial()->revertToStart(); // invoke revertToStart on the material
    err+= updateKRCDG(Section3d,kr3);
    return err;
  }

//! @brief Devuelve la matriz de rigidez tangente inicial.
const XC::Matrix &XC::DqFibras::getInitialTangent(const FiberSection3d &Section3d) const
  {
    static double kInitialData[9];
    static XC::Matrix kInitial(kInitialData, 3, 3);

    kInitialData[0]= 0.0; kInitialData[1]= 0.0; kInitialData[2]= 0.0;
    kInitialData[3]= 0.0; kInitialData[4]= 0.0; kInitialData[5]= 0.0;
    kInitialData[6]= 0.0; kInitialData[7]= 0.0; kInitialData[8]= 0.0;

    UniaxialMaterial *theMat;
    double y,z,areaFibra,tangent; 
    std::deque<Fiber *>::const_iterator i= begin();
    for(;i!= end();i++)
      {
        theMat= (*i)->getMaterial();
        y= (*i)->getLocY();
        z= (*i)->getLocZ();
        areaFibra= (*i)->getArea();

        if(areaFibra!=0.0)
	  {
            tangent= theMat->getInitialTangent();
            KRSeccion::updateK3d(kInitialData,areaFibra,y,z,tangent);
          }
      }
    kInitialData[3]= kInitialData[1]; //Simetría de la matriz de rigidez.
    kInitialData[6]= kInitialData[2];
    kInitialData[7]= kInitialData[5];

    return kInitial;
  }

//! @brief Actualiza los parámetros CDG, rigidez y resultante.
int XC::DqFibras::updateKRCDG(FiberSectionGJ &SectionGJ,KRSeccion &krGJ)
  {
    krGJ.zero();
    double Qy= 0.0,Qz= 0.0;
    double Atot= 0.0;
    double areaFibra= 0.0;
    double yLoc= 0.0, zLoc=0.0;

    // Recompute centroid
    double stress,tangent,fs0;
    std::deque<Fiber *>::iterator i= begin();
    for(;i!= end();i++)
      {
        yLoc= (*i)->getLocY();
        zLoc= (*i)->getLocZ();
        areaFibra= (*i)->getArea();
        if(areaFibra!=0.0)
          {
            Atot+= areaFibra;
            Qz+= -yLoc*areaFibra; //Coordenada y cambiada de signo.
            Qy+= zLoc*areaFibra;

            tangent= (*i)->getMaterial()->getTangent();

            //Actualizamos matriz de rigidez.
            krGJ.updateKGJ(areaFibra,yLoc,zLoc,tangent);

            //Actualizamos la resultante de tensiones.
            stress= (*i)->getMaterial()->getStress();
            fs0= stress * areaFibra;
            krGJ.updateNMzMy(fs0,yLoc,zLoc);
          }
      }
    yCDG= -Qz/Atot; //Coordenada y del CDG  XXX ¿Signo menos?
    zCDG= Qy/Atot; //Coordenada z del CDG 
    krGJ.kData[4]= krGJ.kData[1]; //Simetría de la matriz de rigidez.
    krGJ.kData[8]= krGJ.kData[2];
    krGJ.kData[9]= krGJ.kData[6];
    krGJ.kData[15]= SectionGJ.getGJ(); //(3,3)->15 //Los seis elementos restantes de krGJ.kData son nulos.

    krGJ.rData[3]= SectionGJ.getGJ()*SectionGJ.getSectionDeformation()(3); //Torsión.
    return 0;
  }

//! @brief Añade una fibra XXX Mejorar actualización de parámetros.
XC::Fiber *XC::DqFibras::addFiber(FiberSectionGJ &SectionGJ,Fiber &newFiber,KRSeccion &krGJ)
  {
    Fiber *retval= inserta(newFiber);
    updateKRCDG(SectionGJ,krGJ);
    return retval;
  }

//! @brief Establece los valores de las deformaciones iniciales.
int XC::DqFibras::setInitialSectionDeformation(const FiberSectionGJ &SectionGJ)
  {
    int retval= 0;
    UniaxialMaterial *theMat;
    double y,z; 
    std::deque<Fiber *>::iterator i= begin();
    for(;i!= end();i++)
      {
        theMat= (*i)->getMaterial();
        y= (*i)->getLocY();
        z= (*i)->getLocZ();
        // determine material strain and set it
        retval+= theMat->setInitialStrain(SectionGJ.get_strain(y,z));
      }
    return retval;
  }

//! @brief Establece los valores de las deformaciones de prueba.
int XC::DqFibras::setTrialSectionDeformation(FiberSectionGJ &SectionGJ,KRSeccion &krGJ)
  {
    int retval= 0;
    krGJ.zero();
    UniaxialMaterial *theMat;
    double y,z,areaFibra,tangent,stress, fs0; 
    std::deque<Fiber *>::iterator i= begin();
    for(;i!= end();i++)
      {
        theMat= (*i)->getMaterial();
        y= (*i)->getLocY();
        z= (*i)->getLocZ();
        areaFibra= (*i)->getArea();

        if(areaFibra!=0.0)
          {
            // determine material strain and set it
            retval= theMat->setTrial(SectionGJ.get_strain(y,z), stress, tangent);

            //Actualizamos matriz de rigidez.
            krGJ.updateKGJ(areaFibra,y,z,tangent);

            //Actualizamos la resultante de tensiones.
            fs0= stress * areaFibra;
            krGJ.updateNMzMy(fs0,y,z);
	  }
      }
    krGJ.kData[4]= krGJ.kData[1]; //Simetría de la matriz de rigidez.
    krGJ.kData[8]= krGJ.kData[2];
    krGJ.kData[9]= krGJ.kData[6];
    krGJ.kData[15]= SectionGJ.getGJ(); //(3,3)->15 //Los seis elementos restantes de krGJ.kData son nulos.

    krGJ.rData[3]= SectionGJ.getGJ()*SectionGJ.getSectionDeformation()(3); //Torsión.
    return retval;
  }

//! @brief Devuelve el estado de la sección al del último commit.
int XC::DqFibras::revertToLastCommit(FiberSectionGJ &SectionGJ,KRSeccion &krGJ)
  {
    int err= 0;
    krGJ.zero();
    std::deque<Fiber *>::iterator i= begin();
    for(;i!= end();i++)
      err+= (*i)->getMaterial()->revertToLastCommit(); // invoke revertToLastCommit on the material
    err+= updateKRCDG(SectionGJ,krGJ);
    return err;
  }

//! @brief Devuelve las fibras a su estado inicial.
int XC::DqFibras::revertToStart(FiberSectionGJ &SectionGJ,KRSeccion &krGJ)
  {
    // revert the fibers to start
    int err= 0;
    krGJ.zero();
    std::deque<Fiber *>::iterator i= begin();
    for(;i!= end();i++)
      err+= (*i)->getMaterial()->revertToStart(); // invoke revertToStart on the material
    err+= updateKRCDG(SectionGJ,krGJ);
    return err;
  }

//! @brief Devuelve la matriz de rigidez noval.
const XC::Matrix &XC::DqFibras::getInitialTangent(const FiberSectionGJ &SectionGJ) const
  {
    static double kInitialData[16];

    kInitialData[0]= 0.0; kInitialData[1]= 0.0; kInitialData[2]= 0.0; kInitialData[3]= 0.0;
    kInitialData[4]= 0.0; kInitialData[5]= 0.0; kInitialData[6]= 0.0; kInitialData[7]= 0.0;
    kInitialData[8]= 0.0; kInitialData[9]= 0.0; kInitialData[10]= 0.0; kInitialData[11]= 0.0;
    kInitialData[12]= 0.0; kInitialData[13]= 0.0; kInitialData[14]= 0.0; kInitialData[15]= 0.0;

    static XC::Matrix kInitial(kInitialData, 4, 4);
    UniaxialMaterial *theMat;
    double y,z,areaFibra,tangent; 
    std::deque<Fiber *>::const_iterator i= begin();
    for(;i!= end();i++)
      {
        theMat= (*i)->getMaterial();
        y= (*i)->getLocY();
        z= (*i)->getLocZ();
        areaFibra= (*i)->getArea();

        if(areaFibra!= 0.0)
          {
            tangent= theMat->getInitialTangent();
            //Actualizamos matriz de rigidez.
            KRSeccion::updateKGJ(kInitialData,areaFibra,y,z,tangent);
          }
      }
    kInitialData[4]= kInitialData[1]; //Simetría.
    kInitialData[8]= kInitialData[2]; //Simetría.
    kInitialData[9]= kInitialData[6]; //Simetría.

    kInitialData[15]= SectionGJ.GJ; //(3,3)->15 //Los seis elementos restantes de kInitialData son nulos.
    return kInitial;
  }

//! @brief Obtiene algún parámetro de la respuesta de la sección.
XC::Response *XC::DqFibras::setResponse(const std::vector<std::string> &argv, Information &sectInfo)
  {
    const size_t numFibers= getNumFibers();
    if(argv[0] == "fiber")
      {
        size_t key= numFibers;
        int passarg= 2;
        const int argc= argv.size();
        if(argv.size() <= 2)          // not enough data input
          return 0;
        if(argv.size() <= 3)
          {// fiber number was input directly
            key= atoi(argv[1]);
            if(key < numFibers && key >= 0)
              {
		std::vector<std::string> argv2(argv);
                argv2.erase(argv2.begin(),argv2.begin()+passarg);
                return (*this)[key]->getMaterial()->setResponse(argv2,sectInfo);
              }
            else
              return nullptr;
          }
        if(argc > 4)
          { // find fiber closest to coord. with mat tag
            int matTag= atoi(argv[3]);
            const double yCoord= atof(argv[1]);
            const double zCoord= atof(argv[2]);
            double closestDist2,ySearch, zSearch, dy, dz;
            double distance2;
            size_t j;
            // Find first fiber with specified material tag
            for(j= 0;j<numFibers;j++)
              {
                if(matTag == (*this)[j]->getMaterial()->getTag())
                  {
                    ySearch= -(*this)[j]->getLocY();
                    zSearch=  (*this)[j]->getLocZ();
                    dy= ySearch-yCoord;
                    dz= zSearch-zCoord;
                    closestDist2= dy*dy + dz*dz;
                    key= j;
                    break;
                  }
              }
            // Search the remaining fibers
            for(;j<numFibers;j++)
              {
                if(matTag == (*this)[j]->getMaterial()->getTag())
                  {
                    ySearch= -(*this)[j]->getLocY();
                    zSearch=  (*this)[j]->getLocZ();
                    dy= ySearch-yCoord;
                    dz= zSearch-zCoord;
                    distance2= dy*dy + dz*dz;
                    if(distance2 < closestDist2)
                      {
                        closestDist2= distance2;
                        key= j;
                      }
                  }
              }
            passarg= 4;
          }
        else
          {                  // fiber near-to coordinate specified
            const double yCoord= atof(argv[1]);
            const double zCoord= atof(argv[2]);
            double distance2;
            double ySearch= -(*this)[0]->getLocY();
            double zSearch=  (*this)[0]->getLocZ();
            double dy= ySearch-yCoord;
            double dz= zSearch-zCoord;
            double closestDist2= dy*dy + dz*dz;
            key= 0;
            for(size_t j= 1; j < numFibers; j++)
              {
                ySearch= -(*this)[j]->getLocY();
                zSearch=  (*this)[j]->getLocZ();
                dy= ySearch-yCoord;
                dz= zSearch-zCoord;
                distance2= dy*dy + dz*dz;
                if(distance2 < closestDist2)
                  {
                    closestDist2= distance2;
                    key= j;
                  }
              }
            passarg= 3;
          }
        if(key < numFibers)
          {
            std::vector<std::string> argv2(argv);
            argv2.erase(argv2.begin(),argv2.begin()+passarg);
            return (*this)[key]->getMaterial()->setResponse(argv2,sectInfo);
          }
        else
          return 0;
      }
    // otherwise response quantity is unknown for the XC::FiberSection class
    else
      return 0;
  }

//! \brief Devuelve la propiedad del objeto cuyo código (de la propiedad) se pasa
//! como parámetro.
any_const_ptr XC::DqFibras::GetProp(const std::string &cod) const
  {
    if(verborrea>4)
      std::clog << "DqFibras::GetProp (" << nombre_clase() << "::GetProp) Buscando propiedad: " << cod << std::endl;
    static m_double tmp_m_double;

    if(cod=="numFibras" || cod=="size")
      {
        tmp_gp_szt= getNumFibers();
        return any_const_ptr(tmp_gp_szt);
      }
    else if(cod=="yCDG")
      return any_const_ptr(yCDG);
    else if(cod=="zCDG")
      return any_const_ptr(zCDG);
    else if((cod=="getSumaAreas") || (cod=="getArea"))
      {
        tmp_gp_dbl= getSumaAreas();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getResultante")
      {
        tmp_gp_dbl= Resultante();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getMz")
      {
        tmp_gp_dbl=getMz();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getMy")
      {
        tmp_gp_dbl=getMy();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getResultanteTracc")
      {
        tmp_gp_dbl= ResultanteTracc();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="MzTracc")
      {
        tmp_gp_dbl= getMzTracc();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="MyTracc")
      {
        tmp_gp_dbl= getMyTracc();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getBaricentroTracc")
      {
        Vector p= baricentroTracciones();
        if(!p.isnan())
	  tmp_m_double= vector_to_m_double(p);
        else 
	  std::cerr << "No se pudo obtener el baricentro de tracciones." << std::endl;
        return any_const_ptr(tmp_m_double);
      }
    else if(cod=="getResultanteComp")
      {
        tmp_gp_dbl= ResultanteComp();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="MzComp")
      {
        tmp_gp_dbl= getMzComp();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="MyComp")
      {
        tmp_gp_dbl= getMyComp();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getBaricentroComp")
      {
        Vector p= baricentroCompresiones();
        if(!p.isnan())
          tmp_m_double= vector_to_m_double(p);
        else
	  std::cerr << "No se pudo obtener el baricentro de compresiones." << std::endl;
        return any_const_ptr(tmp_m_double);
      }
    else if(cod=="INearest")
      {
        double x= 0.0,y=0.0;
        if(InterpreteRPN::Pila().size()>1)
          {
            y= convert_to_double(InterpreteRPN::Pila().Pop());
            x= convert_to_double(InterpreteRPN::Pila().Pop());
          }
        else
          err_num_argumentos(std::cerr,2,"GetProp",cod);
        tmp_gp_szt= nearest_fiber(x,y);
        return any_const_ptr(tmp_gp_szt);
      }
    else if(cod=="IMaxProp")
      {
        if(InterpreteRPN::Pila().size()>0)
          {
            tmp_gp_szt= IMaxProp(convert_to_string(InterpreteRPN::Pila().Pop()));
            return any_const_ptr(tmp_gp_szt);
          }
        else
          {
            err_num_argumentos(std::cerr,2,"GetProp",cod);
            return any_const_ptr();
          }
      }
    else if(cod=="IMinProp")
      {
        if(InterpreteRPN::Pila().size()>0)
          {
            tmp_gp_szt= IMinProp(convert_to_string(InterpreteRPN::Pila().Pop()));
            return any_const_ptr(tmp_gp_szt);
          }
        else
          {
            err_num_argumentos(std::cerr,2,"GetProp",cod);
            return any_const_ptr();
          }
      }
    else if(cod=="fibra")
      {
        static Fiber *f= nullptr;
        const size_t indice= popSize_t(cod);
        if(indice<size())
          f= (*this)[indice];
        else
	  std::cerr << "DdFibras::GetProp '" << cod << "', índice: "
                    << indice << " fuera de rango." << std::endl;
        return any_const_ptr(f);
      }
    else if(cod=="getAcEficazFibra")
      {
        const size_t indice= popSize_t(cod);
        if(indice<dq_ac_eficaz.size())
          tmp_gp_dbl= getAcEficazFibra(indice);
        else
	  std::cerr << "DdFibras::GetProp '" << cod << "', índice: "
                    << indice << " fuera de rango." << std::endl;
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getRecubrimientoFibra")
      {
        const size_t indice= popSize_t(cod);
        if(indice<dq_ac_eficaz.size())
          tmp_gp_dbl= getRecubrimientoFibra(indice);
        else
	  std::cerr << "DdFibras::GetProp '" << cod << "', índice: "
                    << indice << " fuera de rango." << std::endl;
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getSeparacionFibra")
      {
        const size_t indice= popSize_t(cod);
        if(indice<dq_ac_eficaz.size())
          tmp_gp_dbl= getSeparacionFibra(indice);
        else
	  std::cerr << "DdFibras::GetProp '" << cod << "', índice: "
                    << indice << " fuera de rango." << std::endl;
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getDiamEqFibra")
      {
        const size_t indice= popSize_t(cod);
        if(indice<dq_ac_eficaz.size())
          tmp_gp_dbl= getDiamEqFibra(indice);
        else
	  std::cerr << "DdFibras::GetProp '" << cod << "', índice: "
                    << indice << " fuera de rango." << std::endl;
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getSigmaSRFibra")
      {
        size_t indice= 0;
        double Ec= 1.0; double Es= 0.0; double fctm= 0.0;
        if(InterpreteRPN::Pila().size()>3)
          {
            fctm= convert_to_double(InterpreteRPN::Pila().Pop());
            Es= convert_to_double(InterpreteRPN::Pila().Pop());
            Ec= convert_to_double(InterpreteRPN::Pila().Pop());
            indice= convert_to_size_t(InterpreteRPN::Pila().Pop());
          }
        else
          err_num_argumentos(std::cerr,4,"GetProp",cod);
        if(indice<dq_ac_eficaz.size())
          tmp_gp_dbl= getSigmaSRFibra(indice,Ec,Es,fctm);
        else
	  std::cerr << "DdFibras::GetProp '" << cod << "', índice: "
                    << indice << " fuera de rango." << std::endl;
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getDistMediaFibras")
      {
        tmp_gp_dbl= getDistMediaFibras();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getVectorBrazoMecanico")
      {
        tmp_gp_mdbl= vector_to_m_double(getVectorBrazoMecanico());
        return any_const_ptr(tmp_gp_mdbl);
      }
    else if(cod=="getBrazoMecanico")
      {
        tmp_gp_dbl= getBrazoMecanico();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getLongDiagonalBND")
      {
        const BND2d tmp= Bnd();
        tmp_gp_dbl= tmp.Diagonal().GetModulus();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getStrainMin") //Deformación mínima.
      {
        tmp_gp_dbl= getStrainMin();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getStrainMax") //Deformación máxima.
      {
        tmp_gp_dbl= getStrainMax();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getStrainMed") //Deformación media.
      {
        tmp_gp_dbl= getStrainMed();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getStrain") //Vector de deformaciones.
      {
        static m_double retval;
        retval= vector_to_m_double(getDeformation());
        return any_const_ptr(&retval);
      }
    else if(cod=="getStressMin") //Tensión mínima.
      {
        tmp_gp_dbl= getStressMin();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getStressMax") //Tensión máxima.
      {
        tmp_gp_dbl= getStressMax();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getStressMed") //Tensión media.
      {
        tmp_gp_dbl= getStressMed();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getStrClaseEsfuerzo")
      {
        tmp_gp_str= getStrClaseEsfuerzo();
        return any_const_ptr(tmp_gp_str);
      }
    else
      return EntCmd::GetProp(cod);
  }

//! Devuelve el valor máximo en el conjunto de fibras de la propiedad que se
//! pasa como parámetro.
size_t XC::DqFibras::IMaxProp(const std::string &nmb_prop) const
  {
    size_t retval= 0;
    double vmax= 0.0;
    const size_t nf= getNumFibers();
    if(nf<1)
      {
        std::cerr << "DqFibras::IMaxProp; No hay fibras definidas." << std::endl;
        return retval;
      }
    register size_t i= 0;
    (*this)[i]->interpreta(nmb_prop);
    boost::any valor_prop= InterpreteRPN::Pila().Pop();
    if(boost_any_is_number(valor_prop))
      {
        vmax= convert_to_double(valor_prop);
        retval= i;
      }
    i++;
    double tmp= vmax;
    for(;i<nf;i++)
      if((*this)[i])
        {
          (*this)[i]->interpreta(nmb_prop);
          boost::any valor_prop= InterpreteRPN::Pila().Pop();
          if(boost_any_is_number(valor_prop))
            {
              tmp= convert_to_double(valor_prop);
              if(tmp>vmax)
                {
                  retval= i;
                  vmax= tmp;
                }
            }
        }
      else
        std::cerr << "DqFibras::IMaxProp; Puntero a fibra nulo." << std::endl;
    return retval;
  }

//! Devuelve el indice de la fibra que presenta el valor mínimo
//! en el conjunto de fibras de la propiedad que se pasa como parámetro.
size_t XC::DqFibras::IMinProp(const std::string &nmb_prop) const
  {
    size_t retval= 0;
    double vmin= 0.0;
    const size_t nf= getNumFibers();
    if(nf<1)
      {
        std::cerr << "DqFibras::IMinProp; No hay fibras definidas." << std::endl;
        return retval;
      }
    register size_t i= 0;
    (*this)[i]->interpreta(nmb_prop);
    boost::any valor_prop= InterpreteRPN::Pila().Pop();
    if(boost_any_is_number(valor_prop))
      {
        vmin= convert_to_double(valor_prop);
        retval= i;
      }
    i++;
    double tmp= vmin;
    for(;i<nf;i++)
      if((*this)[i])
        {
          (*this)[i]->interpreta(nmb_prop);
          valor_prop= InterpreteRPN::Pila().Pop();
          if(boost_any_is_number(valor_prop))
            {
              tmp= convert_to_double(valor_prop);
              if(tmp<vmin)
                {
                  retval= i;
                  vmin= tmp;
                }
            }
        }
      else
        std::cerr << "DqFibras::IMinProp; Puntero a fibra nulo." << std::endl;
    return retval;
  }

//! @brief Devuelve el índice de la fibra cuya coordenada iCoo respecto al sistema
//! de referencia que se pasa como parámetro es máxima.
size_t XC::DqFibras::getFibraCooMax(const Ref3d3d &r,const size_t &iCoo) const
  {
    size_t retval= 0;
    const size_t nf= getNumFibers();
    if(nf<1)
      {
        std::cerr << "DqFibras::getFibraCooMax; no hay fibras definidas." << std::endl;
        return retval;
      }
    register size_t i= 0;
    retval= i;
    Pos3d p(0.0,(*this)[i]->getLocY(),(*this)[i]->getLocZ());
    double v_max= r.GetPosLocal(p)(iCoo);
    double v= v_max;
    i++;
    for(;i<nf;i++)
      {
        p= Pos3d(0.0,(*this)[i]->getLocY(),(*this)[i]->getLocZ());
        v= r.GetPosLocal(p)(iCoo);
        if(v>v_max)
          {
            v_max= v;
            retval= i;
          }
      }
    return retval;
  }

//! @brief Devuelve el índice de la fibra cuya coordenada iCoo respecto al sistema
//! de referencia que se pasa como parámetro es mínima.
size_t XC::DqFibras::getFibraCooMin(const Ref3d3d &r,const size_t &iCoo) const
  {
    size_t retval= 0;
    const size_t nf= getNumFibers();
    if(nf<1)
      {
        std::cerr << "DqFibras::getFibraCooMin; no hay fibras definidas." << std::endl;
        return retval;
      }
    register size_t i= 0;
    retval= i;
    Pos3d p(0.0,(*this)[i]->getLocY(),(*this)[i]->getLocZ());
    double v_min= r.GetPosLocal(p)(iCoo);
    double v= v_min;
    i++;
    for(;i<nf;i++)
      {
        p= Pos3d(0.0,(*this)[i]->getLocY(),(*this)[i]->getLocZ());
        v= r.GetPosLocal(p)(iCoo);
        if(v<v_min)
          {
            v_min= v;
            retval= i;
          }
      }
    return retval;
  }

int XC::DqFibras::setParameter(const int &paramMatTag,const std::vector<std::string> &argv, Parameter &param)
  {
    int parameterID= -1;
    std::deque<Fiber *>::iterator i= begin();
    for(;i!= end();i++)
      if(paramMatTag == (*i)->getMaterial()->getTag())
        parameterID= (*i)->getMaterial()->setParameter(argv, param);
    return parameterID;
  }

int XC::DqFibras::updateParameter(const int &paramMatTag,int parameterID, Information &info)
  {
    int ok= -1;
    std::deque<Fiber *>::iterator i= begin();
    for(;i!= end();i++)
      if(paramMatTag == (*i)->getMaterial()->getTag())
        ok =(*i)->getMaterial()->updateParameter(parameterID, info);
    return ok;
  }

int XC::DqFibras::activateParameter(int passedParameterID)
  {
    std::deque<Fiber *>::iterator i= begin();
    if(passedParameterID == 0 )
      {
        // "Zero out" the identifier in all materials
        for(;i!= end();i++)
          (*i)->getMaterial()->activateParameter(passedParameterID);
      }
    else
      {
        // Extract section and material tags
        int activeSectionTag= (int)( floor((double)passedParameterID) / (100000) );
        passedParameterID -= activeSectionTag*100000;
        int activeMaterialTag= (int)( floor((double)passedParameterID) / (1000) );
        passedParameterID -= activeMaterialTag*1000;

        // Go down to the sections and set appropriate flags
        for(;i!= end();i++)
          if(activeMaterialTag == (*i)->getMaterial()->getTag())
            (*i)->getMaterial()->activateParameter(passedParameterID);
      }
    return 0;
  }

void XC::DqFibras::Print(std::ostream &s,const int &flag)
  {
    s << "\tNumber of Fibers: " << getNumFibers() << std::endl;
    s << "\tCentroid: (" << -yCDG << ", " << zCDG << ')' << std::endl;
    std::deque<Fiber *>::const_iterator i= begin();
    if(flag == 2)
      for(;i!= end();i++)
        {
          s << -(*i)->getLocY() << " "  << (*i)->getLocZ() << " "  << (*i)->getArea() << " " ;
          s << (*i)->getMaterial()->getStress() << " "  << (*i)->getMaterial()->getStrain() << std::endl;
        }
    else if(flag == 1)
      for(;i!= end();i++)
        {
           s << "\nLocation (y, z)= (" << -(*i)->getLocY() << ", " << (*i)->getLocZ() << ")";
           s << "\nArea= " << (*i)->getArea() << std::endl;
           (*i)->getMaterial()->Print(s, flag);
        }
  }


