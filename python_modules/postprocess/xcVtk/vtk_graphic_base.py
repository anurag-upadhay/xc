# -*- coding: utf-8 -*-

''' Display nice images of the model. '''

__author__= "Luis C. Pérez Tato (LCPT) , Ana Ortega (AO_O) "
__copyright__= "Copyright 2016, LCPT, AO_O"
__license__= "GPL"
__version__= "3.0"
__email__= "l.pereztato@ciccp.es, ana.ortega@ciccp.es "


import sys
import vtk
import xc_base
import geom
from postprocess.xcVtk import screen_annotation as sa
from miscUtils import LogMessages as lmsg


class RecordDefGrid(object):
  '''Provide the variables involved in the VTK grid representation
  
  :ivar xcSet:     set to be represented
  :ivar entToLabel:  entities to be labeled (defaults to "nodes")
  :ivar cellType:    specifies the type of data cells (defaults to "nil"). 
        Data cells are simple topological elements like points, lines, 
        polygons and tetrahedra of which visualization data sets are composed.
  :ivar uGrid: unstructure grid (defaults to None). An unstructure grid is a 
        concrete implementation of a vtk data set; represents any combination 
        of any cell types. This includes 0D (e.g. points), 1D (e.g., lines, 
        polylines), 2D (e.g., triangles, polygons), and 3D (e.g., hexahedron, 
        tetrahedron, polyhedron, etc.).

  '''
  def __init__(self):
    self.xcSet= None
    self.entToLabel= "nodes"
    self.cellType= "nil"
    self.uGrid= None

  def getBND(self):
    ''' Returns the grid boundary'''
    retval= geom.BND3d()
    points= self.uGrid.GetPoints()
    if(points.GetNumberOfPoints()>0):
      bounds= points.GetBounds()
      retval= geom.BND3d(geom.Pos3d(bounds[0],bounds[2],bounds[4]),geom.Pos3d(bounds[1],bounds[3],bounds[5]))
    else:
      warnMsg= 'there are no points in the grid: '
      warnMsg+= self.uGrid.name
      warnMsg+= '. Maybe you must call fillDownwards on the set to display.'
      lmsg.warning('Warning; '+warnMsg)
    return retval

