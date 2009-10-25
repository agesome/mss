class BlockGraph
  # cairo context, current drawing coolor
  attr_accessor :context, :lwidth

  def initialize xsize, ysize
    @zline = ysize / 2
    @cx = 0
  end

  def draw_stick h, w
    context.set_line_width lwidth
    context.set_source_rgb 0.45, 0.823, 0.086
    context.rectangle @cx, @zline, w, -h
    context.stroke 1
    context.rectangle @cx + lwidth, @zline - lwidth, w - lwidth, -h + lwidth
    context.set_source_rgb 0.3, 0.6, 0.02
    context.fill
    context.stroke
    @cx += w
  end
end
