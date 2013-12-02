require 'socket'
['Object', 'ObjectNode'].each { |n| Proxy.require File.expand_path('../' + n, __FILE__) }

module Proxy
  # An ObjectNode with support for fetching remote objects.
  class Client < ::Proxy::ObjectNode
    # Initialize a new Client instance.
    # @param [...] *s Arguments to ObjectNode::initialize.
    def initialize(*s)
      if s.length == 1 and s[0].kind_of?(BasicSocket)
        super(s[0])
      elsif s[0].ancestors.include?(BasicSocket)
        super(s[0].new(*s[1..-1]))
      else
        raise ArgumentError.new("Don't know what to do with argument '#{s.inspect}'")
      end
    end

    def self.define_transaction(name, type = nil, *args, &block)
      type ||= name
      if block_given?
        define_method(name) do |*a|
          send_message(type, Message.new(type, *args, block.call(*a)))
          import(receive_message())
        end
      else
        define_method(name) do
          send_message(Message.new(type, *args))
          import(receive_message())
        end
      end
    end

    def list_objects
      send_message(Message.new(:list_exported))
      handle_message(wait_for_message(:note => :exports))
    end

    # define_transaction(:fetch);
    # define_transaction(:remote_eval, :eval)

    # Fetch a remote object.
    #
    # @param [String] name The name of the remote object to fetch.  This should be one of the
    #   names returned by `list_objects`.
    def fetch(name)
      send_message(Message.new(:fetch, name))
      handle_message(wait_for_message(:note => name))
    end
    alias_method :[], :fetch


    # Evaluate code on the other side of the connection.  This is probably a bad idea...
    # @param [String] string Code to be evaluated.
    def remote_eval(string)
      note = 'eval_' + Time.now.to_s
      send_message(Message.new(:eval, string, note))
      import(wait_for_message(:note => note))
    end
  end
end
