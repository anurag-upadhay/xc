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

class_<XC::LineSearch, bases<XC::MovableObject,EntCmd>, boost::noncopyable >("LineSearch", no_init);

class_<XC::BisectionLineSearch, bases<XC::LineSearch>, boost::noncopyable >("BisectionLineSearch", no_init);

class_<XC::InitialInterpolatedLineSearch, bases<XC::LineSearch>, boost::noncopyable >("InitialInterpolatedLineSearch", no_init);

class_<XC::RegulaFalsiLineSearch, bases<XC::LineSearch>, boost::noncopyable >("RegulaFalsiLineSearch", no_init);

class_<XC::SecantLineSearch, bases<XC::LineSearch>, boost::noncopyable >("SecantLineSearch", no_init);
