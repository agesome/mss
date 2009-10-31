require 'HTLLUSB'

# a simple wrap for HTLLUSB
class HTUSBInterface
  
  def initialize dbg_level
    begin
      @dev = HTLLUSB.new dbg_level
    rescue Exception => why
      raise Exception, why.to_s
    end
  end

  def fetch
    m = @dev.fetch_data
    if m != nil
      return m.unpack "s*"
    else
      raise Exception, "Fetch failure"
    end
  end
  
  def destroy
    @dev.close
  end

end
