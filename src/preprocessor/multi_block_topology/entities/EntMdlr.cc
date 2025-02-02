//----------------------------------------------------------------------------
//  XC program; finite element analysis code
//  for structural analysis and design.
//
//  Copyright (C)  Luis Claudio Pérez Tato
//
//  This program derives from OpenSees <http://opensees.berkeley.edu>
//  developed by the  «Pacific earthquake engineering research center».
//
//  Except for the restrictions that may arise from the copyright
//  of the original program (see copyright_opensees.txt)
//  XC is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or 
//  (at your option) any later version.
//
//  This software is distributed in the hope that it will be useful, but 
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details. 
//
//
// You should have received a copy of the GNU General Public License 
// along with this program.
// If not, see <http://www.gnu.org/licenses/>.
//----------------------------------------------------------------------------
//EntMdlr.cc

#include "EntMdlr.h"
#include "xc_utils/src/matrices/RangoIndice.h"
#include "xc_utils/src/geom/pos_vec/Pos3dArray.h"
#include "xc_utils/src/geom/pos_vec/Pos3dArray3d.h"
#include "xc_utils/src/matrices/3d_arrays/Array3dRange.h"
#include "domain/mesh/node/Node.h"
#include "domain/domain/Domain.h"
#include "domain/mesh/element/Element.h"
#include "preprocessor/Preprocessor.h"
#include "preprocessor/set_mgmt/IRowSet.h"
#include "preprocessor/set_mgmt/JRowSet.h"
#include "preprocessor/set_mgmt/KRowSet.h"



#include "vtkCellType.h"

//! @brief Constructor.
XC::EntMdlr::EntMdlr(Preprocessor *m,const size_t &i)
  : SetEstruct("",m), idx(i), doGenMesh(true), ttzNodes(), ttzElements() {}

//! @brief Constructor.
//! @param nombre: Object identifier.
//! @param i: index to be used in VTK arrays.
//! @param m: Pointer to preprocessor.
XC::EntMdlr::EntMdlr(const std::string &nombre,const size_t &i,Preprocessor *m)
  : SetEstruct(nombre,m), idx(i), doGenMesh(true), ttzNodes(), ttzElements() {}


//! @brief Copy constructor.
XC::EntMdlr::EntMdlr(const EntMdlr &other)
  : SetEstruct(other), idx(other.idx), doGenMesh(true), ttzNodes(other.ttzNodes), ttzElements(other.ttzElements) {}

//! @brief Assignment operator.
XC::EntMdlr &XC::EntMdlr::operator=(const EntMdlr &other)
  {
    SetEstruct::operator=(other);
    idx= other.idx;
    doGenMesh= other.doGenMesh;
    ttzNodes= other.ttzNodes;
    ttzElements= other.ttzElements;
    return *this;
  }

//! @brief Assigns the objects index for its use in
//! VTK arrays(see numera in Set).
void XC::EntMdlr::set_index(const size_t &i)
  { idx= i; }

//! @brief Interfaz con VTK.
int XC::EntMdlr::getVtkCellType(void) const
  {
    std::cerr << getClassName() << "::" << __FUNCTION__
              << "; must be overloaded in derived classes."
	      << std::endl;
    return VTK_EMPTY_CELL;
  }

//! @brief Returns true if the object lies inside the
//! geometric object.
//!
//! @param geomObj: object to be contained in.
//! @param tol: tolerance.
bool XC::EntMdlr::In(const GeomObj3d &geomObj, const double &tol) const
  {
    std::cerr << getClassName() << "::" << __FUNCTION__
	      << "; not implemented yet."
	      << std::endl;
    return false;
  }

//! @brief Returns true if the object lies outside the
//! geometric object.
//!
//! @param geomObj: object to be contained in.
//! @param tol: tolerance.
bool XC::EntMdlr::Out(const GeomObj3d &geomObj, const double &tol) const
  {
    std::cerr << getClassName() << "::" << __FUNCTION__
	      << "; not implemented yet."
	      << std::endl;
    return false;
  }

//! @brief Clears object contents.
void XC::EntMdlr::clearAll(void)
  {
    SetEstruct::clearPyProps();
    BorraPtrNodElem();
  }

//! @brief Clears pointer to nodes and elements.
void XC::EntMdlr::BorraPtrNodElem(void)
  {
    ttzNodes.clearAll();
    ttzElements.clearAll();
  }

