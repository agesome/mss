#!/usr/bin/env ruby

require 'gtk2'
require 'cairo'
require 'graph'
require 'usbif'

class MenuBar
  def initialize
    @bar = Gtk::MenuBar.new
    @items = Hash.new
    @subitems = Hash.new
  end

  def add_item(name)
    @items[name] = MenuItem.new(name)
    @items[name].add_to(@bar)
  end

  def add_subitem(item, name)
    @items[item].add_subitem(name) { yield }
  end

  def add_to(box)
    box.add(@bar)
  end

  class MenuItem
    def initialize(name)
      @item = Gtk::MenuItem.new(name)
      @content = Gtk::Menu.new
      @item.set_submenu(@content)
      @subitems = Hash.new
    end

    def add_subitem(name)
      @subitems[name] = Gtk::MenuItem.new(name)
      @subitems[name].signal_connect("activate") { yield }
      @content.append(@subitems[name])
    end

    def add_to(menu)
      menu.append(@item)
    end
  end
end

class DataGetter

  def initialize(delay)
    connect(delay)
  end
  
  def do_fetch(delay)
    GLib::Timeout.add(delay)  do
      begin
        m = @if.fetch
      rescue StandardError => why
        puts "Warning: #{why}"
      end
      @data = m if m
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
    begin
      @if = HTUSBInterface.new(3)
    rescue StandardError=> why
      puts "Failed to initialize USB interface: #{why}"
      exit(1)
    end
    @fetch = true
    @data = [0, 0]
    do_fetch(delay)
  end

end

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
