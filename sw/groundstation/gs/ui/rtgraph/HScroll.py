from __future__ import division

from Graph import PolledGraph
import Tweak
import pango, gtk, os

class _Label:
    def __init__(self, context, value, y):
        self.layout = pango.Layout(context)
        self.set_value(value)
        self.y = y - (self.height // 2)

    def set_value(self, value):
        self.layout.set_text("%+2.2f" % value)
        self.width, self.height = self.layout.get_pixel_size()

class HScrollGraph(PolledGraph):
    """A graph that shows time on the horizontal axis, multiple channels
       of data on the Y axis, and scrolls horizontally so current data
       is always on the right edge of the graph.

           gridSize: grid size, in pixels
         scrollRate: Graph scrolling rate, in pixels per second
       """
    def __init__(self,
                 size         = (384,128),
                 channels     = [],
                 gridSize     = 32,
                 scrollRate   = 50,
                 pollInterval = 10,
                 bgColor      = None,
                 gridColor    = None,
                 axisLabel    = False
                 ):
        PolledGraph.__init__(self, size, channels, pollInterval, bgColor, gridColor)
        self.gridSize = gridSize
        self.scrollRate = scrollRate
        self.axisLabel = axisLabel
        self.gridPhase = 0.0       # Number of pixels we've scrolled, modulo gridSize

        # Normally we copy from backbuffer to backbuffer when scrolling.
        # This breaks under the win32 implementation of GDK, so use a temporary
        # buffer then.
        self.useTemporaryPixmap = os.name == "nt"

    def resized(self):
        PolledGraph.resized(self)
        if self.useTemporaryPixmap:
            self.tempPixmap = gtk.gdk.Pixmap(self.window, self.width, self.height)

    def graphChannel(self, channel):
        """Hook for graphing the current values of each channel. Called for
           every channel each time the graph is scrolled, and called on one
           channel each time that channel changes.
           """
        pass

    def clearAxis(self):
        #clear right hand side rectangle, if any
        if self.gwidth != self.width:
            self.backingPixmap.draw_rectangle(self.bgGc, True, self.gwidth, 0, self.width - self.gwidth, self.height)

        #clear bottom rectangle, if any
        if self.gheight != self.height:
            self.backingPixmap.draw_rectangle(self.bgGc, True, 0, self.gheight, self.width, self.height)

    def drawBackground(self):
        """Draw our grid pixmap and backing store"""
        if self.axisLabel:
            #guess a default text width
            lw = _Label(self.get_pango_context(), 0.0, 0).width
            #restrict the graph width to a subregion of the total pixmap. The
            #axis labels are now drawn on the right of this
            self.gwidth = self.width - (lw + 1)
            self.gheight = self.height - 0

        self.initGrid(self.gridSize, self.gheight)
        # Initialize the backing store
        self.drawGrid(0, self.gwidth)

    def drawAxis(self):
        """Draw axis labels"""
        if self.axisLabel:
            self.clearAxis()

            pc = self.get_pango_context()

            top = self.range[1]
            nsteps = self.gheight // self.gridSize

            if nsteps:
                step = (self.range[1] - self.range[0]) / float(nsteps)

                #skip don't label the top grid
                grids = range(self.gridSize, self.gheight, self.gridSize)
                value = top - step

                # Draw the axis labels into the graph
                labels = []
                for y in grids:
                    l = _Label(pc, value, y)
                    self.backingPixmap.draw_layout(self.gc, self.gwidth+1, l.y, l.layout)
                    value -= step

                # blit the labels to the screen
                self.queue_draw_area(self.gwidth, 0, self.width, self.gheight)

    def initGrid(self, width, height):
        """Draw our grid on the given drawable"""
        # Create a grid pixmap as wide as our grid and as high as our window,
        # used to quickly initialize new areas of the graph with our grid pattern.
        self.gridPixmap = gtk.gdk.Pixmap(self.window, self.gridSize, self.gheight)

        self.gridPixmap.draw_rectangle(self.bgGc, True, 0, 0, width, height)

        # Horizontal grid lines
        for y in range(0, height, self.gridSize):
            self.gridPixmap.draw_rectangle(self.gridGc, True, 0, y, width, 1)

        # Vertical grid lines
        for x in range(0, width, self.gridSize):
            self.gridPixmap.draw_rectangle(self.gridGc, True, x, 0, 1, height)

    def drawGrid(self, x, width):
        """Draw grid lines on our backing store, using the current gridPhase,
           to the rectangle (x, 0, width, self.gheight)
           """
        srcOffset = (x + int(self.gridPhase)) % self.gridSize
        gc = self.get_style().fg_gc[gtk.STATE_NORMAL]

        if srcOffset > 0:
            # Draw the first partial grid column
            columnWidth = self.gridSize - srcOffset
            if columnWidth > width:
                columnWidth = width
            self.backingPixmap.draw_drawable(gc, self.gridPixmap, srcOffset, 0, x, 0,
                                             columnWidth, self.gheight)
            x += columnWidth
            width -= columnWidth

        while width > 0:
            # Draw each remaining full or partial grid column
            columnWidth = self.gridSize
            if columnWidth > width:
                columnWidth = width
            self.backingPixmap.draw_drawable(gc, self.gridPixmap, 0, 0, x, 0,
                                             columnWidth, self.gheight)
            x += self.gridSize
            width -= self.gridSize

    def integrate(self, dt):
        """Update the graph, given a time delta from the last call to this function"""

        # Can't update if we aren't mapped
        if not (self.width and self.height):
            return

        # Calculate the new gridPhase and the number of freshly exposed pixels,
        # correctly accounting for subpixel gridPhase changes.
        oldGridPhase = self.gridPhase
        self.gridPhase += dt * self.scrollRate
        newPixels = int(self.gridPhase) - int(oldGridPhase)
        self.gridPhase %= self.gridSize

        if newPixels > 0:
            # Scroll the backing store left by newPixels pixels
            gc = self.get_style().fg_gc[gtk.STATE_NORMAL]
            if not gc:
                return

            if self.useTemporaryPixmap:
                # We can't safely copy from and to the same pixmap, copy this
                # via a temporary off-screen buffer.
                self.tempPixmap.draw_drawable(gc, self.backingPixmap, newPixels, 0, 0, 0,
                                              self.gwidth - newPixels, self.gheight)
                self.backingPixmap.draw_drawable(gc, self.tempPixmap, 0, 0, 0, 0,
                                                 self.gwidth - newPixels, self.gheight)
            else:
                # Copy directly from and to the backbuffer
                self.backingPixmap.draw_drawable(gc, self.backingPixmap, newPixels, 0, 0, 0,
                                                 self.gwidth - newPixels, self.gheight)

            # Draw a blank grid in the new area
            self.drawGrid(self.gwidth - newPixels, newPixels)

            # Let subclasses update their positions to account for our scrolling
            self.exposedPixels(newPixels)

            # Graph all channels
            for channel in self.channels:
                # Effectively clear the channel's "dirty flag", so in deciding
                # whether a draw is necessary if we're not scrolling we don't account
                # for changes that will be drawn now.
                channel.hasChanged(self)
                self.graphChannel(channel)

            # Schedule an expose event to blit the changed part of the 
            # backbuffer to the screen
            self.queue_draw_area(newPixels, 0, self.gwidth, self.gheight)

        else:
            # Even if we're not scrolling, we should update the graph if the channel
            # values have changed. This is especially necessary when the channels
            # are being updated much more often than the graph is scrolled.
            for channel in self.channels:
                if channel.hasChanged(self):
                    self.graphChannel(channel)
            self.queue_draw_area(self.gwidth-1, 0, 1, self.gheight)

    def exposedPixels(self, nPixels):
        """Called when the graph scrolls, with the number of pixels it has scrolled by.
           Used as a hook for updating drawing coordinates in subclasses.
           """
        pass

    def getTweakControls(self):
        return PolledGraph.getTweakControls(self) + [
            Tweak.Quantity(self, 'scrollRate', range=(0,200), name="Scroll Rate")
            ]

class HScrollLineGraph(HScrollGraph):
    """A horizontally scrolling real-time line plot.
       Expects scalar values within the given range.
       """
    def __init__(self,
                 size         = (384,128),
                 channels     = [],
                 gridSize     = 32,
                 scrollRate   = 50,
                 range        = (0,1),
                 autoScale    = False,
                 axisLabel    = False,
                 pollInterval = 10,
                 bgColor      = None,
                 gridColor    = None,
                 ):
        HScrollGraph.__init__(self, size, channels, gridSize,
                              scrollRate, pollInterval, bgColor, 
                              gridColor, axisLabel)
        self.autoScale = autoScale
        self.range = list(range)
        self.penVectors = {}

    def rescale(self, val, idx):
        self.range[idx] = val

        self.drawBackground()
        self.drawAxis()
        self.resized()

    def graphChannel(self, channel):
        value = channel.getValue()
        if value is None:
            return

        if self.autoScale and value > self.range[1]:
            return self.rescale(value, 1)
        if self.autoScale and value < self.range[0]:
            return self.rescale(value, 0)

        # Scale the channel value to match a range of (0,1)
        scaled = (value - self.range[0]) / (self.range[1] - self.range[0])
        scaled = min(1.5, max(-0.5, scaled))

        # Calculate a current pen position, always at the right side of the graph
        penVector = (self.gwidth-1, int((self.gheight-1) * (1-scaled)))

        # If we have both a new pen vector and an old pen vector, we can draw a line
        if self.penVectors.has_key(channel):
            oldPenVector = self.penVectors[channel]
            self.graphChannelLine(channel, oldPenVector, penVector)

        # Store the pen vector for later
        self.penVectors[channel] = penVector

    def graphChannelLine(self, channel, oldPenVector, penVector):
        self.backingPixmap.draw_line(channel.getGC(self),
                                     oldPenVector[0], oldPenVector[1],
                                     penVector[0], penVector[1])

    def resized(self):
        HScrollGraph.resized(self)

        # Invalidate saved pen vectors
        self.penVectors = {}

    def exposedPixels(self, nPixels):
        """Scrolls our old pen vectors along with the graph,
           culling out old vectors while we're at it.
           """
        # Note that it's important to use items() here,
        # ince the penVectors dict might change while we're
        # iterating.
        for channel, penVector in self.penVectors.items():
            if channel in self.channels:
                self.penVectors[channel] = (
                    penVector[0] - nPixels,
                    penVector[1])
            else:
                del self.penVectors[channel]

class HScrollAreaGraph(HScrollLineGraph):
    """A horizontally scrolling real-time filled area plot."""
    def graphChannelLine(self, channel, oldPenVector, penVector):
        self.backingPixmap.draw_polygon(channel.getGC(self), True, (
            (oldPenVector[0], oldPenVector[1]),
            (penVector[0], penVector[1]),
            (penVector[0], self.gheight-1),
            (oldPenVector[0], self.gheight-1)))