//! @brief Returns a pointer to the node which indexes are
//! being passed as parameters.
//!
//! @param i: index of the layer.
//! @param j: index of the row.
//! @param k: index of the column.
XC::Node *XC::EntMdlr::getNode(const size_t &i,const size_t &j,const size_t &k)
  {
    if(!ttzNodes.Null())
      return ttzNodes(i,j,k);
    else
      return nullptr;
  }

//! @brief Returns a pointer to the node which indexes are
//! being passed as parameters.
//!
//! @param i: index of the layer.
//! @param j: index of the row.
//! @param k: index of the column.
const XC::Node *XC::EntMdlr::getNode(const size_t &i,const size_t &j,const size_t &k) const
  {
    if(!ttzNodes.Null())
      return ttzNodes(i,j,k);
    else
      return nullptr;
  }

//! @brief Return the node closest to the point being passed as parameter.
XC::Node *XC::EntMdlr::getNearestNode(const Pos3d &p)
  { return ttzNodes.getNearestNode(p); }

//! @brief Return the node closest to the point being passed as parameter.
const XC::Node *XC::EntMdlr::getNearestNode(const Pos3d &p) const
  {
    EntMdlr *this_no_const= const_cast<EntMdlr *>(this);
    return this_no_const->getNearestNode(p);
  }

//! @brief Return the indexes of the node being passed as parameter.
XC::ID XC::EntMdlr::getNodeIndices(const Node *n) const
  { return ttzNodes.getNodeIndices(n); }

//! @brief Returns a pointer to the element which indexes
//! are being passed as paremeters.
//!
//! @param i: index of the layer.
//! @param j: index of the row.
//! @param k: index of the column.
XC::Element *XC::EntMdlr::getElement(const size_t &i,const size_t &j,const size_t &k)
  {
    if(!ttzElements.Null())
      return ttzElements(i,j,k);
    else
      return nullptr;
  }

//! @brief Returns a pointer to the element which indices are being passed as paremeters.
//!
//! @param i: index of the layer.
//! @param j: index of the row.
//! @param k: index of the column.
const XC::Element *XC::EntMdlr::getElement(const size_t &i,const size_t &j,const size_t &k) const
  {
    if(!ttzElements.Null())
      return ttzElements(i,j,k);
    else
      return nullptr;
  }

//! @brief Return the element closest to the point being passed as parameter.
XC::Element *XC::EntMdlr::getNearestElement(const Pos3d &p)
  { return ttzElements.getNearestElement(p); }

//! @brief Return the element closest to the point being passed as parameter.
const XC::Element *XC::EntMdlr::getNearestElement(const Pos3d &p) const
  {
    EntMdlr *this_no_const= const_cast<EntMdlr *>(this);
    return this_no_const->getNearestElement(p);
  }

//! @brief Returns a pointer to the node cuyo identifier is being passed as parameter.
XC::Node *XC::EntMdlr::findNode(const int &tag)
  { return ttzNodes.findNode(tag); }

//! @brief Returns a pointer to the node cuyo identifier is being passed as parameter.
const XC::Node *XC::EntMdlr::findNode(const int &tag) const
  { return ttzNodes.findNode(tag); }

//! @brief Returns a pointer to the element
//! identified by the tag being passed as parameter.
XC::Element *XC::EntMdlr::findElement(const int &tag)
  { return ttzElements.findElement(tag); }

//! @brief Returns a pointer to the element
//! identified by the tag being passed as parameter.
const XC::Element *XC::EntMdlr::findElement(const int &tag) const
  { return ttzElements.findElement(tag); }

//! @brief Creates a set that corresponds to a row of nodes and elements.
XC::SetEstruct *XC::EntMdlr::create_row_set(const Array3dRange &rango,const std::string &nmb)
  {
    SetEstruct *retval= nullptr;
    if(getPreprocessor())
      {
        MapSet &map_set= getPreprocessor()->get_sets();
        if(rango.isIRow())
          {
            retval= map_set.create_set_estruct(getVarRefIRow(rango,nmb));
          }
        else if(rango.isJRow())
          {
            XC::JRowSet fj= getVarRefJRow(rango,nmb);
            retval= map_set.create_set_estruct(fj);
          }
        else if(rango.isKRow())
          {
            retval= map_set.create_set_estruct(getVarRefKRow(rango,nmb));
          }
        else
	  std::cerr << getClassName() << "::" << __FUNCTION__
	            << "; can't create row set." << std::endl;
      }
    else
      {
	std::cerr << getClassName() << "::" << __FUNCTION__
		  << "; preprocessor undefined." << std::endl;
        return nullptr;
      }
    return retval;
  }

