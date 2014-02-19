/** @file
 *
 * Client/server program for the basic controls test.
 *
 * Herein we implement a simple control system for an RGB LED connected to an
 * Arduino (running the sketch in rgb-led/rgb-led.ino, q.v.) attached to a
 * Raspberry Pi via I2C.  The client (master) sends control messages (consisting
 * of red, green, and blue channel data) to the server (slave) at a fixed rate.
 */
#include <cstdio>

/* Manually disable client mode on ARM machines (e.g. the Raspberry Pi, which
   gives us enough grief anyway... */
#if defined(__arm__) && ! defined(DISABLE_CLIENT)
# define DISABLE_CLIENT 1
#endif

/* ****************************************************************
 * Help text stuff.
 */
#if ! defined(DISABLE_CLIENT)
#  define PRINT_USAGE(stream) fprintf(stream, "Usage: %s [OPTION]... ADDRESS PORT [EVDEV]\n", argv[0])

#  define HELP_TEXT "\
Simple controls-test program.\n\
\n\
Options:\n\
  -h	Show this help.\n\
\n\
Client mode is assumed when EVDEV argument is present; otherwise server mode is\n\
enabled.\n"
#else
#  define PRINT_USAGE(stream) fprintf(stream, "Usage: %s [OPTION]... ADDRESS PORT\n", argv[0])
#  define HELP_TEXT "\
Simple controls-test program: server-only build.\n\
\n\
Options:\n\
  -h	Show this help.\n\
\n"
#endif  /* ! defined(DISABLE_CLIENT) */
/* **************************************************************** */

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <crisp/comms/BasicNode.hh>
#include <crisp/comms/NodeServer.hh>

/* Define some aliases for our communication-types for convenience. */
using namespace crisp::comms;
typedef BasicNode<boost::asio::ip::tcp> Node;
typedef typename Node::Protocol Protocol;
typedef typename Protocol::endpoint Endpoint;

/** Pretty-print (using ANSI terminal codes) the red, green, and blue values in
 * a ModuleControl object to a file.
 *
 * @param fp File-pointer to print the values to.
 *
 * @param mc ModuleControl instance to fetch values from.
 */
static void
print_rgb(FILE* fp, const ModuleControl& mc)
{
  const crisp::comms::DataValue<>* dv ( nullptr );
  if ( (dv = mc.value_for("red")) != nullptr )
    fprintf(fp, " \033[1;31m%d\033[0m", dv->get<uint8_t>());
  if ( (dv = mc.value_for("green")) != nullptr )
    fprintf(fp, " \033[1;32m%d\033[0m", dv->get<uint8_t>());
  if ( (dv = mc.value_for("blue")) != nullptr )
    fprintf(fp, " \033[1;34m%d\033[0m", dv->get<uint8_t>());
}

#include "gnublin_i2c.hpp"

static int
run_server(boost::asio::io_service& service,
           const Endpoint& target_endpoint)

{
  crisp::comms::NodeServer<Node> server ( service, target_endpoint );

  gnublin_i2c i2c ( "/dev/i2c-1", 3 );

  /* I don't trust this "Gnublin" code (it's terribly written and I had to fix
     it to even get it to compile).   We'll be replacing it eventually. */
  i2c.setDevicefile("/dev/i2c-1");
  i2c.setAddress(3);

  if ( i2c.fail() )             /* Won't happen because the Gnublin stuff opens
                                   the device on-demand.  /Le sigh.../ */
    return 1;

  uint8_t values[3];
  memset(values, 0, sizeof(values));

  using namespace crisp::comms::keywords;

  /* Declare our interface configuration. */
  server.configuration.add_module( "LED", 3)
    .add_input<uint8_t>({ "red",   { _neutral = 0, _minimum = 0, _maximum = 255 } })
    .add_input<uint8_t>({ "green", { _neutral = 0, _minimum = 0, _maximum = 255 } })
    .add_input<uint8_t>({ "blue",  { _neutral = 0, _minimum = 0, _maximum = 255 } });

  /* Disconnect the default "module-control received" handler, and connect our
     own. */
  server.dispatcher.module_control.received.clear();
  server.dispatcher.module_control.received.connect(
    [&](Node& node, const crisp::comms::ModuleControl& mc)
    {
      fprintf(stderr, "[0x%0x] \033[1;33mModule-control received\033[0m:", THREAD_ID);
      print_rgb(stderr, mc);
      fputc('\n', stderr);

      const crisp::comms::DataValue<>* dv ( nullptr );
      
      /* Update our input-value array. */
      if ( (dv = mc.value_for("red")) != nullptr )
          values[0] = dv->get<uint8_t>();
      if ( (dv = mc.value_for("green")) != nullptr )
          values[1] = dv->get<uint8_t>();
      if ( (dv = mc.value_for("blue")) != nullptr )
          values[2] = dv->get<uint8_t>();


      /* This call here is just so EW EW EW EW EW.  There's a reason for it, of
         course, but that reason is irrelevant with the way we're using I2C. */
      i2c.send(values[0], &values[1], 2);

      if ( i2c.fail() )
        {
          perror("write");
        }
    });

  server.run();
  return 0;
}


