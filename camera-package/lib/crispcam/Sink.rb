#!/usr/bin/ruby

old_dashw = $-w
$-w = false
require 'gst'
Gst.init if not Gst.respond_to?(:initialized?) or not Gst.initialized?
$-w = old_dashw
CrispCam.require('crispcam/util')

module CrispCam
  
  # Self-contained GStreamer-based video sink for streaming h.264 video from a Crispcam::CameraSource.
  class Sink
    @pipeline = nil
    @source = nil
    @main_loop = nil
    @thread = nil

    def initialize()
      @main_loop = nil
      @thread = nil
      
      @pipeline, vars = build_pipeline([ ['udpsrc', :source],
                                         ['capsfilter', { :caps => Gst::Caps.from_string('application/x-rtp, media=(string)video, encoding-name=(string)H264, payload=(int)127') } ],
                                         'rtph264depay',
                                         'avdec_h264',
                                         'autovideosink'
                                       ])
      vars.each_pair { |k, v| instance_variable_set(('@' + k.to_s).intern, v) }
    end

    def run(_port = nil)
      @source.port = _port if not _port.nil?
      run_main
    end
    def stop
      @main_loop.quit if running?
      @main_loop = nil
    end
    def running?
      @thread.alive?
    end

    def start(_port = nil)
      @source.port = _port if not _port.nil?
      @thread = Thread.new { run_main }
    end

    private
    def run_main
      @main_loop = GLib::MainLoop.new

      # Set up the message handler.
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

      FlyTrap.trap(:INT) { $stderr.puts "Caught interrupt: exiting main loop."; @main_loop.quit }

      # start playing
      @pipeline.play
      begin
        @main_loop.run
      rescue Interrupt
      ensure
        @pipeline.stop
        @main_loop.quit
      end
    end

  end
end
