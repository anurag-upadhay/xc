# -*- coding: utf-8 -*-

import math
import cairo
import xc_base
import geom
from geom_utils import auxCairoPlot

# Dibuja las armaduras de la sección en un archivo PostScript
def plotArmaduras(armaduras, ctx):
  fmt= "%u"
  print "numReinfBars= ", armaduras.getNumReinfBars
  barras= armaduras.getReinfBars
  for b in barras:
    ptPlot= b.getPos2d
    rPlot= b.getBarDiam/2.0
    labelPlot= fmt.format(round(getBarDiam*1e3))
    ctx.set_line_width(rPlot/5)
    ctx.arc(ptPlot.x,ptPlot.y,rPlot,2*math.pi)
    ctx.move_to(ptPlot.x+1.3*rPlot,ptPlot.y)
    ctx.set_font_size(3*rPlot)
    ctx.text_path(labelPlot)

# Dibuja la geometría de la sección en un archivo PostScript
def plotGeomSeccion(geomSection, path):
  WIDTH, HEIGHT = 256, 256
  surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, WIDTH, HEIGHT)
  ctx = cairo.Context(surface)
  ctx.scale (WIDTH, HEIGHT) # Normalizing the canvas
  ctx.set_line_width(0.01)
  ctx.set_source_rgb(0, 0, 0) # Black solid color 
  regiones= geomSection.getRegions
  for r in regiones:
    auxCairoPlot.plotPolygon(r.getPoligono(),ctx)
  armaduras= geomSection.getReinfLayers
  plotArmaduras(armaduras,ctx)
  geom_utils.plotEjesYz(0.06,ctx)
  ctx.stroke()
  surface.write_to_eps(path) # Output to PNG