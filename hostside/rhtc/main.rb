#!/usr/bin/env ruby

require 'gtk2'
require 'cairo'
require 'graph'
require 'usbif'
require 'gui'

class MainGui
  WTITLE='rHTC'
  MAXTEMP = 125.0

  def initialize
    begin
      @dg = DataGetter.new(1000, @sb)
    rescue StandardError => why
      puts(why.to_s)
      Kernel::exit
    end
    
    init_base
    init_menu
    init_log
    init_statusbar
    init_data
    init_graphs
  end

  public

  def begin
    trap("INT") { exit }
    @wmain.show_all
    Gtk.main
  end

  def exit
    @dg.disconnect
    Gtk.main_quit
  end
  
  private

  def init_base
    @wmain = Gtk::Window.new
    @wmain.set_title WTITLE
    @wmain.signal_connect("delete-event") { exit }

    @box_main = Gtk::VBox.new
    @boxes = {
      :menu => Gtk::HBox.new,
      :data => Gtk::HBox.new,
      :graphs => Gtk::HBox.new,
      :controls => Gtk::HBox.new,
      :status => Gtk::HBox.new
    }
    @wmain.add(@box_main)
    @boxes.values.each { |box| @box_main.pack_start(box) }

  end

  def init_menu
    @menu = MenuBar.new
    @menu.add_item("Main")
    @menu.add_subitem("Main", "Quit") { exit }

    @menu.add_subitem("Main", "Log") { @logwindow.vtoggle }

    @menu.add_to(@boxes[:menu])
  end

  def init_statusbar
    @statusbar = Gtk::Statusbar.new
    @boxes[:status].add(@statusbar)
    @sb = StatusBar.new(@statusbar, @logwindow)
  end

  def init_log
  @logwindow = LogWindow.new(@wmain)
  end

  def init_data
    @pbar = Gtk::ProgressBar.new
    @boxes[:data].add(@pbar)

    GLib::Timeout.add(750) do
      @pbar.fraction =  @dg.temp / 10 / MAXTEMP
      @pbar.text = "Temperature: #{@dg.temp.to_f / 10} C"
      true
    end
  end


  def init_graphs
    @tgraph = Graph.new(@dg.points)
    da = Gtk::DrawingArea.new
    da.set_size_request(300, 200)
    @boxes[:graphs].pack_start(da)
    da.signal_connect("expose-event") {
      @tgraph.cr = da.window.create_cairo_context
      @tgraph.curve
    }
  end

end

# form a log of statusbar messages
class LogWindow

  def initialize(window)
    @logw = Gtk::Window.new("Events log")
    @buffer = Gtk::TextBuffer.new
    @view = Gtk::TextView.new
    @view.buffer = @buffer
    @logw.add(@view)
  end

  def add_msg(text)
    @buffer.insert_at_cursor(text + "\n")
  end

  def show
    @logw.show_all
  end

  def hide
    @logw.hide_all
  end

  def vtoggle
    if @logw.visible? then hide else show end
  end

end

class StatusBar

  def initialize(instance, logwindow)
    @sb = instance
    @ci = @sb.get_context_id("Status reports")
    @lw = logwindow
  end

  def message(text)
    @sb.pop(@ci)
    msg = Time.now.strftime("%H:%M:%S") + ': ' + text
    @sb.push(@ci, msg)
    @lw.add_msg(msg)
  end

end

class DataGetter
  DEBUG = 3

  def initialize(delay, statusbar)
    raise "delay must be Fixnum" unless delay.kind_of? Fixnum
    connect(delay)
    @sb = statusbar
    @points = []
  end

  def do_fetch(delay)
    iteration = 0
    GLib::Timeout.add(delay) do
      begin
        m = @if.fetch
      rescue EOFError => why
#        puts("Exiting: #{why.to_s}")
        exit
      rescue StandardError => why
#        @sb.message("Warning: #{why.to_s}")
        retry
      else
        @data = m
        iteration += 1
        @points << [iteration * 5, (m[0]/10).to_i]#((150 / @data[0]) * 100).to_i]
      end
      @fetch
    end
  end

  def temp
    return @data[0]
  end

  def uptime
    return @data[1]
  end

  def points
    return @points
  end

  def disconnect
    @fetch = false
    @if.destroy
  end

  def connect(delay)
    raise "delay must be Fixnum" unless delay.kind_of? Fixnum
    begin
      @if = HTUSBInterface.new(DEBUG)
    rescue StandardError => why
      raise StandardError, "Failed to initialize USB interface: #{why.to_s}"
    end
    @fetch = true
    @data = [0, 0]
    do_fetch(delay)
  end
end

gui = MainGui.new
gui.begin
