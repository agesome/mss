require 'HTLLUSB'

# a simple wrap for HTLLUSB
class HTUSBInterface
  
  def initialize dbg_level
    begin
      @dev = HTLLUSB.new dbg_level
    rescue StandardError => why
      raise StandardError, why.to_s
    end
  end

  def fetch
    begin
      m = @dev.fetch_data
    rescue StandardError
      raise StandardError, "Fetch failure"
    end
    return m.unpack "s*" if m
    return nil
  end
  
  def destroy
    @dev.close
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