//! @brief Return Simpson's weights.
XC::Vector XC::EntMdlr::getSimpsonWeights(const std::string &ijk,const std::string &strExpr,const size_t &f,const size_t &c, const size_t &n) const
  {
    Vector retval;
    const ExprAlgebra e(strExpr);
    if(ijk=="i")
      retval= ttzNodes.IRowSimpsonIntegration(f,c,e,n);
    else if(ijk=="j")
      retval= ttzNodes.JRowSimpsonIntegration(f,c,e,n);
    else if(ijk=="k")
      retval= ttzNodes.KRowSimpsonIntegration(f,c,e,n);
    return retval;
  }

//! @brief Creates a node at the position being passed as parameter.
XC::Node *XC::EntMdlr::create_node(const Pos3d &pos,size_t i,size_t j, size_t k)
  {
    Node *retval= getPreprocessor()->getNodeHandler().newNode(pos);
    ttzNodes(i,j,k)= retval;
    return retval;
  }

//! @brief Creates nodes at the positions being passed as parameters.
void XC::EntMdlr::create_nodes(const Pos3dArray3d &positions)
  {
    const size_t n_layers= positions.getNumberOfLayers();
    if(n_layers<1) return;
    if(ttzNodes.Null())
      {
        const size_t n_rows= positions(1).getNumberOfRows();
        const size_t cols= positions(1).getNumberOfColumns();
        ttzNodes = NodePtrArray3d(n_layers,n_rows,cols);

        if(!getPreprocessor()) return;
        for(register size_t i= 1;i<=n_layers;i++)
          for(register size_t j= 1;j<=n_rows;j++)
            for(register size_t k= 1;k<=cols;k++)
              create_node(positions(i,j,k),i,j,k);
        if(verbosity>5)
	  std::cerr << getClassName() << "::" << __FUNCTION__
		    << "; created " << ttzNodes.NumPtrs() << " node(s)."
		    << std::endl;
      }
    else
      if(verbosity>2)
        std::clog << getClassName() << "::" << __FUNCTION__
	          << "; nodes from entity: '" << getName()
		  << "' already exist." << std::endl;
  }

//! @brief Creates elements on the nodes created
//! in create_nodes.
bool XC::EntMdlr::create_elements(meshing_dir dm)
  {
    bool retval= false;
    if(!ttzNodes.empty())
      {
        if(ttzNodes.HasNull())
          std::cerr << getClassName() << "::" << __FUNCTION__
	            << "; there are null pointers."
                    << " Elements were not created." << std::endl;
        else
          if(ttzElements.Null())
            {
              if(getPreprocessor())
                {
                  if(verbosity>4)
                    std::clog << "Creating elements of entity: '"
			      << getName() << "'...";   
                  const Element *smll= getPreprocessor()->getElementHandler().get_seed_element();
                  if(smll)
                    {
                      ttzElements= smll->put_on_mesh(ttzNodes,dm);
                      add_elements(ttzElements);
                      retval= true;
                    }
                  else if(verbosity>0)
                    std::clog << getClassName() << "::" << __FUNCTION__
		              << "; seed element not set." << std::endl;
                  if(verbosity>4)
                    std::clog << "created." << std::endl;
                }
              else
                std::cerr << getClassName() << "::" << __FUNCTION__
		          << "; preprocessor undefined." << std::endl;
            }
      }
    else
      std::cerr << getClassName() << "::" << __FUNCTION__
		<< "; there is no nodes for the elements." << std::endl;
    const size_t numElements= ttzElements.NumPtrs();
    if(numElements==0 && verbosity>0)
      std::clog << getClassName() << "::" << __FUNCTION__
	        << "; warning 0 elements created for line: " << getName()
	        << std::endl;

    return retval;
  }

//! @brief Returns true if the points begins or ends in the line.
void XC::EntMdlr::setGenMesh(bool m)
  { doGenMesh= m; }

