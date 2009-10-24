#!/usr/bin/env ruby

require 'gtk2'
require 'cairo'

TITLE='rHTC'

#hint: try remembering last middle point (mx, my)
def smooth_to (cr, tx, ty)
    sx = cr.current_point[0]
    sy = cr.current_point[1]
    mx = (sx + tx) / 2
    my = (sy + ty) / 4

    cr.curve_to sx, sy, mx, my, tx, ty
    cr.stroke
    cr.arc sx, sy, 4, 0, 2*Math::PI
    cr.stroke
    cr.arc tx, ty, 4, 0, 2*Math::PI
    cr.stroke
end

wmain = Gtk::Window.new
wmain.set_title TITLE
wmain.signal_connect "delete-event" do Gtk.main_quit end

box_main = Gtk::VBox.new
box_menu = Gtk::HBox.new
box_data = Gtk::HBox.new
box_status = Gtk::HBox.new
box_graphs = Gtk::VBox.new

wmain.add box_main
box_main.pack_start box_menu
box_main.pack_start box_data
box_main.pack_start box_graphs
box_main.pack_start box_status

menubar = Gtk::MenuBar.new
mainmenu = Gtk::MenuItem.new "Main"
mainmenu_contents = Gtk::Menu.new
mainmenu_quit = Gtk::MenuItem.new "Quit"

box_menu.add menubar
menubar.append mainmenu
mainmenu.set_submenu mainmenu_contents
mainmenu_contents.append mainmenu_quit
mainmenu_quit.signal_connect "activate" do Gtk.main_quit end

# temporary set of testing coordinates
x1 = 100; y1 = 150; x2 = 180; y2 = 80; x3 = 300; y3 = 180;

tgraph = Gtk::DrawingArea.new
tgraph.set_size_request 500, 200
box_graphs.pack_start tgraph
tgraph.signal_connect "expose-event" do
  cr = tgraph.window.create_cairo_context

  cr.move_to x1, y1
  smooth_to cr, x2, y2
  cr.move_to x2, y2
  smooth_to cr, x3, y3
  cr.set_line_width 3
  cr.stroke
  true
end

wmain.show_all

Gtk.main
