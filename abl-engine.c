#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _DEBUG
#define _BREAK __debugbreak()
#else
#define _BREAK 0
#endif

typedef int bool;
#define false 0
#define true 1
#define PANIC (_BREAK, exit(__LINE__), 1)

#define OUTPUT_BUFFER_SIZE 32768ull
#define DNSMASQ_LINE_MAX_LENGTH 1024
// TODO other prefixes
#define OUTPUT_PREFIX "local=/"
#define OUTPUT_SUFFIX "/"
#define OUTPUT_DELIM "/"

static void finishOutputLine(char* output, int* output_i, int* output_startOfLine, FILE* fOut)
{
    // add suffix
    memcpy(output + *output_i, OUTPUT_SUFFIX, sizeof(OUTPUT_SUFFIX) - 1) || PANIC;
    *output_i += sizeof(OUTPUT_SUFFIX) - 1;

    output[*output_i] = '\n';
    *output_startOfLine = ++(*output_i);

    // write out buffer
    if (*output_i >= OUTPUT_BUFFER_SIZE - DNSMASQ_LINE_MAX_LENGTH - 1)
    {
        fwrite(output, 1, *output_i, fOut);
        *output_startOfLine = 0;
        *output_i = 0;
    }
}

static void printHelp(void)
{
    printf("Usage: abl-engine -i PATH -t ORIGIN [-n NUM] [-o OUT]\n"
           "\n"
           "  PATH         Path of file or url pointing to the domain list.\n"
           "\n"
           "  ORIGIN       One of the values:\n"
           "    local: The domain list is a local file.\n"
           "    dl:    The domain list will be fetched from the given url.\n"
           "    dbg:   Just generate a dummy input file at PATH (overwrites existing) and do no processing.\n"
           "\n"
           "  NUM          Number of entries to generate when the ORIGIN is dbg. Must be positive.\n"
           "\n"
           "  OUT          Path of output file, instead of writing to standard output.\n"
           "\n"
           "  TODO argument for like uhhhh output path or maybe stdout ye.\n"
           "  TODO argument for a command to use for downloading web resource.\n"
           "  TODO other arguments that are needed\n"
           "");
}

typedef enum { ListOrigin_invalid, ListOrigin_local, ListOrigin_download, ListOrigin_dbg_generate_input } ListOrigin;

