# -*- coding: utf-8 -*-
Gem::Specification.new do |s|
  s.platform	= Gem::Platform::RUBY
  s.name        = 'crispcam'
  s.summary     = 'GStreamer-based streaming utility/code for the Raspberry Pi'
  s.description = <<-EOF
  CRISPCam provides an on-demand camera server/client and control interface for the
  Raspberry Pi's camera board.  Based on GStreamer, it uses the Pi's pre-encoded h.264 video
  stream for efficiency.
EOF
  s.version     = '1.0.0'
  s.files       = [ 'bin/crispcam-client',
                    'bin/crispcam-daemon',
                    'lib/crispcam.rb',
                    'lib/crispcam/Source.rb',
                    'lib/crispcam/Sink.rb',
                    'lib/crispcam/util.rb'
                    ]
  s.authors	= ['Collin J. Sutton']
  s.add_runtime_dependency('gstreamer', '~> 2.0', '>= 2.0.2')
  s.add_runtime_dependency('proxy-rmi', '~> 0.1')
  s.executables << 'crispcam-client' \
  		<< 'crispcam-daemon'
end  
