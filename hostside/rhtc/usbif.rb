require 'HTLLUSB'

# a simple wrap for HTLLUSB
class HTUSBInterface
  
  def initialize(dbg_level)
    raise "dbg_level must be Fixnum" unless dbg_level.kind_of? Fixnum
    begin
      @dev = HTLLUSB.new dbg_level
    rescue StandardError => why
      raise StandardError, why.to_s
    end
  end

  def fetch
    begin
      return @dev.fetch_data.unpack "s*"
    rescue IOError => why
      raise StandardError, "Fetch failure: #{why.to_s}"
    end
  end
  
  def destroy
    @dev.close
  end

end

class DataGetter

  def initialize(delay)
    raise "delay must be Fixnum" unless delay.kind_of? Fixnum
    connect(delay)
  end
  
  def do_fetch(delay)
    GLib::Timeout.add(delay)  do
      begin
        m = @if.fetch
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
      @if = HTUSBInterface.new(3)
    rescue StandardError=> why
      raise StandardError, "Failed to initialize USB interface: #{why.to_s}"
    end
    @fetch = true
    @data = [0, 0]
    do_fetch(delay)
  end

end
