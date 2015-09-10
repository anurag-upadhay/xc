//----------------------------------------------------------------------------
//  programa XC; cálculo mediante el método de los elementos finitos orientado
//  a la solución de problemas estructurales.
//
//  Copyright (C)  Luis Claudio Pérez Tato
//
//  Este software es libre: usted puede redistribuirlo y/o modificarlo 
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
//python_interface.tcc

class_<XC::BidimLoad, bases<XC::ElementBodyLoad>, boost::noncopyable >("BidimLoad", no_init)
  .add_property("category", &XC::BidimLoad::Categoria)
  ;

class_<XC::BidimMecLoad, bases<XC::BidimLoad>, boost::noncopyable >("BidimMecLoad", no_init)
  .add_property("axial1Component", &XC::BidimMecLoad::getAxial1Component, &XC::BidimMecLoad::setAxial1Component)
  .add_property("axial2Component", &XC::BidimMecLoad::getAxial2Component, &XC::BidimMecLoad::setAxial2Component)
  .add_property("transComponent", &XC::BidimMecLoad::getTransComponent, &XC::BidimMecLoad::setTransComponent)
  ;

class_<XC::ShellMecLoad, bases<XC::BidimMecLoad>, boost::noncopyable >("ShellMecLoad", no_init)
  .add_property("getDimVectorFuerza", &XC::ShellMecLoad::getDimVectorFuerza)
  .add_property("getDimVectorMomento", &XC::ShellMecLoad::getDimVectorMomento)
  .def("getLocalForces",make_function(&XC::ShellMecLoad::getLocalForces, return_internal_reference<>() ))
  .def("getLocalMoments",make_function(&XC::ShellMecLoad::getLocalMoments, return_internal_reference<>() ))
  .def("getGlobalVectors",make_function(&XC::ShellMecLoad::getGlobalVectors, return_internal_reference<>() ))
  .def("getGlobalForces",make_function(&XC::ShellMecLoad::getGlobalForces, return_internal_reference<>() ))
  .def("getGlobalMoments",make_function(&XC::ShellMecLoad::getGlobalMoments, return_internal_reference<>() ))
  ;

class_<XC::ShellUniformLoad, bases<XC::ShellMecLoad>, boost::noncopyable >("ShellUniformLoad", no_init)
  ;

class_<XC::BidimStrainLoad, bases<XC::BidimLoad>, boost::noncopyable >("BidimStrainLoad", no_init)
  .def("getStrain",make_function(&XC::BidimStrainLoad::getDeformacion, return_internal_reference<>() ))
.def("setStrainComp",&XC::BidimStrainLoad::setStrainComp)
  ;

class_<XC::ShellStrainLoad, bases<XC::BidimStrainLoad>, boost::noncopyable >("ShellStrainLoad", no_init)
  ;


