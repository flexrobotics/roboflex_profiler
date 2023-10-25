#include <iostream>
//#include <boost/program_options.hpp>
#include "roboflex_transport_mqtt/mqtt_nodes.h"
#include "roboflex_profiler/metrics_central_impl.h"

//namespace po = boost::program_options;

struct metrics_central_params {
    std::string mqtt_broker_address = "127.0.0.1";
    int mqtt_broker_port            = 1883;
    std::string mqtt_metrics_topic  = "roboflexmetrics";
};

void metrics_central_print_usage(int, char ** argv, const metrics_central_params & params) {
    fprintf(stderr, "\n");
    fprintf(stderr, "Usage: %s [options]\n", argv[0]);
    fprintf(stderr, "\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -h, --help\n");
    fprintf(stderr, "  -a, --mqtt_broker_address <address> (default: %s)\n", params.mqtt_broker_address.c_str());
    fprintf(stderr, "  -p, --mqtt_broker_port <port> (default: %d)\n", params.mqtt_broker_port);
    fprintf(stderr, "  -t, --mqtt_metrics_topic <topic> (default: %s)\n", params.mqtt_metrics_topic.c_str());
    fprintf(stderr, "\n");
}

bool metrics_central_params_parse(int argc, char ** argv, metrics_central_params & params) {
    for (int i=1; i<argc; i++) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            metrics_central_print_usage(argc, argv, params);
            exit(0);
        }
        else if (arg == "-a" || arg == "-mqtt_broker_address") { params.mqtt_broker_address = argv[++i]; }
        else if (arg == "-p" || arg == "-mqtt_broker_port") { params.mqtt_broker_port = stoi(argv[++i]); }
        else if (arg == "-t" || arg == "-mqtt_metrics_topic") { params.mqtt_metrics_topic = argv[++i]; }
        else {
            std::cerr << "ERROR: Unknown argument \"" << arg << "\"." << std::endl;
            metrics_central_print_usage(argc, argv, params);
            exit(0);
        }
    }
    return true;
}

int main(int argc, char* argv[]) {

    metrics_central_params params;

    if (metrics_central_params_parse(argc, argv, params) == false) {
        return 1;
    }

    // std::string mqtt_broker_address;
    // int mqtt_broker_port;
    // std::string mqtt_metrics_topic;

    // // Get CLI options
    // po::options_description desc("Metrics Central Options");
    // desc.add_options()
    //     ("help,h", "produce help message")
    //     ("mqtt_broker_address,a", po::value<std::string>(&mqtt_broker_address)->default_value("127.0.0.1"), "MQTT broker address.")
    //     ("mqtt_broker_port,p", po::value<int>(&mqtt_broker_port)->default_value(1883), "MQTT broker port.")
    //     ("mqtt_metrics_topic,t", po::value<std::string>(&mqtt_metrics_topic)->default_value("roboflexmetrics"), "MQTT metrics topic.")
    // ;
    // po::variables_map vm;
    // po::store(po::parse_command_line(argc, argv, desc), vm);
    // po::notify(vm);
    // if (vm.count("help")) {
    //     std::cerr << desc << "\n";
    //     return 1;
    // }

    std::cerr << "STARTING METRICS CENTRAL, LISTENING TO TOPIC "
              << "\"" << params.mqtt_metrics_topic << "\" AT "
              << params.mqtt_broker_address << ":"
              << params.mqtt_broker_port
              << std::endl;

    try {

        // Set up the mqtt subscriber that will receive metrics messages from a broker.
        auto context = roboflex::transportmqtt::MakeMQTTContext();
        int keepalive_seconds = 60;
        int qos = 0;
        int loop_timeout_milliseconds = 100;
        bool debug = false;
        std::string node_name = "MQTTMetricsSubscriber";
    
        auto subscriber = roboflex::transportmqtt::MQTTSubscriber(
            context, params.mqtt_broker_address, params.mqtt_broker_port, params.mqtt_metrics_topic, node_name, keepalive_seconds, qos, loop_timeout_milliseconds, debug);

        subscriber.start();

        std::cerr << "RUNNING..." << std::endl;

        // Run the metrics viewer in a window.
        std::string window_title = "Metrics Central: " +
            params.mqtt_metrics_topic + " on " + params.mqtt_broker_address +
            ":" + std::to_string(params.mqtt_broker_port);

        // Will block until the window is closed.
        int retval = run_metrics_viewer(subscriber, window_title);

        subscriber.stop();

        return retval;

    } catch (...) {
        std::cerr << "\n\nHEY, COULD NOT CONNECT TO AN MQTT BROKER.\n"
                  << "IS ONE RUNNING AT " << params.mqtt_broker_address << ":"
                  << params.mqtt_broker_port << "?\n"
                  << "You might want to install one - try googling for "
                  << "how to install mosquitto, a popular mqtt broker.\n\n" << std::endl;
        throw;
    }
}
