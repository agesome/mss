#!/usr/bin/env ruby

require 'gtk2'
require 'cairo'
require 'graph'
require 'usbif'
require 'gui'

class MainGui
  WTITLE='rHTC'
  MAXTEMP = 125.0
  DEBUG = 0
  
  def initialize
    init_base
    init_menu
    init_statusbar
    init_data
    
    begin
      @dg = DataGetter.new(500)
    rescue StandardError => why
      puts(why.to_s)
      Kernel::exit
    end
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
      :graphs => Gtk::VBox.new,
      :controls => Gtk::HBox.new,
      :status => Gtk::HBox.new
    }
    
    @wmain.add @box_main
    @boxes.values.each { |box| @box_main.pack_start(box) }

  end
  
  def init_menu
    @menu = MenuBar.new
    @menu.add_item("Main")
    @menu.add_subitem("Main", "Quit") { exit }
    @menu.add_to(@boxes[:menu])
  end

  def init_statusbar    
    @statusbar = Gtk::Statusbar.new
    @boxes[:status].add(@statusbar)
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
  
  class DataGetter
    
    def initialize(delay)
      raise "delay must be Fixnum" unless delay.kind_of? Fixnum
      connect(delay)
    end
    
    def do_fetch(delay)
      GLib::Timeout.add(delay) do
        begin
          m = @if.fetch
          
        rescue EOFError => why
          puts "Exiting: #{why.to_s}"
          exit
        rescue StandardError => why
          puts "Warning: #{why.to_s}"
        else
          @data = m
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
end

gui = MainGui.new
gui.begin