/* Disable client functionality on the Raspberry Pi.

   _With_ this code (and using the old controller code based on the
   terribly-bloated Boost.Signals2 library), I had trouble getting the program
   to compile on the Pi without running out of memory.

   We don't need to support client mode on the Pi, so we might as well keep it
   disabled anyway.
 */
#if ! defined(DISABLE_CLIENT)

#include <crisp/input/EvDevController.hh>
#include <float.h>

/** Given some angle, return a coterminal angle between 0 and two pi.
 */
static float
stdpos(float a)
{
  if ( a > 0 && a < 2 * M_PI )
    return a;
  else
    return std::fmod(a, 2 * M_PI) + (a > 0 ? 0 : 2 * M_PI);
}

/** Calculate the standard-position angle in the X-Y plane given by two
 *  cartesian coordinates.
 */
static float
theta(float x, float y)
{
  if ( std::abs(x) < 4 * FLT_EPSILON )
    {
      if ( std::abs(y) < 4 * FLT_EPSILON )
	return 0;
      else
	return y > 0 ? M_PI_2 : 3 * M_PI_2;
    }
  else if ( std::abs(y) < 4 * FLT_EPSILON )
    return x > 0 ? 0 : M_PI;
  else
    {
      float at ( std::atan(y/x) );
      if ( x < 0 ) 
        return stdpos(y < 0 ? at - M_PI : at + M_PI);
      else
        return stdpos(at);
    }
}


/** Update a ModuleControl instance to reflect the input coordinates.
 *
 * @param mc ModuleControl to update.
 *
 * @param x Value of the X input coordinate.
 *
 * @param y Value of the Y input coordinate.
 *
 */
void
update_control(crisp::comms::ModuleControl& mc,
               float x, float y)
{
  float
    angle = theta(x, y),
    intensity = std::min(std::hypot(x, y), 1.0f);
  fprintf(stderr, "angle %f, intensity %f:", angle * 180 / M_PI, intensity);
  
  mc.set<uint8_t>("red", 255 * intensity * (1 + std::cos(angle))/2);
  mc.set<uint8_t>("green", 255 * intensity * (1 + std::cos(angle + 2 * M_PI / 3))/2);
  mc.set<uint8_t>("blue", 255 * intensity * (1 + std::cos(angle + 4 * M_PI / 3))/2);

  print_rgb(stderr, mc);
  fputc('\n', stderr);
}


/** Run the client code, connecting to the specified endpoint and taking input
 *  from the specified Linux `evdev` input device.
 */