//! @brief Returns true if the points begins or ends in the line.
const bool &XC::EntMdlr::getGenMesh(void) const
  { return doGenMesh; }

//! @brief Creates a point at the position being passed as parameter.
XC::Pnt *XC::EntMdlr::create_point(const Pos3d &pos)
  { return getPreprocessor()->getMultiBlockTopology().getPoints().New(pos); }

//! @brief Creates points at the positions being passed as parameters.
void XC::EntMdlr::create_points(const Pos3dArray &positions)
  {
    if(verbosity>4)
      std::clog << "Creating points for line: '" << getName() << "'...";   
    const size_t n_rows= positions.getNumberOfRows();
    const size_t cols= positions.getNumberOfColumns();

    if(getPreprocessor())
      {
        size_t cont= 0;
        for(size_t i= 1;i<=n_rows;i++)
          for(size_t j= 1;j<=cols;j++)
            {
              create_point(positions(i,j));
              cont++;
            }
      }
    else
      std::cerr << getClassName() << __FUNCTION__
	        << "; preprocessor needed." << std::endl;
    if(verbosity>4)
      std::clog << "creados." << std::endl;
  }

XC::IRowSet XC::EntMdlr::getVarRefIRow(size_t f,size_t c,const std::string &nmb)
  { return IRowSet(*this,f,c,nmb,getPreprocessor()); }
XC::IRowSet XC::EntMdlr::getVarRefIRow(const RangoIndice &layer_range,size_t f,size_t c,const std::string &nmb)
  { return IRowSet(*this,layer_range,f,c,nmb,getPreprocessor()); }
XC::IRowSet XC::EntMdlr::getVarRefIRow(const Array3dRange &rango,const std::string &nmb)
  { return getVarRefIRow(rango.getLayerRange(),rango.getRowRange().Inf(),rango.getColumnRange().Inf(),nmb); }

XC::JRowSet XC::EntMdlr::getVarRefJRow(size_t layer,size_t c,const std::string &nmb)
  { return JRowSet(*this,layer,c,nmb,getPreprocessor()); }
XC::JRowSet XC::EntMdlr::getVarRefJRow(size_t layer,const RangoIndice &row_range,size_t c,const std::string &nmb)
  { return JRowSet(*this,layer,row_range,c,nmb,getPreprocessor()); }
XC::JRowSet XC::EntMdlr::getVarRefJRow(const Array3dRange &rango,const std::string &nmb)
  { return getVarRefJRow(rango.getLayerRange().Inf(),rango.getRowRange(),rango.getColumnRange().Inf(),nmb); }

XC::KRowSet XC::EntMdlr::getVarRefKRow(size_t layer,size_t f,const std::string &nmb)
  { return KRowSet(*this,layer,f,nmb,getPreprocessor()); }
XC::KRowSet XC::EntMdlr::getVarRefKRow(size_t layer,size_t f,const RangoIndice &column_range,const std::string &nmb)
  { return KRowSet(*this,layer,f,column_range,nmb,getPreprocessor()); }
XC::KRowSet XC::EntMdlr::getVarRefKRow(const Array3dRange &rango,const std::string &nmb)
  { return getVarRefKRow(rango.getLayerRange().Inf(),rango.getRowRange().Inf(),rango.getColumnRange(),nmb); }

//! @brief Return the squared distance to
//! the position being passed as parameter.
double XC::EntMdlr::getSquaredDistanceTo(const Pos3d &pt) const
  {
    std::cerr << getClassName() << "::" << __FUNCTION__
              << "; not implemented." << std::endl;
    return 0.0;
  }

//! @brief Return the distance to the position being passed as parameter.
double XC::EntMdlr::getDistanceTo(const Pos3d &pt) const
  { return sqrt(getSquaredDistanceTo(pt)); }

//! @brief Fixes the nodes of the set.
void XC::EntMdlr::fix(const SFreedom_Constraint &spc)
  { ttzNodes.fix(spc); }


//! @brief Return the centroid of the object.
Pos3d XC::EntMdlr::getCentroid(void) const
  {
    Pos3d retval;
    std::cerr << getClassName() << "::" << __FUNCTION__
              << "; not implemented." << std::endl;
    return retval;
  }
//! @brief Destructor.
XC::EntMdlr::~EntMdlr(void)
  { clearAll(); }
