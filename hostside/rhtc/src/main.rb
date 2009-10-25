#!/usr/bin/env ruby

require 'gtk2'
require 'cairo'
require 'block_graph.rb'

TITLE='rHTC'

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

tgraph = Gtk::DrawingArea.new
tgraph.set_size_request 500, 400
box_graphs.pack_start tgraph

gr = BlockGraph.new 500, 400

tgraph.signal_connect "expose-event" do
  gr.context = tgraph.window.create_cairo_context
  gr.lwidth = 5

  gr.draw_stick 150, 100
  gr.draw_stick 20, 80
  gr.draw_stick -60, 60

  true
end

wmain.show_all

Gtk.main
