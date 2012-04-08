
#include <assert.h>
#include <stdint.h>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <math.h>
#include <stdio.h>

/*
 * use this get_finger_table_as_string() function. when asked for its
 * finger table, a node should respond with the string returned by
 * this function.
 */


/* "..." is some struct/class that contains a member "id" as the id of
 * the node pointed to by that entry.
 *
 * myid is the id of the node
 * calling this function.
 */

std::string
get_finger_table_as_string(const std::vector<...>& table,
                           const uint32_t m,
                           const uint32_t myid,
                           const uint32_t idx_of_entry1)
{
    std::stringstream s;
    assert(table.size() == (idx_of_entry1 + m));
    s << "finger table:\n";
    for (size_t i = 1; (i - 1 + idx_of_entry1) < table.size(); ++i) {
        using std::setw;
	s << "entry: i= " << setw(2) << i << ", interval=["
	  << setw(4) << (myid + (int)pow(2, i-1)) % ((int)pow(2, m))
	  << ",   "
	  << setw(4) << (myid + (int)pow(2, i)) % ((int)pow(2, m))
	  << "),   node= "
	  << setw(4) << table.at(i - 1 + idx_of_entry1).id
	  << "\n";
    }
    return s.str();
}

/********************************************************/

/*
 * use this get_keys_table_as_string() function. when asked for its
 * keys table, a node should respond with the string returned by this
 * function.
 */


/* "..." is some struct/class that contains members "name" and "data"
 * as the name and data of the file.
 */

std::string
get_keys_table_as_string(const std::map<int32_t, ...>& table)
{
    std::stringstream s;
    std::map<int32_t, ...>::const_iterator it = table.begin();
    /* std::map keeps the keys sorted, so our iteration will be in
     * ascending order of the keys
     */
    s << "keys table:\n";
    for (; it != table.end(); ++it) {
        using std::setw;
	/* assuming file names are <= 10 chars long */
	s << "entry: k= " << setw(4) << it->first
	  << ",  fname= " << setw(10) << it->second.name
	  << ",  fdata= " << it->second.data
	  << "\n";
    }
    return s.str();
}


/********************************************************/

/* use the following for the main function of your Chord-node file
 * (e.g., analogous to the "MyService_server.cpp" in the thrift
 * example).
 */

#include <getopt.h>
#include <stdarg.h>
#include <assert.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    INIT_LOCAL_LOGGER();
    int opt;
    int long_index;

    int m = -1;
    int id = -1;
    int port = -1;
    int introducerPort = -1;
    int stabilizeInterval = -1;
    int fixInterval = -1;
    int seed = -1;
    const char *logconffile = NULL;

    struct option long_options[] = {
        /* mandatory args */

        {"m", required_argument, 0, 1000},

        /* id of this node: 0 for introducer */
        {"id", required_argument, 0, 1001},

        /* port THIS node will listen on, at least for the
         * Chord-related API/service
         */
        {"port", required_argument, 0, 1002},



        /* optional args */

        /* if not introducer (id != 0), then this is required: port
         * the introducer is listening on.
         */
        {"introducerPort", required_argument, 0, 1003},

        /* path to the log configuration file */
        {"logConf", required_argument, 0, 1004},

        /* intervals (seconds) for runs of the stabilization and
         * fixfinger algorithms */
        {"stabilizeInterval", required_argument, 0, 1005},
        {"fixInterval", required_argument, 0, 1006},

        {"seed", required_argument, 0, 1007},

        {0, 0, 0, 0},
    };
    while ((opt = getopt_long(argc, argv, "", long_options, &long_index)) != -1)
    {
        switch (opt) {
        case 0:
            if (long_options[long_index].flag != 0) {
                break;
            }
            printf("option %s ", long_options[long_index].name);
            if (optarg) {
                printf("with arg %s\n", optarg);
            }
            printf("\n");
            break;

        case 1000:
            m = strtol(optarg, NULL, 10);
            assert((m >= 3) && (m <= 10));
            break;

        case 1001:
            id = strtol(optarg, NULL, 10);
            assert(id >= 0);
            break;

        case 1002:
            port = strtol(optarg, NULL, 10);
            assert(port > 0);
            break;

        case 1003:
            introducerPort = strtol(optarg, NULL, 10);
            assert(introducerPort > 0);
            break;

        case 1004:
            logconffile = optarg;
            break;

        case 1005:
            stabilizeInterval = strtol(optarg, NULL, 10);
            assert(stabilizeInterval > 0);
            break;

        case 1006:
            fixInterval = strtol(optarg, NULL, 10);
            assert(fixInterval > 0);
            break;

        case 1007:
            seed = strtol(optarg, NULL, 10);
            break;

        default:
            exit(1);
        }
    }

    /* if you want to use the log4cxx, uncomment this */
    // configureLogging(logconffile);

    assert((m >= 3) && (m <= 10));

    assert((id >= 0) && (id < pow(2,m)));

    assert (port > 0);

    srand(seed);
