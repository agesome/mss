require 'HTLLUSB'

# a simple wrap for HTLLUSB
class HTUSBInterface
  
  def initialize(dbg_level)
    raise "dbg_level must be Fixnum" unless dbg_level.kind_of? Fixnum
    begin
      @dev = HTLLUSB.new(dbg_level)
    rescue Exception => why
      raise StandardError, why.to_s
    end
  end

  def fetch
    begin
      return @dev.fetch_data.unpack "s*"
    rescue EOFError
      raise EOFError, "Device disconnected."
    rescue IOError => why
      raise StandardError, "Fetch failure: #{why.to_s}"
    end
  end
  
  def destroy
    @dev.close
  end

end
