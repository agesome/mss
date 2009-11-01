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
