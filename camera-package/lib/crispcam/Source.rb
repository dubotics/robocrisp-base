# Pipeline for network streaming of the Raspberry Pi camera feed.

old_dashw = $-w
$-w = false
require 'gst'
Gst.init if not Gst.respond_to?(:initialized?) or not Gst.initialized?
CrispCam.require('open3')
CrispCam.require('crispcam/util')
$-w = old_dashw

module CrispCam
  class RaspiCameraSource
    @resolution = nil
    @fps = nil
    @bitrate = nil
    @metering = nil
    @white_balance = nil
    @exposure = nil

    attr_reader :fps, :bitrate, :resolution, :metering

    @pipeline = nil
    @sink = nil
    @source = nil

    @main_loop = nil
    @thread = nil

    def metering=(m)
      @metering = m
      restart if running?
    end

    def width=(w)
      @width = w
      restart if running?
    end

    def height=(w)
      @height = w
      restart if running?
    end


    def exposure=(a)
      @exposure = a
      restart if running?
    end


    def white_balance=(a)
      @white_balance = a
      restart if running?
    end

    def resolution=(a)
      @width = a[0]
      @height = a[1]
      restart if running?
    end

    def bitrate=(b)
      @bitrate = b
      restart if running?
    end

    def fps=(f)
      @fps = f
      restart if running?
    end

    def initialize(_width = 1280, _height = 853, _fps = 25, _bitrate = 1024000, _white_balance = 'auto', _metering = 'matrix', _exposure = 'auto')
      @thread = nil
      @main_loop = nil

      @width = _width
      @height = _height
      @fps = _fps
      @bitrate = _bitrate
      @white_balance = _white_balance
      @metering = _metering
      @exposure = _exposure

      elements = [ ['fdsrc', :source ],
                   'queue',
                   'h264parse' ,
                   ['rtph264pay', { :pt => 127 }],
                   'queue',
                   [ 'udpsink', :sink, { :'force-ipv4' => true, :sync => false } ]
                 ]
      @pipeline, vars = build_pipeline(elements)
      vars.each_pair { |k, v| instance_variable_set(('@' + k.to_s).intern, v) }
    end

    def running?
      (not @thread.nil?) and @thread.alive?
    end

    def restart
      start
    end

    def stop
      @main_loop.quit if running?
      @thread.kill if not @thread.nil?
    end

    def start(_remote_host = nil, _remote_port = nil)
      @sink.host = _remote_host if not _remote_host.nil?
      @sink.port = _remote_port if not _remote_port.nil?
      raise 'Remote host not set!' if @sink.host == 'localhost'

      $stderr.puts("#{self}.#{__method__}(): main thread is #{Thread.main}.")
      @thread = Thread.new { run_main() }
    end

    # Run the pipeline, blocking until a keyboard interrupt is received.
    def run(_remote_host = nil, _remote_port = nil)
      @sink.host = _remote_host if not _remote_host.nil?
      @sink.port = _remote_port if not _remote_port.nil?
      raise 'Remote host not set!' if @sink.host == 'localhost'

      run_main(true)
    end
    
    
    private
    def raspivid_command(verbose = false)
      verbose = true if $-d or $-w
      o = ['raspivid', '--nopreview', '--timeout', '0', '--output', '/dev/stdout', '--vstab']
      o.concat([ '--verbose' ]) if verbose
      o.concat([ '--width', @width.to_s ]) if not @width.nil?
      o.concat([ '--height', @height.to_s ]) if not @height.nil?
      o.concat([ '--framerate', @fps.to_s ]) if not @fps.nil?
      o.concat([ '--bitrate', @bitrate.to_s ]) if not @bitrate.nil?
      o.concat([ '--exposure', @exposure.to_s ]) if not @exposure.nil?
      o.concat([ '--metering', @metering ]) if not @metering.nil?
      o.concat([ '--awb', @white_balance ]) if not @white_balance.nil?
      o
    end

    def run_main(verbose = false)
      @main_loop = GLib::MainLoop.new
      @keep_running = true
      Open3.popen3(*raspivid_command(verbose)) { |sin, sout, serr, wait_thread|
        sin.close
        @source.fd = sout.fileno
        Thread.new { puts serr.readline while not serr.eof? }

        # listen to playback events
        
        @pipeline.bus.add_watch do |bus, message|
          case message.type
          when Gst::MessageType::EOS
            @main_loop.quit
          when Gst::MessageType::ERROR
            p message.parse_error
            @main_loop.quit
          when Gst::MessageType::INFO
            p message.parse_info
          when Gst::MessageType::WARNING
            p message.parse_warning
          end
          true
        end

        sig_handler = Proc.new do
          $stderr.puts "Caught interrupt: exiting camera loop."
          @pipeline.stop
          @main_loop.quit
          Process.kill(:SIGTERM, wait_thread.pid) if wait_thread.alive?
        end

        FlyTrap.trap(:QUIT, sig_handler)
        FlyTrap.trap(:TERM, sig_handler)

        # start playing
        @pipeline.play
        begin
          @main_loop.run
        ensure
          @pipeline.stop
          @main_loop.quit
          begin
            Process.kill(:SIGKILL, wait_thread.pid) if wait_thread.alive?
          rescue Errno::ESRCH
          end
        end
      }
    end

  end
end
