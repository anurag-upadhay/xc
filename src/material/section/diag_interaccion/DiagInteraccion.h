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
//DiagInteraccion.h

#ifndef DIAG_INTERACCION_H
#define DIAG_INTERACCION_H

#include "xc_utils/src/geom/d2/Triedro3d.h"
#include <set>
#include <deque>
#include "ClosedTriangleMesh.h"

class MallaTriang3d;

namespace XC {

class Vector;
class FiberSectionBase;
class DatosDiagInteraccion;

//! \@ingroup MATSCCDiagInt
//
//! @brief Diagrama de interacción (N,Mx,My) de una sección.
class DiagInteraccion: public ClosedTriangleMesh
  {
  protected:
    typedef std::set<const Triedro3d *> set_ptr_triedros;

    
    set_ptr_triedros triedros_cuadrante[8];

    void clasifica_triedro(const Triedro3d &tdro);
    void clasifica_triedros(void);
    void setMatrizPosiciones(const Matrix &);
    GeomObj::list_Pos3d get_interseccion(const Pos3d &p) const;
  public:
    DiagInteraccion(void);
    DiagInteraccion(const Pos3d &org,const MallaTriang3d &mll);
    DiagInteraccion(const DiagInteraccion &otro);
    DiagInteraccion &operator=(const DiagInteraccion &otro);
    virtual DiagInteraccion *clon(void) const;

    const Triedro3d *BuscaPtrTriedro(const Pos3d &p) const;
    double FactorCapacidad(const Pos3d &esf_d) const;
    Vector FactorCapacidad(const GeomObj::list_Pos3d &lp) const;

    void Print(std::ostream &os) const;
  };

DiagInteraccion calc_diag_interaccion(const FiberSectionBase &scc,const DatosDiagInteraccion &datos);

} // fin namespace XC

#endif