class CameraParameters(object):
  ''' Provides the parameters to define the camera.
  
  :ivar viewName:    name of the view that contains the renderer 
        (defaults to "XYZPos")
  :ivar viewUpVc: vector defined as [x,y,z] to orient the view. This vector of the
                   model is placed in vertical orientation in the display
  :ivar posCVc:    vector defined as [x,y,z] that points to the camera position
  :ivar zoom:        (defaults to 1.0)
  :ivar hCamFct:     factor that applies to the height of the camera position 
        in order to change perspective of isometric views 
        (defaults to 1, usual values 0.1 to 10)
  '''
  def __init__(self, viewNm= 'XYZPos', hCamF= 1.0):
    self.viewName= viewNm
    self.viewUpVc= [0,1,0]
    self.posCVc= [0,0,100]
    self.zoom= 1.0
    self.hCamFct= hCamF
    self.defineViewParametersFromViewName()
    
  def defineViewParametersFromViewName(self):
    '''Sets the values of the view parameters
    from the following predefined viewNames:
    "ZPos","ZNeg","YPos","YNeg","XPos","XNeg","XYZPos"
    Zpos: View from positive Z axis (Z+)
    Zneg: View from negative Z axis (Z-)
    Ypos: View from positive Y axis (Y+)
    Yneg: View from negative Y axis (Y-)
    Xpos: View from positive X axis (X+)
    Xneg: View from negative X axis (X-)
    XYZPos or +X+Y+Z: View from point (1,1,1)
    +X+Y-Z: View from point (1,1,-1)
    +X-Y+Z: View from point (1,-1,1)
    +X-Y-Z: View from point (1,-1,-1)
    -X+Y+Z: View from point (-1,1,1)
    -X+Y-Z: View from point (-1,1,-1)
    -X-Y+Z: View from point (-1,-1,1)
    XYZNeg or -X-Y-Z: View from point (-1,-1,-1)
    '''
    if(self.viewName=="ZPos"):
      self.viewUpVc= [0,1,0]
      self.posCVc= [0,0,100]
    elif(self.viewName=="ZNeg"):
      self.viewUpVc= [0,1,0]
      self.posCVc= [0,0,-100]
    elif(self.viewName=="YPos"):
      self.viewUpVc= [0,0,1]
      self.posCVc= [0,100,0]
    elif(self.viewName=="YNeg"):
      self.viewUpVc= [0,0,1]
      self.posCVc= [0,-100,0]
    elif(self.viewName=="XPos"):
      self.viewUpVc= [0,0,1]
      self.posCVc= [100,0,0]
    elif(self.viewName=="XNeg"):
      self.viewUpVc= [0,0,1]
      self.posCVc= [-100,0,0]
    elif(self.viewName=="XYZPos" or self.viewName=="+X+Y+Z"):
      self.viewUpVc= [-1,-1,1]
      self.posCVc= [100,100,self.hCamFct*100]
    elif(self.viewName=="+X+Y-Z"):
      self.viewUpVc= [1,1,1]
      self.posCVc= [100,100,-1*self.hCamFct*100]
    elif(self.viewName=="+X-Y+Z"):
      self.viewUpVc= [-1,1,1]
      self.posCVc= [100,-100,self.hCamFct*100]
    elif(self.viewName=="+X-Y-Z"):
      self.viewUpVc= [1,-1,1]
      self.posCVc= [100,-100,-1*self.hCamFct*100]
    elif(self.viewName=="-X+Y+Z"):
      self.viewUpVc= [1,-1,1]
      self.posCVc= [-100,100,self.hCamFct*100]
    elif(self.viewName=="-X+Y-Z"):
      self.viewUpVc= [-1,+1,1]
      self.posCVc= [-100,100,-1*self.hCamFct*100]
    elif(self.viewName=="-X-Y+Z"):
      self.viewUpVc= [1,1,1]
      self.posCVc= [-100,-100,self.hCamFct*100]
    elif(self.viewName=="XYZNeg" or self.viewName=="-X-Y-Z"):
      self.viewUpVc= [-1,-1,1]
      self.posCVc= [-100,-100,-1*self.hCamFct*100]
    elif(self.viewName!="Custom"):
      sys.stderr.write("View name: '"+self.viewName+"' unknown.")
  
  def setView(self, camera):
    '''Sets the camera parameters.'''
    if(self.viewName!="Custom"):
      self.defineViewParametersFromViewName()
    camera.SetViewUp(self.viewUpVc[0],self.viewUpVc[1],self.viewUpVc[2])
    camera.SetPosition(self.posCVc[0],self.posCVc[1],self.posCVc[2])
    camera.SetParallelProjection(1)
    camera.Zoom(self.zoom)

