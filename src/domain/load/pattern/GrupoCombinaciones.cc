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
//GrupoCombinaciones.cc

#include "GrupoCombinaciones.h"
#include "modelador/loaders/LoadLoader.h"
#include "xc_utils/src/base/CmdStatus.h"

#include "domain/load/pattern/Combinacion.h"
#include "domain/domain/Domain.h"

#include "xc_utils/src/base/any_const_ptr.h"
#include "xc_utils/src/base/utils_any.h"
#include "utility/actor/actor/MovableMap.h"
#include "xc_utils/src/base/Lista.h"


//! @brief Constructor por defecto.
XC::GrupoCombinaciones::GrupoCombinaciones(LoadLoader *owr)
  : LoadLoaderMember(owr) {}


//! @brief Devuelve un puntero a la combinación cuyo nombre se pasa como parámetro.
XC::Combinacion *XC::GrupoCombinaciones::busca_combinacion(const std::string &comb_code)
  {
    Combinacion *retval= nullptr;
    const_iterator icomb= find(comb_code);
    if(icomb!=end())
      retval= icomb->second;
    return retval;
  }

//! @brief Devuelve un puntero a la combinación cuyo nombre se pasa como parámetro.
const XC::Combinacion *XC::GrupoCombinaciones::buscaCombinacion(const std::string &comb_code) const
  {
    Combinacion *retval= nullptr;
    const_iterator icomb= find(comb_code);
    if(icomb!=end())
      retval= icomb->second;
    return retval;
  }

//! @brief Devuelve el nombre de la combinación a la que apunta el parámetro.
const std::string &XC::GrupoCombinaciones::getNombreCombinacion(const Combinacion *ptr) const
  {
    static std::string retval;
    retval= "";
    for(const_iterator i= begin();i!=end();i++)
      if((*i).second == ptr)
        {
          retval= (*i).first;
          break;
        }
    return retval;
  }

//! @brief Elimina la combinación del dominio.
void XC::GrupoCombinaciones::remove(const std::string &comb_code)
  {
    Combinacion *comb= nullptr;
    iterator icomb= find(comb_code);
    if(icomb!=end())
      comb= icomb->second;
    if(comb)
      {
        getDomain()->removeCombinacion(comb);
        delete comb;
        (*icomb).second= nullptr;
        erase(icomb);
      }
    else
      std::cerr << "GrupoCombinaciones::remove: no se encontró la combinación: " 
                << comb_code << std::endl;
  }

//! @brief Agrega la combinación al dominio.
void XC::GrupoCombinaciones::addToDomain(const std::string &comb_code)
  {
    Combinacion *comb= busca_combinacion(comb_code);
    if(comb)
      getDomain()->addCombinacion(comb);
    else
      std::cerr << "GrupoCombinaciones::addToDomain: no se encontró la combinación: " 
                << comb_code << std::endl;
  }

//! @brief Elimina la combinación del dominio.
void XC::GrupoCombinaciones::removeFromDomain(const std::string &comb_code)
  {
    Combinacion *comb= busca_combinacion(comb_code);
    if(comb)
      getDomain()->removeCombinacion(comb);
    else
      std::cerr << "GrupoCombinaciones::removeFromDomain: no se encontró la combinación: " 
                << comb_code << std::endl;
  }

//! @brief Elimina las combinaciones del dominio.
void XC::GrupoCombinaciones::removeAllFromDomain(void)
  {
    Domain *dom= getDomain();
    for(iterator i= begin();i!=end();i++)
      dom->removeCombinacion((*i).second);
  }

XC::Combinacion *XC::GrupoCombinaciones::newLoadCombination(const std::string &code,const std::string &descomp)
  {
    int tag_comb= getLoadLoader()->getTagLP();
    Combinacion *comb= busca_combinacion(code);
    if(comb) //La combinación ya existe.
      {
        std::cerr << "La combinación: " << code
                  << " ya existe, se redefine." << std::endl;
        comb->setDescomp(descomp);
      }
    else //La combinación es nueva.
      {
        comb= new Combinacion(this,code,tag_comb,getLoadLoader());
        getLoadLoader()->setTagLP(tag_comb+1);
        if(comb)
          {
            comb->setDomain(getDomain());
            comb->setDescomp(descomp);
            (*this)[comb->getNombre()]= comb;
          }
      }
    return comb;
  }

