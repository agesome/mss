class Graph
  attr_accessor :cr

  def initialize(points)
    @points = points
  end

  def curve
    prepared_data = []
    for i in 0...@points.size
      x, y = @points[i][0], @points[i][1]
      
      if i != 0 and i != @points.size - 1
        x_left, y_left = @points[i - 1][0], @points[i - 1][1]
        x_right, y_right = @points[i + 1][0], @points[i + 1][1]
        step_x_left = (x - x_left) / 2
        step_x_right = (x_right - x) / 2
        dx, dy = x_right - x_left, y_right - y_left
        h = Math.sqrt(dx**2 + dy**2)
        if h == 0
          cx1, cy1, cx2, cy2 = x, y, x, y
        else
          dx1, dy1 = (dx * step_x_left) / h, (dy * step_x_left) / h
          dx2, dy2 = (dx * step_x_right) / h, (dy * step_x_right) / h
          cx1, cx2 = x - dx1, x + dx2
          cy1, cy2 = y - dy1, y + dy2
        end
      else
        cx1, cy1, cx2, cy2 = x, y, x, y
      end
      prepared_data << [x, y, cx1, cy1, cx2, cy2]
    end
    
    for i in 0...prepared_data.size - 1 #range(0, len(prepared_data) - 1):
      x, y = prepared_data[i][0], prepared_data[i][1]
      cx1, cy1 = prepared_data[i][4], prepared_data[i][5]
      cx2, cy2 = prepared_data[i + 1][2], prepared_data[i + 1][3]
      x2, y2 = prepared_data[i + 1][0], prepared_data[i + 1][1]
      cr.move_to(x, y)
      cr.curve_to(cx1, cy1, cx2, cy2, x2, y2)
      cr.stroke
    end
  end
end