static int
run_client(boost::asio::io_service& service,
           const Endpoint& target_endpoint,
           const char* evdev)
{
  /* Create a socket, and try to connect it to the specified endpoint. */
  boost::system::error_code ec;
  typename Protocol::socket socket ( service );
  typename Protocol::resolver resolver ( service );

  boost::asio::connect(socket, resolver.resolve(target_endpoint), ec);

  if ( ec )
    { fprintf(stderr, "connect: %s\n", strerror(ec.value()));
      return 1; }
  else
    {
      /* We're going to be modifying the ModuleControl instance on-demand
         (whenever the input controller changes state), but sending it at a
         fixed rate.  Since this will be happening in different threads and
         there's no easy way to coordinate the memory access and modification
         otherwise,  we need a mutex to keep everything in sync.

         (If we didn't do this, we could e.g., get halfway through updating the
         module control instance, send a control packet based on that [likely
         invalid] data, and then finish updating.)
      */
      std::mutex mutex;
      ModuleControl mc;
      float x = 0, y = 0;

      /* Create the input device. */
      using namespace crisp::input;
      EvDevController controller ( evdev );
      assert(controller.axes.size() >= 2);

      /* Make sure the axes we'll be using are in absolute mode. */
      for ( size_t i ( 0 ); i < 2; ++i )
        if ( controller.axes[i].type != Axis::Type::ABSOLUTE )
          {                     /* Set up emulation for the axis. */
            controller.axes[i].mode = Axis::Type::ABSOLUTE;

            /* We need to manually initialize the absolute-axis raw-value
               configuration when an axis isn't actually absolute... */
            controller.axes[i].raw.minimum =
              - (controller.axes[i].raw.maximum = 256);
            controller.axes[i].raw.neutral = 0;

            /* set cubic mapping: value = 1 * x^3 + 0 * x^2 + 0 * x + 0.  */
            controller.axes[i].set_coefficients({1, 0, 0, 0});
          }

      /* Add event handlers for each of the axes we're interested in. */
      controller.axes[0].hook([&](const Axis& axis, Axis::State state)
                              {
                                std::unique_lock<std::mutex> lock ( mutex );
                                x = state.value;
                                if ( mc.module )
                                  update_control(mc, x, y);
                              });
      controller.axes[1].hook([&](const Axis& axis, Axis::State state)
                              {
                                std::unique_lock<std::mutex> lock ( mutex );
                                y = state.value;
                                if ( mc.module )
                                  update_control(mc, x, y);
                              });


      /* Instantiate the network node. */
      Node node ( std::move(socket), NodeRole::MASTER );

      
      /* Override the default configuration-response-recieved handler with
         this:

         - First, make sure that the received configuration is at least similar to what
           we expect; shut down the node if it isn't.

         - Store the received configuration.

         - Set up the module-control object to 
      */
      node.dispatcher.configuration_response.received.connect(
        [&](Node& _node, const Configuration& configuration)
        {
          node.configuration = configuration;
          mc.reset(&(node.configuration.modules[0]));

          using namespace crisp::util::literals;

          /* Set up the control packet to be sent at 25 Hz. */
          fprintf(stderr, "Setting up send action... ");
          node.scheduler.schedule(25_Hz,
                                  [&](crisp::util::PeriodicAction&) {
                                    std::unique_lock<std::mutex> lock ( mutex );
                                    node.send(mc);
                                    return;
                                  });
          fprintf(stderr, "done.");
        });

      /* clear the module-control-sent handler -- it's just a lot of spam. */
      node.dispatcher.module_control.sent.clear();


      std::atomic<bool> controller_run_flag ( true );

      /* On user interrupt (Ctrl+C), shut down the node and stop the
         controller's main loop.

         If the program doesn't stop, wiggle the controller a bit. (Current
         EvDevController implementation uses a blocking read, so it won't notice
         that the run flag has changed until it gets another event.)
      */
      boost::asio::signal_set ss ( service, SIGINT );
      ss.async_wait([&](const boost::system::error_code& error, int sig)
                    {
                      if ( ! error )
                        {
                          node.halt();
                          controller_run_flag = false;
                        }
                    });

      /* Queue a configuration-query message send. */
      node.send(MessageType::CONFIGURATION_QUERY);

      /* Start the controller and the network node. */
      std::thread controller_thread([&]() { controller.run(controller_run_flag); });
      node.run();
    }

  return 0;
}
#endif  /* ! defined(DISABLE_CLIENT) */

int
main(int argc, char* argv[])
{
  /* Parse user options. */
  int c;
  while ( (c = getopt(argc, argv, "h")) != -1 )
    switch ( c )
      {
      case 'h':
	PRINT_USAGE(stdout);
	fprintf(stdout, HELP_TEXT);
	return 0;

      default:
	abort();
      }

  if ( argc - optind < 2 )
    {
      PRINT_USAGE(stderr);
      return 1;
    }

  /* Parse the address and port passed via the command line, and inform the user
     if they're doin' it wrong. */
  char* tail;
  boost::system::error_code pec;

  namespace ip = boost::asio::ip;

  ip::address address ( ip::address::from_string(argv[optind], pec) );

  int port ( strtoul(argv[optind + 1], &tail, 0) );

  if ( pec )
    fprintf(stderr, "%s: %s (failed to parse IP address).\n", argv[optind], pec.message().c_str());

  if ( *tail != '\0' )
    fprintf(stderr, "%s: Need integer port number.\n", argv[optind + 1]);

  if ( pec || (*tail != '\0') )
    return 1;

  /* Construct the target endpoint (IP address + port number) object. */
  using Protocol = Node::Protocol;
  typename Protocol::endpoint target_endpoint(address, port);


  /* Asio's io_service manages all the low-level details of parceling out work
     to IO worker threads in a thread-safe manner.  It's required for pretty
     much any network-related function, so we'll create one here.  */
  boost::asio::io_service service;

#if ! defined(DISABLE_CLIENT)
  return ( argc - optind < 3 )
    ? run_server(service, target_endpoint)
    : run_client(service, target_endpoint, argv[optind+2]);
#else
  return run_server(service, target_endpoint);
#endif
}