//! @brief Lee un objeto TimeSeries desde archivo
int XC::GrupoCombinaciones::procesa_combinacion(const int &tag_lp,const std::string &cmd,CmdStatus &status)
  {
    int retval= tag_lp;
    std::deque<boost::any> fnc_indices= status.Parser().SeparaIndices(this);
    if(fnc_indices.size()<1)
      std::cerr << "combinacion: uso combinacion[code,tag] " << std::endl;
    std::string code= "nil";
    if(fnc_indices.size()>0)
      code= convert_to_string(fnc_indices[0]); //Código de la combinacion.
    Combinacion *comb= busca_combinacion(code);
    if(comb) //La combinación ya existe. 
      comb->LeeCmd(status);
    else //La combinación es nueva.
      {
        if(fnc_indices.size()>1)
          retval= convert_to_int(fnc_indices[1]); //Tag de la combinación.
        comb= new Combinacion(this,code,retval,getLoadLoader());
        retval++;
        if(comb)
          {
            comb->setDomain(getDomain());
            comb->LeeCmd(status);
            (*this)[comb->getNombre()]= comb;
          }
      }
    return retval;
  }

void XC::GrupoCombinaciones::EjecutaBloqueForEach(CmdStatus &status,const std::string &blq)
  {
    const std::string nmbBlq= nombre_clase()+":for_each";
    for(iterator i= begin();i!= end();i++)
      (*i).second->EjecutaBloque(status,blq,nmbBlq);
  }



//! @brief Lee un objeto GrupoCombinaciones desde archivo
bool XC::GrupoCombinaciones::procesa_comando(CmdStatus &status)
  {
    const std::string cmd= deref_cmd(status.Cmd());
    if(verborrea>2)
      std::clog << "(GrupoCombinaciones) Procesando comando: " << cmd << std::endl;
    Combinacion *cmb_ptr= busca_combinacion(cmd);
    if(cmb_ptr)
      {
        cmb_ptr->LeeCmd(status);
        return true;
      }
    else if(cmd == "combinacion")
      {
        std::deque<boost::any> fnc_indices= status.Parser().SeparaIndices(this);
        if(fnc_indices.size()<1)
	  std::cerr << "combinacion: uso combinacion[code,tag] " << std::endl;
	std::string code= "nil";
        if(fnc_indices.size()>0)
          code= convert_to_string(fnc_indices[0]); //Código de la combinacion.
        Combinacion *comb= busca_combinacion(code);
        if(comb) //La combinación ya existe. 
          comb->LeeCmd(status);
        else //La combinación es nueva.
	  std::cerr << "Las combinaciones nuevas se definen en LoadLoader."
                    << std::endl;
        return true;
      }
    else if(cmd == "remove") //Elimina la combinación del grup.
      {
        const std::string comb_code= interpretaString(status.GetString());
        remove(comb_code);
        return true;
      }
    else if(cmd == "add_to_domain")
      {
        const std::string comb_code= interpretaString(status.GetString());
        addToDomain(comb_code);
        return true;
      }
    else if(cmd == "remove_from_domain")
      {
        const std::string comb_code= interpretaString(status.GetString());
        removeFromDomain(comb_code);
        return true;
      }
    else if(cmd == "remove_all_from_domain")
      {
        status.GetString();
        removeAllFromDomain();
        return true;
      }
    else if(cmd == "for_each")
      {
        const std::string bloque= status.GetBloque();
        EjecutaBloqueForEach(status,bloque);
        return true;
      }
    else if(cmd == "clear")
      {
        status.GetBloque(); //Ignoramos entrada.
        clear();
        return true;
      }
    else
      return LoadLoaderMember::procesa_comando(status);
  }

//! @brief Returns container's keys.
boost::python::list XC::GrupoCombinaciones::getKeys(void) const
  {
    boost::python::list retval;
    for(const_iterator i=begin();i!=end();i++)
      retval.append((*i).first);
    return retval;
  }

//! @brief Borra todas las combinaciones.
void XC::GrupoCombinaciones::clear(void)
  {
    removeAllFromDomain();
    for(iterator i= begin();i!=end();i++)
      {
        delete (*i).second;
        (*i).second= nullptr;
      }
    map_combinaciones::clear();
  }

//! @brief Destructor.
XC::GrupoCombinaciones::~GrupoCombinaciones(void)
  { clear(); }

//! @brief Devuelve un vector para almacenar los dbTags
//! de los miembros de la clase.
XC::DbTagData &XC::GrupoCombinaciones::getDbTagData(void) const
  {
    static DbTagData retval(1);
    return retval;
  }

//! @brief Envía los miembros a través del canal que se pasa como parámetro.
int XC::GrupoCombinaciones::sendData(CommParameters &cp)
  { return sendMap(*this,cp,getDbTagData(),CommMetaData(0)); }

//! @brief Envía los miembros a través del canal que se pasa como parámetro.
int XC::GrupoCombinaciones::recvData(const CommParameters &cp)
  {
    clear();
    int res= receiveMap(*this,cp,getDbTagData(),CommMetaData(0),&FEM_ObjectBroker::getNewCombinacion);
    for(iterator i= begin();i!= end();i++)
      {
        (*i).second->set_owner(this);
        (*i).second->setLoader(getLoadLoader());
        (*i).second->recvDescomp();
      }
    return res;
  }

