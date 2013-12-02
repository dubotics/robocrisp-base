# Utility functions.


# Build a GStreamer pipeline out of the given elements.  If an element is specified by name
# only, we simply create that element and move on.  If the element is specified as an an array,
# however, it's treated differently.  The first entry must always be a type-name string, and
# additional entries can specify
#
#   * an instance variable into which to copy the element reference (a symbol, with leading '@'), and
#
#   * any properties to be set on the element (a hash of the form Symbol => (value) ).
def build_pipeline(elements)
  pipeline = Gst::Pipeline.new('raspi-stream')
  vars = {}

  prev = nil
  elements.each do |el|
    element = if el.kind_of?(String)
                Gst::ElementFactory.make(el)
              else
                opts = el.select { |s| s.kind_of?(Hash) }[0]
                sym = el.select { |s| s.kind_of?(Symbol) }[0]
                o = Gst::ElementFactory.make(el[0])

                opts.each_pair { |k, v| o.set_property(k.to_s, v) } if not opts.nil?
                vars[sym] = o if not sym.nil?
                o
              end
    pipeline.add(element)
    prev >> element if not prev.nil?
    prev = element

  end
  [pipeline, vars]
end


# FlyTrap: set multiple handlers for a given signal.
module FlyTrap
  InvertedSignalsList = Signal.list.invert
  @@handlers = nil

  def self.trap(sig, a = nil, &b)
    block = if block_given?
              b
            else
              a
            end
    @@handlers = {} if @@handlers.nil?
    @@handlers[Thread.current] = {} if not @@handlers.has_key?(Thread.current)
    ct = Thread.current
    thread_handlers = @@handlers[ct]
    if not thread_handlers.has_key?(sig)
      thread_handlers[sig] = []
      Signal.trap(sig) do |ss|
        if @@handlers.has_key?(Thread.current)
          $stderr.puts("[#{Thread.current.inspect}] Handling #{InvertedSignalsList[ss]}");
          @@handlers[Thread.current][sig].each { |handler| handler.call(ss) }
        end
      end
    end
    
    thread_handlers[sig] << block.to_proc

  end
end