int main(int argc, const char** argv)
{
    if (OUTPUT_BUFFER_SIZE < DNSMASQ_LINE_MAX_LENGTH - 2)
        return PANIC;
    ListOrigin origin = ListOrigin_invalid;
    const char* input_path = 0;
    const char* output_path = 0;
    FILE* fIn;
    FILE* fOut = stdout;
    int genEntryCount = 100;

    // for development time
    if (0)
    {
        argc = 7;
        argv = malloc(sizeof(void*) * argc);
        if (!argv) return PANIC;
        argv[1] = "-i";
        argv[2] = "_dbg";
        argv[3] = "-o";
        argv[4] = "_dbg";
        argv[5] = "-t";
        argv[6] = "local";
    }

    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "--help"))
        {
            printHelp();
            return 0;
        }
        if (argv[i][0] == '-' && strlen(argv[i]) == 2)
        {
            switch (argv[i][1])
            {
                case 'H':
                case 'h':
                    printHelp();
                    return 0;
                case 'T':
                case 't':
                    if (++i >= argc)
                        break;
                    if (!strcmp(argv[i], "local"))
                        origin = ListOrigin_local;
                    else if (!strcmp(argv[i], "dl"))
                        origin = ListOrigin_download;
                    else if (!strcmp(argv[i], "gen"))
                        origin = ListOrigin_dbg_generate_input;
                    else
                    {
                        fprintf(stderr, "Invalid ORIGIN -t, expected local/dl/gen\n");
                        return PANIC;
                    }
                    continue;
                case 'I':
                case 'i':
                    if (++i >= argc)
                        break;
                    input_path = argv[i];
                    if (!strcmp(input_path, "_dbg"))
                        input_path = "C:/Users/SimpleVar/Desktop/dnsmasq/test.txt";
                    continue;
                case 'O':
                case 'o':
                    if (++i >= argc)
                        break;
                    output_path = argv[i];
                    if (!strcmp(output_path, "_dbg"))
                        output_path = "C:/Users/SimpleVar/Desktop/dnsmasq/out.txt";
                    continue;
                case 'N':
                case 'n':
                    if (++i >= argc)
                        break;
                    genEntryCount = atoi(argv[i]);
                    if (genEntryCount <= 0)
                    {
                        fprintf(stderr, "Invalid entry count to generate -n, expected a positive integer\n");
                        return PANIC;
                    }
                    continue;
                default:
                    fprintf(stderr, "Unrecognized argument %s\n", argv[i]);
                    return PANIC;
            }
            fprintf(stderr, "Missing value after -%c\n", argv[i][1]);
            return PANIC;

        }
        fprintf(stderr, "Invalid arguments\n");
        return PANIC;
    }

    if (!input_path)
    {
        fprintf(stderr, "Missing PATH -i\n");
        return PANIC;
    }

    switch (origin)
    {
        case ListOrigin_invalid:
            fprintf(stderr, "Missing ORIGIN -t (local/dl/gen)\n");
            return PANIC;
        case ListOrigin_download:
            fprintf(stderr, "Not Implemented yet");
            return PANIC;
        case ListOrigin_dbg_generate_input:
            if ((fIn = fopen(input_path, "w")) == 0) return PANIC;
            fputs("# comment 1\n", fIn);
            fputs("a.xy\n", fIn);
            fputs("# comment 2\n", fIn);
            for (int i = 1; i < genEntryCount; i++)
            {
                fprintf(fIn, "%i", i);
                int r = rand() & 15;
                if (r < 1)		fputs("wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww.domain.ext\n", fIn);
                else if (r < 2) fputs("wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww.domain.ext\n", fIn);
                else if (r < 4) fputs("wwwwwwwwwwwwww.domain.ext\n", fIn);
                else if (r < 8) fputs("www.domain.ext\n", fIn);
                else			fputs("domain.ext\n", fIn);
            }
            return 0;
        case ListOrigin_local:
            break;
        default:
            return PANIC;
    }

    if ((fIn = fopen(input_path, "r")) == 0) return PANIC;
    if (output_path)
        if ((fOut = fopen(output_path, "w")) == 0) return PANIC;

    char* output = (char*)malloc(OUTPUT_BUFFER_SIZE);
    if (!output) return PANIC;
    int output_i = 0;
    int output_startOfLine = 0;
    bool isCommentLine = false;
    char buff[DNSMASQ_LINE_MAX_LENGTH + 1];
    while (fgets(buff, sizeof(buff), fIn))
    {
        int i = 0;
        // skip whitespaces in beginning of line
        while (i < sizeof(buff) && (buff[i] == ' ' || buff[i] == '\t'))
            i++;
        if (buff[i] == '#') isCommentLine = true;
        if (isCommentLine)
        {
            while (i < sizeof(buff) && buff[i] != '\n')
                i++;
            if (i < sizeof(buff))
                isCommentLine = false; // reached \0 before \n, so the comment line is still going, longer than buff size
            continue;
        }

        // read domain line
        int start = i;
        while (i < sizeof(buff) && buff[i] != '\n')
            i++;
        if (i >= sizeof(buff))
            return __LINE__; // we expect non-comment lines to always fit in buff

        // trim whitespace from end of line
        while (i > start && (buff[i] == ' ' || buff[i] == '\t'))
            i--;

        int lineLength = i - start;
        if (lineLength == 0)
            continue;

        // do we fit in the current output line
        if (sizeof(OUTPUT_DELIM) - 1 + sizeof(OUTPUT_SUFFIX) - 1 + output_i - output_startOfLine + lineLength > DNSMASQ_LINE_MAX_LENGTH)
            finishOutputLine(output, &output_i, &output_startOfLine, fOut);

        if (output_i == output_startOfLine)
        {
            // prefix a new output line
            memcpy(output + output_i, OUTPUT_PREFIX, sizeof(OUTPUT_PREFIX) - 1) || PANIC;
            output_i += sizeof(OUTPUT_PREFIX) - 1;
        }
        else
        {
            // delimiter between domains in the same line
            memcpy(output + output_i, OUTPUT_DELIM, sizeof(OUTPUT_DELIM) - 1) || PANIC;
            output_i += sizeof(OUTPUT_DELIM) - 1;
        }

        // append domain to current output line
        memcpy(output + output_i, buff + start, lineLength) || PANIC;
        output_i += lineLength;

        // sanity check
        output_i >= OUTPUT_BUFFER_SIZE && PANIC;
    }

    if (output_i != output_startOfLine)
        finishOutputLine(output, &output_i, &output_startOfLine, fOut);
    fwrite(output, 1, output_i, fOut);

    fclose(fOut);
    fclose(fIn);

    _BREAK;
    return 0;
}