//! @brief Envía el objeto por el canal que se pasa como parámetro.
int XC::GrupoCombinaciones::sendSelf(CommParameters &cp)
  {
    inicComm(1);
    int res= sendData(cp);
    const int dataTag= getDbTag(cp);
    res+= cp.sendIdData(getDbTagData(),dataTag);
    if(res<0)
      std::cerr << "GrupoCombinaciones::sendSelf() - failed to send data.\n";    
    return res;
  }


//! @brief Recibe el objeto por el canal que se pasa como parámetro.
int XC::GrupoCombinaciones::recvSelf(const CommParameters &cp)
  {
    int res= 0;
    //Si no está vacío entendemos que valen las definiciones existentes.
    if(empty())
      {
        inicComm(1);
        const int dataTag= getDbTag();
        res= cp.receiveIdData(getDbTagData(),dataTag);
        if(res<0)
          std::cerr << "GrupoCombinaciones::recvSelf() - data could not be received.\n" ;
        else
          res+= recvData(cp);
      }
    return res;
  }

std::deque<std::string> XC::GrupoCombinaciones::getListaNombres(void) const
  {
    std::deque<std::string> retval;
    for(const_iterator i= begin();i!= end();i++)
      retval.push_back((*i).first);
    return retval;
  }

//! @brief Busca la combinación más próxima a la que se pasa como parámetro
//! de entre las que tienen sus coeficientes de ponderación iguales o menores
//! que los de ésta.
XC::GrupoCombinaciones::const_iterator XC::GrupoCombinaciones::buscaCombPrevia(const Combinacion &c) const
  {
    const_iterator retval= end();
    Combinacion dif, tmpDif;
    for(const_iterator i= begin();i!= end();i++)
      {
        const Combinacion *tmp= (*i).second;
        if(c.dominaA(*tmp))
          {
            if(retval==end())
              {
                retval= i;
                dif= c-(*tmp);
              }
            else
              {
                tmpDif= c-(*tmp);
                if(tmpDif.size()<dif.size()) //Está más cerca.
                  {
                    retval= i;
                    dif= tmpDif; 
                  }
              }
	  }
      }
    return retval;
  }

const XC::Combinacion *XC::GrupoCombinaciones::getPtrCombPrevia(const Combinacion &c) const
  {
    const Combinacion *retval= nullptr;
    const_iterator i= buscaCombPrevia(c);
    if(i!=end())
      retval= (*i).second;
    return retval;
  }

XC::GrupoCombinaciones::const_iterator XC::GrupoCombinaciones::buscaCombPrevia(const std::string &nmb) const
  {
    const_iterator retval= end();
    const_iterator i= find(nmb);
    if(i!=end())
      retval= buscaCombPrevia(*i->second);
    return retval;
  }

const std::string XC::GrupoCombinaciones::getNombreCombPrevia(const std::string &nmb) const
  {
    std::string retval= "";
    const_iterator i= buscaCombPrevia(nmb);
    if(i!=end())
      retval= i->first;
    return retval;
  }

int XC::GrupoCombinaciones::getTagCombPrevia(const std::string &nmb) const
  {
    int retval= -1;
    const_iterator i= buscaCombPrevia(nmb);
    if(i!=end())
      retval= i->second->getTag();
    return retval;
  }

//! \brief Devuelve la propiedad del objeto cuyo código (de la propiedad) se pasa
//! como parámetro.
//!
//! Soporta los códigos:
any_const_ptr XC::GrupoCombinaciones::GetProp(const std::string &cod) const
  {
    if(cod=="numCombinaciones")
      {
        tmp_gp_szt= size();
        return any_const_ptr(tmp_gp_szt);
      }
    if(cod=="getNombreCombPrevia")
      {
        const std::string nmb= popString(cod);
        tmp_gp_str= getNombreCombPrevia(nmb);
        return any_const_ptr(tmp_gp_str);
      }
    if(cod=="getTagCombPrevia")
      {
        const std::string nmb= popString(cod);
        tmp_gp_int= getTagCombPrevia(nmb);
        return any_const_ptr(tmp_gp_int);
      }
    else if(cod == "getListaNombresCombinaciones")
      {
        EntCmd *this_no_const= const_cast<GrupoCombinaciones *>(this);
        tmp_gp_lista.set_owner(this_no_const);
        tmp_gp_lista.clearAll();
        const std::deque<std::string> tmp= getListaNombres();
	for(std::deque<std::string>::const_iterator i= tmp.begin();i!= tmp.end();i++)
          tmp_gp_lista.Inserta((*i));
        return any_const_ptr(tmp_gp_lista);
      }
    else
      return LoadLoaderMember::GetProp(cod);
  }