class RecordDefDisplay(object):
  ''' Provides the variables to define the output device.
  
  :ivar renderer:    specification of renderer. A renderer is an object that
        controls the rendering process for objects. Rendering is the 
        process of converting geometry, a specification for lights, and
        a camera view into an image. (defaults to None)
  :ivar renWin:  rendering window (defaults to None). A rendering window is a 
        window in a graphical user interface where renderers draw their images. 
  :ivar windowWidth: resolution expresed in pixels in the width direction of 
        the window (defaults to 800)
  :ivar windowHeight: resolution expresed in pixels in the height direction of 
        the window (defaults to 600)
  :ivar cameraParameters: parameters that define the camera position,
                          zoom, etc.
  :ivar bgRComp:     red component (defaults to 0.65)
  :ivar bgGComp:     green component (defaults to 0.65)
  :ivar bgBComp:     blue component (defaults to 0.65)
  '''
  def __init__(self):
    self.renderer= None
    self.renWin= None
    self.windowWidth= 800
    self.windowHeight= 600
    self.annotation= sa.ScreenAnnotation()
    self.bgRComp= 0.65
    self.bgGComp= 0.65
    self.bgBComp= 0.65
    self.cameraParameters= CameraParameters()


  def setView(self):
    '''Sets the view'''
    self.renderer.ResetCamera()
    cam= self.renderer.GetActiveCamera()
    self.cameraParameters.setView(cam)
    self.renderer.ResetCameraClippingRange()


  def setupAxes(self):
    '''Add an vtkAxesActor to the renderer.'''
    bnd= self.gridRecord.getBND()
    offsetVector= bnd.diagonal*0.1
    offset= offsetVector.getModulo()
    axesPosition= bnd.pMin-offsetVector
    transform = vtk.vtkTransform()
    transform.Translate(axesPosition.x, axesPosition.y, axesPosition.z)
    axes= vtk.vtkAxesActor()
    #  The axes are positioned with a user transform
    axes.SetUserTransform(transform)

    length= offset
    axes.SetTotalLength(length,length,length)

    textSize= int(3*offset)
    axes.GetXAxisCaptionActor2D().GetTextActor().SetTextScaleMode(False)
    axes.GetXAxisCaptionActor2D().GetTextActor().GetTextProperty().SetFontSize(textSize)
    axes.GetYAxisCaptionActor2D().GetTextActor().SetTextScaleMode(False)
    axes.GetYAxisCaptionActor2D().GetTextActor().GetTextProperty().SetFontSize(textSize)
    axes.GetZAxisCaptionActor2D().GetTextActor().SetTextScaleMode(False)
    axes.GetZAxisCaptionActor2D().GetTextActor().GetTextProperty().SetFontSize(textSize)

    # properties of the axes labels can be set as follows
    # this sets the x axis label to red
    # axes.GetXAxisCaptionActor2D().GetCaptionTextProperty().SetColor(1,0,0)
 
    # the actual text of the axis label can be changed:
    # axes.SetXAxisLabelText("test")
    self.renderer.AddActor(axes)

  def setupWindow(self,caption= ''):
    '''sets the rendering window. A rendering window is a window in a
       graphical user interface where renderers draw their images.
    '''
    self.renWin= vtk.vtkRenderWindow()
    self.renWin.SetSize(self.windowWidth,self.windowHeight)
    self.renWin.AddRenderer(self.renderer)
    #Axes
    self.setupAxes()

    #Time stamp and window decorations.
    if(caption==''):
      lmsg.warning('setupWindow; window caption empty.')
    vtkCornerAnno= self.annotation.getVtkCornerAnnotation(caption)
    self.renderer.AddActor(vtkCornerAnno)
    return self.renWin

  def setupWindowInteractor(self):
    '''sets the window interactor, which provides a platform-independent
    interaction mechanism for mouse/key/time events.
    '''
    iren= vtk.vtkRenderWindowInteractor()
    iren.SetRenderWindow(self.renWin)
    iren.SetSize(self.windowWidth,self.windowHeight)
    iren.Initialize()
    return iren

  def displayScene(self,caption= '', fName= None):
    ''' Displaying scene

    :param caption: caption to display with the scene.
    :param fName: name of the image file, in none -> screen window.
    '''
    self.setView()
    self.setupWindow(caption)
    if(fName):
      self.plot(fName)
    else:
      iren= self.setupWindowInteractor() 
      iren.Start()   

  def muestraEscena(self):
    lmsg.warning('muestraEscena is deprecated. Use displayScene')
    self.displayScene('noCaption', None)

  def setupGrid(self,xcSet):
    ''' Parameters:
       xcSet:     set to be represented
    '''
    self.gridRecord= RecordDefGrid()
    self.gridRecord.xcSet= xcSet
    return self.gridRecord

  def displayGrid(self, caption= ''):
    '''Displays the grid in the output device

    :param caption: caption to display with the scene.
    '''
    self.defineMeshScene(None)
    self.displayScene(caption)

  def plot(self,fName):
    '''Plots window contents'''
    w2i = vtk.vtkWindowToImageFilter()
    writer = vtk.vtkJPEGWriter()
    w2i.SetInput(self.renWin)
    w2i.Update()
    writer.SetInputConnection(w2i.GetOutputPort())
    writer.SetFileName(fName)
    self.renWin.Render()
    w2i.Update()
    writer.Write()
 
 
