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
//KRowSet.cc

#include "KRowSet.h"
#include "preprocessor/multi_block_topology/entities/EntMdlr.h"
#include <boost/any.hpp>
#include "domain/mesh/node/Node.h"
#include "domain/mesh/element/Element.h"

XC::KRowSet::KRowSet(EntMdlr &e,const size_t &f,const size_t &c,const std::string &nmb,XC::Preprocessor *preprocessor)
  : RowSet<tNodeRow,tElemRow>(e.getTtzNodes().getVarRefKRow(f,c),e.getTtzElements().getVarRefKRow(f,c),nmb,preprocessor) {}

XC::KRowSet::KRowSet(EntMdlr &e,const size_t &layer,const size_t &f,const RangoIndice &column_range,const std::string &nmb,Preprocessor *preprocessor)
  : RowSet<tNodeRow,tElemRow>(e.getTtzNodes().getVarRefKRow(layer,f,column_range),e.getTtzElements().getVarRefKRow(layer,f,column_range),nmb,preprocessor) {}
