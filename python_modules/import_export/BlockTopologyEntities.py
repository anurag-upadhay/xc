# -*- coding: utf-8 -*-

__author__= "Luis C. Pérez Tato (LCPT) and Ana Ortega (AOO)"
__copyright__= "Copyright 2015, LCPT and AOO"
__license__= "GPL"
__version__= "3.0"
__email__= "l.pereztato@gmail.com" "anaOrtegaOrt@gmail.com"

import BasicEntities as be
import MeshEntities as me
from miscUtils import LogMessages as lmsg

class PointRecord(me.NodeRecord):
  '''kPoint type entity'''
  def __init__(self,id, coords, labels= None):
    '''
    Key point constructor.

    :param id: identifier for the point.
    :param coords: coordinates of the point.
    :param labels: string list that helps to identify the role of the point in the model.
    '''    
    super(PointRecord,self).__init__(id,coords)
    if(labels):
      self.labels= labels
    else:
      self.labels= list()
  def getStrXCCommand(self,pointHandlerName):
    strId= str(self.id)
    strCommand= '.newPntIDPos3d(' + strId + ',geom.Pos3d(' + str(self.coords[0]) + ',' + str(self.coords[1]) +','+ str(self.coords[2])+'))'
    return 'pt' + strId + '= ' + pointHandlerName + strCommand

class PointDict(me.NodeDict):
  ''' Node container.'''
  def append(self,id,x,y,z):
    self[id]= PointRecord(int(id),[x,y,z])
  def getName(self):
    return 'points'
  def readFromXCSet(self,xcSet):
    pointSet= xcSet.getPoints
    for p in pointSet:
      pos= p.getPos
      self.append(p.tag, pos.x, pos.y, pos.z)
  def writeToXCFile(self,f,xcImportExportData):
    ''' Write the XC commands that define nodes.'''
    for key in self:
      strCommand= self[key].getStrXCCommand(xcImportExportData.pointHandlerName)
      f.write(strCommand+'\n')
      if(self.labels):
        pointName= strCommand.split('= ')[0]
        strCommand= pointName+'.setProp("labels",'+str(self.labels)+')'
        f.write(strCommand+'\n')

class BlockRecord(me.CellRecord):
  '''Block type entities: line, face, body,...'''
  def __init__(self,id, typ, kPoints, labels= None,thk= 0.0):
    '''
    Block record constructor.

    :param id: identifier for the block.
    :param typ: block type.
    :param kPoints: key points that define block geometry and topology.
    :param labels: string list that helps to identify the role of the block in the model.
    '''    
    super(BlockRecord,self).__init__(id,typ,kPoints,thk)
    if(labels):
      self.labels= labels
    else:
      self.labels= list()
  def getType(self):
    return self.cellType
  def getStrKeyPointsIds(self):
    tmp= str(self.nodeIds)
    return tmp[tmp.index("[") + 1:tmp.rindex("]")]
  def getStrXCCommand(self,xcImportExportData):
    strId= str(self.id)
    handlerName= xcImportExportData.getBlockHandlerName(self.getType())
    strCommand= None
    if(self.cellType=='line'):
      strId= 'l'+strId
      strCommand= strId + '= ' + handlerName + '.newLine(' + self.getStrKeyPointsIds() +')'
    elif(self.cellType=='face'):
      strId= 'f'+strId
      strCommand= strId + '= ' + handlerName + '.newQuadSurfacePts(' + self.getStrKeyPointsIds()  +')'
    else:
      lmsg.error('BlockRecord::getStrXCCommand not implemented for blocks of type: '+ self.cellType)
    if(self.labels):
      strCommand+= '; '+strId+'.setProp("labels",'+str(self.labels)+')'
    return strCommand

class BlockDict(dict):
  '''Block container.'''
  def append(self,cell):
    self[cell.id]= cell
  def readFromXCSet(self,xcSet):
    surfaces= xcSet.getSurfaces
    for s in surfaces:
      points= s.getKPoints()
      numPoints= len(points)
      if(numPoints==4):
        tagPoints= [points[0],points[1],points[2],points[3]]
        thickness= s.getElement(1,1,1).getPhysicalProperties.getVectorMaterials[0].h
        block= BlockRecord(s.tag,str(s.tag),tagPoints,thickness)
      elif(numPoints==2):
        tagPoints= [points[0],points[1]]
        block= BlockRecord(s.tag,str(s.tag),tagPoints,0.0)
      self.append(block)        
  def writeDxf(self,nodeDict,drawing):
    '''Write the cells in dxf file.'''
    layerName= 'blocks'
    drawing.add_layer(layerName)
    for key in self:
      self[key].writeDxf(nodeDict,drawing,layerName)
  def writeToXCFile(self,f,xcImportExportData):
    '''Write the XC commands that define the cells (elements).'''
    for key in self:
      cell= self[key]
      type= xcImportExportData.convertCellType(cell.cellType)
      if(type!=None):
        strCommand= self[key].getStrXCCommand(xcImportExportData)
        f.write(strCommand+'\n')
  def getTags(self):
    retval= list()
    for key in self:
      retval.append(self[key].id)
    return retval
  def __str__(self):
    retval= ''
    for key in self:
      retval+= str(self[key]) + '\n'

