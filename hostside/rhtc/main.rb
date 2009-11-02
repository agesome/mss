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
    init_base
    init_menu
    init_statusbar
    init_data

    @dg = DataGetter.new(500)
  end

  def init_base
    @wmain = Gtk::Window.new
    @wmain.set_title WTITLE
    @wmain.signal_connect("delete-event") { Gtk.main_quit }
    
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
    @menu.add_subitem("Main", "Quit") { Gtk.main_quit }
    @menu.add_to(@boxes[:menu])
  end

  def init_statusbar    
    @statusbar = Gtk::Statusbar.new
    @boxes[:status].add(@statusbar)
  end
  
  def init_data
    @temp = Gtk::Label.new("Temperature")
    @boxes[:data].add(@temp)
    @pbar = Gtk::ProgressBar.new
    @boxes[:data].add(@pbar)

    GLib::Timeout.add(750) do
      @pbar.fraction =  @dg.temp / 10 / MAXTEMP
      @pbar.text = "Temperature: #{@dg.temp.to_f / 10} C"
      true
    end
  end
  
  def begin
    @wmain.show_all
    Gtk.main
  end
  
end

gui = MainGui.new
gui.begin
