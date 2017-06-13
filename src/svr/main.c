#include "all.h"
#include <getopt.h>

///

static void print_usage(char *argv0)
{
	printf("SeaCat.io Agent %s for %s\n", SEACAT_VERSION "", SEACAT_TARGET_TRIPLET "");
	printf("\n%s\n", seacatio_ascii);
	printf("(C) 2014-2017 TeskaLabs Ltd\n");
	printf("https://www.seacat.io/\n\n");
	printf("Build uses:\n\tSeaCat C-Core %s\n\t%s\n\tlibev %d.%d\n\n", seacatcc_version(), SSLeay_version(SSLEAY_VERSION), ev_version_major(), ev_version_minor());
	printf("This product includes software developed by the OpenSSL Project\nfor use in the OpenSSL Toolkit (http://www.openssl.org/)\n\n");
	printf("Usage: %s [ -vh ] [ -c FILE ]\n\n", argv0);

	printf("Arguments:\n");
	printf(" -c [--config] FILE     : Specify file path to configuration file\n                          default value location: '%s'\n", sca_config.config_file);
	printf(" -v [--verbose]         : Print more information (enable debug output)\n");
	printf(" -h [--help]            : Show this help\n");
}

static void optparse(int argc, char **argv)
{
	int c;

	while (1)
	{
		static struct option long_options[] = 
		{
			{"verbose", no_argument,  0,  'v' },
			{"help", no_argument, 0, 'h'},
			{"config", required_argument, 0, 'c'},
			{0, 0, 0, 0}
		};

		// getopt_long stores the option index here.
		int option_index = 0;

		c = getopt_long(argc, argv, "vhc:", long_options, &option_index);

		// Detect the end of the options.
		if (c == -1) break;

		switch (c)
		{     
			case 'h':
				print_usage(argv[0]);
				exit(EXIT_SUCCESS);

			case 'v':
				ft_config.log_verbose = true;
				break;

			case 'c':
				sca_config.config_file = strdup(optarg);
				break;

			case '?':
				exit(EXIT_FAILURE);
     
			default:
				exit(EXIT_FAILURE);
		}
	}

	// Print any remaining command line arguments (not options)
	if (optind < argc)
	{
		fprintf(stderr, "Unknown options on command line:\n");
		while (optind < argc) fprintf(stderr, "  %s\n", argv[optind++]);
		exit(EXIT_FAILURE);
	}

}

///

struct sca_app sca_app;

int main(int argc, char **argv)
{
	// Set configuration values 
	sca_config_environ();

	optparse(argc, argv);

	bool ok = sca_app_init(&sca_app);
	if (!ok) return EXIT_FAILURE;

	int ret = sca_app_run(&sca_app);

	FT_DEBUG("Exiting with status %d", ret);

	sca_app_fini(&sca_app);
	return ret;
}