class PointSupportRecord(be.SupportRecord):
  ''' Constraints for node displacements'''
  def __init__(self, id, pointId, nsr):
    super(PointSupportRecord,self).__init__(id,nsr.xComp, nsr.yComp, nsr.zComp, nsr.rxComp, nsr.ryComp, nsr.rzComp)
    self.nodeId= pointId
  def __str__(self):
    return str(self.id) + ' nodeId: ' + str(self.nodeId) + ' ' + self.getStrConstraints()

class PointSupportDict(dict):
  def append(self, ps):
    if (ps.nodeId in self):
      lmsg.warning('support for node: '+ps.nodeId+' redefined.')
    self[ps.nodeId]= ps
  def readFromXCSet(self,xcSet,points):
    '''Read supports from an XC set.'''
    nodeSupports= me.NodeSupportDict()
    preprocessor= xcSet.getPreprocessor
    nodeSupports.readFromXCDomain(preprocessor.getDomain)
    nodeSupportsTags= nodeSupports.getNodeTags()
    supportId= 0
    for k in points:
      p= preprocessor.getMultiBlockTopology.getPoints.get(k)
      pointTag= p.tag
      node= p.getNode()
      if(node):
        nodeTag= node.tag
        if(nodeTag in nodeSupportsTags):
          psr= PointSupportRecord(supportId,pointTag,nodeSupports[nodeTag])
          self.append(psr)
          supportId+= 1

class BlockData(object):
  '''Block topology entities container: points, lines, faces, solids,... '''
  def __init__(self):
    self.name= None
    self.materials= me.MaterialDict()
    self.points= PointDict()
    self.blocks= BlockDict()
    self.pointSupports= PointSupportDict()


  def appendPoint(self,id,x,y,z,labels= None):
    self.points[id]= PointRecord(int(id),[x,y,z],labels)
  def appendBlock(self,block):
    self.blocks[block.id]= block

  def readFromXCSet(self,xcSet):
    '''Read points and surfaces from an XC set.'''
    self.points.readFromXCSet(xcSet)
    self.blocks.readFromXCSet(xcSet)
    self.pointSupports.readFromXCSet(xcSet,self.points)

  def readFromDxfFile(self,fName,preprocessor,dxfLayers):
    dxfReader= DxfReader.OldDxfReader()
    dxfReader.read(fName,preprocessor,dxfLayers)
    pointSet= preprocessor.getSets.getSet("total").getPoints
    for p in pointSet:
      pos= p.getPos
      self.appendPoint(p.tag, pos.x, pos.y, pos.z)
    lineSet= preprocessor.getSets.getSet("total").getLines
    for l in lineSet:
      points= l.getKPoints()
      tagPoints= [points[0],points[1]]
      block= BlockRecord(l.tag,'line',tagPoints)
      self.appendBlock(block)
    self.pointsInLayer= dxfReader.pointsInLayer
    self.blocksInLayer= dxfReader.blocksInLayer

  def writeDxf(self,drawing):
    '''Write the blocs (point, lines, surfaces, volumes,...) in a DXF file'''
    layerName= 'points'
    drawing.add_layer(layerName)
    for key in self.points:
      self.points[key].writeDxf(drawing,layername)

    drawing.add_layer('lines')
    drawing.add_layer('surfaces')
    drawing.add_layer('volumes')
    for key in self.blocks:
      block= self.blocks[key]
      block.writeDxf(drawing)
    

  def writeToXCFile(self,xcImportExportData):
    f= xcImportExportData.outputFile
    for key in self.points:
      strCommand= self.points[key].getStrXCCommand(xcImportExportData.pointHandlerName)
      f.write(strCommand+'\n')
    for key in self.blocks:
      block= self.blocks[key]
      strCommand= block.getStrXCCommand(xcImportExportData)
      f.write(strCommand+'\n')

  def getPointTags(self):
    tags= []
    for key in self.points:
      tags.append(self.points[key].id)
    return tags

  def getBlockTags(self):
    tags= []
    for key in self.blocks:
      tags.append(self.blocks[key].id)
    return tags


  def __str__(self):
    for key in self.points:
      retval+= str(self.points[key]) + '\n'
    for key in self.blocks:
      retval+= str(self.blocks[key]) + '\n'
    return retval
