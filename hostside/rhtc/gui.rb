class MenuBar
  def initialize
    @bar = Gtk::MenuBar.new
    @items = Hash.new
    @subitems = Hash.new
  end

  def add_item(name)
    raise "name must be String" unless name.kind_of? String
    @items[name] = MenuItem.new(name)
    @items[name].add_to(@bar)
  end

  def add_subitem(item, name)
    raise "name and item must be String" unless name.kind_of? String and item.kind_of? String
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
